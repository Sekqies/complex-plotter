Rendering images of a selected area with arbitrarily large precision. This means allowing a user-controlled amount of floats of precision for each complex number, the amount of iterations executed for non-elementary function, and possibly a different method of rendering colors to make sure it's more precise.

UHPM should be used only to render static "images", so it shouldn't care about performance. It should be fine to wait a couple of seconds to wait for a picture of a specific range in the graph to be rendered, if this is going to be used for specific analysis, or for an image to be used in, for instance, a scientific paper.

Okay, let's attempt to do this.

In order to get a functioning high precision library, there is some basics that we have to do. 
First, we need the four basic operations, exponentiation, logarithm and trigonometric functions. We can use whatever numerical method we want for this so long as it runs in a non-absurd manner (a good limit to set is, at maximum, 3 seconds for a whole frame.) 
Once we have the four operations down, all those other functions can be computed quite fast through Newton-Raphson and their Taylor Series. Since precision is our main objective, we should run these numerical methods a number of times needed to give us an error smaller than our current precision.
This also means making the number of iterations in our non-elementary functions user-defined.

We don't want to be rewriting all our functions defined for regular precision in higher precision. So, once we have these basic functions down, and some other helper functions used by GLSL (like length, step, mix, etc), we will need to write a transpiler that transforms functions' implementations to use our higher precision library.
For instance, the function `cadd(vec2 a, vec2 b){return a+b;}` should become `cadd(hp_vec2 a, hp_vec2 b) {return hp_vec2(hp_add(a.x,b.x),hp_add(a.y,b.y)));}`. It might be useful to manually replace all occurances of regular additions with a helper function `scalar_add(a,b)` in order to make this parsing easier (and avoid having to create yet another RPN interpreter)

There will be lots of precomputing necessary on the CPU side, namely for constants like PI, E and lookup tables for non-elementary functions. This means that we will need to have a C++ way to generate these arbitrary precision `number` structs. 

Optionally, we could simply write a parser that transforms GLSL into C++ in build time, giving us the possibility of running our "shader" in the CPU if necessary. This might be such case if we ask for a very high limb count (for instance, 32 `uint` limbs, which equates to 1024 bits), which might cause the GPU to run for too long and end up losing context.
This would give us a more stable, albeit far slower, method of drawing functions with any precision. Since this would all be transpiled from GLSL, we actually wouldn't need to rewrite any code, and would give us some pretty nice luxuries, like arbitrary function constant folding.

In short, our steps to implement UHPM are:
1. Implementing functions
1.1. Implementing the four operations
1.2. Implementing ln, exp, sin, cos, tan
1.3. Implementing other GLSL specific functions used in other pieces of code

2. Writing a transpiler from low-precision GLSL to high-precision GLSL

3. Working with CPU side arbitrary precision
3.1 Sending arbitrary precision numbers to the shader 
3.2 Precomputing constants
3.3 Precomputing non-elementary function lookup tables

4. Writing a GLSL to C++ transpiler (optional)
4.1 Hooking up GLSL function names to their respective C++ functions automatically
4.1.1 Constant folding with C++ GLSL definitions
4.2 Running shaders in the CPU