# Advanced
This is meant for developers that either want to contribute to this project, or just want to understand the underlying structure behind it. 

## Build structure and language(s) of choice.
This is a project written in Microsoft Visual Studio 2026 that uses a CMake build system with ninja (or jinja), using C++20. It purposefully avoids using MSVC-specific features for portability: had this project used C++ Modules, this wouldn't be portable to other systems, namely linux (since GCC and Clang haven't finished implementing them yet).
Since all this project does is graphics, we delegate most of the math to our GPU. This is done through the OpenGL framework, the shader code being written in GLSL. This particular stack was chosen because C++ is a highly performant language, and there is many online resources and guides for using it along OpenGL. 

## Folder structure
All the source code is stored in two main directories:
- `src/`, which stores the source code written for, and specific to, this project
- `dependencies/`, which are all external libraries/reused code from old projects. Namely, the `learnopengl` subdirectory contains custom classes (like `Texture`), that are not directly taken from an outside source, and rather developed by myself.
The subdirectories of `src` refer to specific modularized features of the plotter, which will be discussed in this document.

## Types 
There are three main types defined for parsing and handling functions and operators (functions are treated as operators internally, so `sin(z)` and `sin z` are the same): `Operator`, `TokenOperator` and `FullOperator`.
- `Operator` is an enum used to identify operators, it contains no other information
- `TokenOperator` is a struct used for most parsing purposes. It contains the operators's arity (an `Arity` enum, `NULLARY` for constants), its associativity (an `Associativity` enum), its string representation, whether it's higher order or not, precedence and value, if a constant. It contains all informations needed to parse and constant fold most expressions.
- `FullOperator` is a struct all the information needed to prepare shaders: a `TokenOperator`, its GLSL`#define` directory name and its GLSL corresponding function or string (`Operator::ADD` corresponds to `cadd(vec2 z)` and `SHADER_ADD`)



## Types of rendering
There are two main ways we render things: through **Interpreted Shaders** and **Compiled Shaders**.

### Interpreted Shaders
Interpreted shaders have a list of pre-defined opcodes that are source-injected into it at compile-time. Then, a Texture Buffer Object containing the queue of operations to be done (in reverse polish notation) is sent at runtime, and evaluated by the interpreter. 
Two different stacks are sent to interpreted shaders: a `vec2` queue of constant values, and an `unsigned char` queue of opcodes (internally represented as `uint`). Whenever the interpreter comes across the `CONSTANT` opcode, it pops the top of constant queue, and appends it to our evaluation stack.
For example, assume we want to represent the expression `(z+3)*2`. Or, in RPN, `2 z 3 + *`. This would be sent as following:
Operator queue: `CONSTANT, VARIABLEZ, CONSTANT, ADD, MULT`
Constant stack: `vec2(2.0f,0.0f),vec2(3.0f,0.0f)`
We use vec2s because our constants are complex-valued: they have real and imaginary components. Whenever the interpreter comes across a `CONSTANT` symbol, it grabs the front of the queue, and pops it.

The advntage of interpreted shaders is that there is instant feedback: binding a TBO is a very cheap operator, meaning it can change seamlessly. The problem is that doing stack operations at runtime is expensive, making it slower for large expressions

### Compiled Shaders
Compiled shaders inline all the logic that interpreted shaders do. If you want to render the expression `e^z * z`, rather than sending a stack to be evaluated at runtime, it will source inject the string `cmult(exp(z),z)` into the function's value and recompile the shader. 
This is far faster than interpreting the function, but recompiling takes time. So, it doesn't have the same "instant" change as the interpreted shaders do. That's why a shader is by default interpreted, and if the user wants more performance, they can compile them. 

## The pipeline
There are 4 main steps that this program works in, with the following sequence:
```
Pre-processing -> Parsing -> Preparing shaders -> Rendering
```

