ARBITRARY PRECISION IS UPON US!

Keeping things short, I tried, and failed, to implement my own GPU, GLSL arbitrary precision math library. Why? Because nobody else has! And for good reason.
- First, I built the math library in a functional way `(add(x,y) -> z)`. This causes the GPU to run out of registers, because returning arrays means allocating temporary memory
- Then, I rewrote the entire library to use 16 global registers, and by mutating a third input `(add(x,y,z) -> void)`. This worked at first, but for minimally nested functions (like `cos(sin(z))`), the GPU tries, and fails, to unroll the loops. So the shader doesn't compile
- To fix that, I once again optimized the library and added flags for the GPU not to unroll. _then_ we run into a problem where the _assembly_ becomes too large. To my knowledge there is no practical solution for this.

At this point, I realized that true arbitrary precision is impossible in the GPU. Thankfully, GLSL translates pretty neatly to C++, so I could just, at build step, transform my complex functions into C++, and do all my arbitrary precision in the CPU!

It is very slow, but it's meant for very high detail images. So, completely fine! Attached, some plots!

**Commits**
[2895bc0](https://url.jam06452.uk/2cwiyv): UHPM renders, but not properly
[6d39bca](https://url.jam06452.uk/px4ufa): UHPM works, but not for big expressions
[5d96733](https://url.jam06452.uk/154kn6z): High precision zooming
[43e4423](https://url.jam06452.uk/s0xmt3): GPU to CPU transpiling
[f262c96](https://url.jam06452.uk/137or8f): CPU rendering!

**Issues**
[#Issue #40](.): Arbitrary precision in the CPU
[Issue #41](.): GLSL to C++ Transpiler
    -[#48](.): Synchronizing GLSL and C++ functions
    -[#49](.): Running shaders in the CPU
[Issue #50](.): Reoptimize UHPM 
    -[#51](.): Rewrite math library to use inout
    -[#52](.): Expanding nested expressions in transpiler #52
    -[#53](.): Using registers in transpiler #53