# Optimizations
There are many little optimizations done all throughout this project, that makes it able to run in very fast speeds. 

## Benchmarking
First, it is important to show that the optimizations used in this tool actually provide a substantial performance increase. For such, 4 distinct publicly available complex plotter were used for comparison:
- Samuel Li's [Complex Function Plotter](https://samuelj.li/complex-function-plotter/). The main inspiration for this project.
- Peter E. Francis' [Complex Function Plot](https://peterefrancis.com/complex-function-plot/plotter.html). A CPU based plotter
- David Bau's [Conformal Map Plotter](https://davidbau.com/conformal/). While this function does not use regular domain coloring, it _still_ is a complex function plotter
- Fernando Theodoro & Mateus Bastazini's [Complex Functions](https://www2.fc.unesp.br/matematicaearte/plotter/). A project I contributed to previously

The computer for benchmarking had the following specs:
CPU: Intel Core Ultra 7 255HX
Random Memory: 16GB DDR5 5600 MT/s
Integrated Graphics Card: NVIDIA Geforce RTX 5060

All benchmarking measures the _parsing and drawing_ time for different complex functions. 
1. Polynomial: $z^{10} - z^9 - z^8 - z^7 - z^6 - z^5 - z^4 - z^3 - z^2 - z - 1$
2. Trigonometric and exponential: $\sin(\cos(\tan(z))) \cdot e^z$
3. Non-elementary: $\Gamma(\zeta(z))$

| Creator | Uses GPU | Polynomial | Trigonometric and exponential | Non-elementary |
| :--- | :---: | :--- | :--- | :--- |
| **Peter E. Francis** | No | 11336 ms | 7098 ms | *Not supported* |
| **Samuel Li** | Yes | 279 ms | 459 ms | 11369 ms |
| **David Bau** | Yes | 369 ms | 120 ms | *Not supported* |
| **Theodoro & Bastazini** | Yes | 226 ms | 293 ms | 194 ms |
| **This (Web)** | Yes | 4.14 ms | **2.42 ms** | **2.27 ms** |
| **This (Desktop)**| Yes | **2.91 ms** | 3.93 ms | 3.14 ms |

It is important to notice that the web version outperforms the desktop version for small functions. This is likely due to optimizations introduced in the Emscripten to WebAssembly optimizations, besides the transpilation from OpenGL to WebGL. 

It can be observed from the table that this tool outperforms all of its other competitors. This is due to our optimizations!

## Architectural Optimizations

### Compiled and Interpreted Modes
One of the main bottlenecks of GPU based renderers is dynamically writing the shader code, and compiling it at runtime, which introduces a short stutter. This tool uses **per-request** compiling, meaning that it only compiles the shader if the user splicitly requests it.
Otherwise, the parsed expression is transformed into a sequence of bytecode instructions in Reverse-Polish Notation and sent to the shader as a texture. The shader then evaluates this as a stack. This requires no recompilations. 
For higher performance, the expression can also be turned into a GLSL string, and recompiled.

### Web Assembly and C++
This tool is mainly desktop-focused, but the web version is compiled directly from C++ into WebAssembly using Emscripten. Web Assembly is a far lower-level, higher-performance alternative to Javascript, which makes its use a big player in outperforming other javascript-based plotters.

### GPU Rendering
All simple calculations are done in the GPU, rather than the CPU. This allows the plots to be drawn far faster than their CPU counterparts (as evidenced in the Benchmarking section)

### Threading
For the high precision CPU renders, the plot is drawn in multiple concurrent threads. 

## Numeric optimizations
### Constant Folding
Constant expressions are evaluated before the drawing and rendering logic. For instance, `(13 * 7)*z` will be sent for rendering as `91 * z`

### Simplification
Expressions that can be evaluated to constants will be, during runtime. Such examples include:
1. Division by itself `z/z = 1` (with an added singularity at `z=0`)
2. Composition of inverses `sin(arcsin(z)) = z`

### Analyic Differentiation
Derivatives are calculated analytically rather than numerically. This means that an expression such as `d/dz(z^2 + z)` is correctly transformed into `2z + 1` in parse-time, rather than numerically evaluated through approximations in render-time.

## Memory Optimizations

### Manual Register Management and Mutable Math
In the now deprecated arbitrary-precision math library for the GPU, registers were manually managed. Rather than using local variables, 16 registers of shared memory was used between every complex function. This was transpiled automatically from the low precision GLSL. 
Also, this library uses non-functional math. This means that rather than functions returning a new copy to their result, they modify a given register. Such an example is the hp_add function:
```glsl
number R[16];
number hp_add(number a, number b){
    void hp_add(in number a, in number b, out number res) {
    if (a.sign == b.sign) {
        abs_sum(a, b, R[0]);
        res.sign = a.sign;
        return;
    }
    int cmp = 0;
    compare_abs(a, b, cmp);
    if (cmp >= 0) {
        abs_hp_sub(a, b, R[0]);
        res.sign = a.sign;
        return;
    }
    abs_hp_sub(b, a, R[0]);
    R[0].sign = b.sign;
    res = R[0];
}
}
```