### Pre-processing
This step is used for two main purposes:
1. Shader opcode synchronization
2. Source injection
**Shader opcode synchronization** is simply making sure that the opcodes we send to the interpreted shaders match the ones it internally stores. After all, if we send the expression `CONSTANT CONSTANT ADD` as `2u,2u, 6u`, and, internally, `6u` is the code for `MULT`, we are getting a wrong plot. This is prevented here, as the source code for all interpreter logic are written at runtime. 
**Source injection** is also done at this step: all the base code is written to our 2D interpreted shader (`plotter.frag`), and all the shaders that require functions shared with `plotter.frag` copy these 'blocks' from it. After all, we might have issues where the same function being interpreted and compiled looks different, which is not something we can have.

We tell the preprocessor where these "blocks" are with the following syntax:
```c++
#define BLOCK_NAME HERE
[block content...];
#define END_BLOCK_NAME HERE
```

### Parsing
The name "parsing" might be misleading, because we do far more things in this step than just processing user input. This step involves:
1. Processing user input
2. Evaluate higher-order functions
3. Simplifying the result
We **process user input** by first breaking the input string into TokenOperators. This is done by verifying whether the current character matches an operator name, or, if not, is a string with a name matching an operator's (this uses `TokenOperator::str_repr`). 
Once we converted the user's input to `TokenOperator`s, we verify if there is implicit multiplication (`2z` is the same as `2*z`) and break apart ambiguous operations (`-z` is negation, not subtraction), and modify accordingly. Then, we run the shunting-yard algorithm in the resulting list of `TokenOperator` to get a reverse polish notation queue. 

Then, we transform our RPN queue into an Abstract Syntax Tree to **evaluate higher-order functions**. In particular, the derivative has hard-coded rules for binary operators (product rule, quotient rule, etc) and uses a lookup table for unary operators and functions, applying the chain rule to it.
Namely, we use the `build_expression` function to create a subtree through text, with its variables set to `uplaceholder` (so, for instance, if `sin(z)' = cos(z)`, in our lookup table, we'd have `table[Operator::SIN] = build_expression("cos(uplaceholder)")`). Finally, we replace `uplaceholder` with its derivative, recursively applying the chain rule.
The resulting syntax tree is then converted back into an RPN stack, and sent to the next steps.

Then, we have to `simplify` our final results. Simplifying does the following:
1. Transform broken up pairs `a + b*i` into a single complex scalar, `(a,bi)`
2. Constant folds known expressions `(2 + 3 * 5) z` -> `17z`

### Preparing shaders
This is the step in which we pass uniforms to the shader, and all UI logic is handled. Notably, there's two UI states we are keeping track of:
1. FunctionState
2. ViewState
**Function State** stores all relevant information related to the _rendering_ of complex functions: whether it's interpreted or compiled, if it's 3d, if we should reparse it, between others. Essentially, all the UI buttons you see and interact with, alter function state.
**View State** contains information regarding user _interactions_. Namely, it is what event callbacks see whenever you zoom or pan across the function's plot. 
This is also in which the program decides which shader to render and show to the screen.

### Rendering
Here's where all our math logic is ran. We have a block in `plotter.frag` that contains all complex function definitions we need, which is source-injected to any other shaders that might need to run these functions. Currently, the plotter supports every elementary function.
The grid is drawn either using `z` or `f(z)` as a parameter, which is what the "distort grid" parameter does.
The domain_coloring function is what maps `f(z)` to a color. These are the parameters used:

![Domain Coloring parameters](assets\HSL.png)

Where $|l(z)| = \frac{2}{\pi}\arctan(|z|)$

## 3D mode
3D mode is rendered by making an NxN grid mesh, handled by the `Mesh` struct and `create_grid_mesh` function, and shaping it around the function's "real" shape (picking points in set spaces and placing vertices there). 
Height is directly mapped to |f(z)|, so information is redundant. Movement is done through a custom `Camera` struct.

## Picker
Whenever you hover over a value and get a number back, this is done through a "picker" shader. It's a 1x1 grid rendered off-screen that calculates the value of f(z) precisely where you are hovering. The result is then shared as an `out vec4`, the first two floats being `z`, and second two, `f(z)`.