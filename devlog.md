Problem!

Currently, our Ultra High Precision Mode math library is written in a functional manner. This means that all functions are pure, beautiful and return things. This allows us to transform operations like `x * (y + z)` nicely into `hp_mult(x,hp_add(y,z))`

The problem is that GLSL really doesn't like when we return arrays, and nesting functions _also_ introduce multiple temporary variables, which occupate registers. It would much rather have this expression turned into `number expr; hp_add(y,z,expr); hp_mult(expr,x);`. It _also, also_ dislikes temporary variables being allocated in functions.

To fix this, we have to write our code like assembly! This means that we'll have to keep a list of global registers that are used whenever needed by a function, and we do composition on these registers!

This means lots of work ahead:

1. Change math library to use `inout` rather than `return`
2. Change transpiler to expand nested expressions
3. Change `generate_glsl_string` to expand nested expressions
4. Rewrite transpiler to automatically use registers