We've got a usable UI, and the good stuff that comes necessary for any good math program, now! 

First, let's get the developer stuff out of the way. Before, if we wanted to add a new function, we'd have to write code in 6 distinct spots, as discussed in [Issue #31](https://url.jam06452.uk/lputto), a consequence of my `#ifndef` clauses black magic. This, now, is all done in load time,making my life far easier.

Now, for the math stuff. I'll make a rare use of bullet points, I:
- Added every elementary function to our constant-folding simplification step.
- Added some (one) utility function: the modulo (%) operator!

Our most important addition is, by far, **hovering** to show a function's value. All our functions are calculated on shaders, so we'd either have to hard code a C++ counterpart for ever GLSL function, or find a way to send data from the shader to the main program.
The problem is, while it's easy to send data _to_ shaders (through `uniform` variables), it's quite difficult to do the opposite. And even if there was an easy way to do this, remember: our fragment shader runs for _every_ pixel, so they all would be sending data to the CPU at the same time, which would be far too expensive.
Instead, what we do is create a 1x1 pixel shader, and send it a copy of our function, plus the mouse coordinates. It then shares a single `vec4` variable that stores both `z` (the input) and `f(z)` (the output). Voilá, we can _get_ things from the shader, now!

Drawing the grid is just some additional math in the fragment shader, and the UI changes are, well, UI. 

**Related Commits**
[Commit 1d123a6](https://url.jam06452.uk/e1qeij): Automated interpreter shader creation
[Commit ba3f299](https://url.jam06452.uk/1n8chst): UI and constant folder overhaul
[Commit 5893ae0](https://url.jam06452.uk/t2uooa): Inspector done!
[Commit 1ff791c](https://url.jam06452.uk/1kc13d6): Added grid


**Related Issues**
[Issue #31](https://url.jam06452.uk/lputto)

