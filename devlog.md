We've got got compiled shaders up and running!

All the benefits and reasoning behind doing this were throughouly discussed in [Issue #23](https://url.jam06452.uk/1ff5wk1). What matters is now we have our interpreted shader as a "preview", and whenever we want improved performance, it's just a matter of hitting "enter" and we're done!

First, we had to convert our stack of opcodes into a GLSL string. After the type overhaul, this was simple enough of a task.
Since I'm dead-set on having a semi-decent developer experience while coding with my fragment shader (`shader/plotter.frag`), I had to do some string manipulation trickery to guarantee that all important function and coloring definitions are shared between shader modes. 
This in turn required me to modify our existing `Shader` class dependency to accept strings rather than file paths to compile, and to create a new `CompilerShader` class to handle all the aforementioned string manipulation. There are some optimizations still there to be made to increase compile time (namely ommiting function definitions we're not using), but this is not necessary for the time being, as compile times are low.
Also, since compile times were shown to be tiny, the async proposal related to [Issue #28](https://url.jam06452.uk/cwjcm1) might not be necessary.
This also involved changing our global function state and callback functions to take a reference to different shaders, so we can change them on the fly. 

That's all for this devlog. Attached, a video of me rendering a huge function with the interpreter vs compiler: the performance difference is palpable!

**Related commits:**
[Commit 5f38f07](https://url.jam06452.uk/sg59ba): Converting stack into glsl infix string
[Commit 8f40852](https://url.jam06452.uk/g4japn): CompilerShader class done
[Commit 48d3fed](https://url.jam06452.uk/1lh28x0): Added shader switching for compilation

**Related issues**
[Issue #28](https://url.jam06452.uk/cwjcm1) and its subissues.