**We can do things in 3D now!**

This might make you ask *what* are we plotting in this additional dimension, and the answer is simple: nothing that we weren't showing already

In our plotter, we use brightness to represent magnitude (how large a number is). Black corresponds to zero, and white corresponds to a very high number, making roots of functions look like "sinks". It turns out that it's far easier to visualize these "sinks", "valleys" and "peaks" with the help of a third dimension (akin to looking at a satellite picture of a mountain range vs being there). 

There's many ways to render functions in 3D, but the one I chose is by laying out a large 256x256 grid of "mini-planes" that are molded to look like the function (there are methods with 'infinite' precision but they are horribly slow and, if we want more precision, we can just add more segments, anyway!). This required me to create a `Mesh` struct and functions to handle their creation and use.
Also, since all this logic operates on vertices, we have to move the 3D code into a vertex shader. This required massive refactoring of pretty much *all* of our preprocessing functions, since they were all rigged to work with the fragment shader. We also had to do more dependency injection trickery to make sure the 2D and 3D shaders remain synchronized (of course).
Finally, we had to create a custom camera to actually move around our 3D plot, and update our state variables to see if the plot is 3D. 

Attached, the third dimension! :O

**Related Issues:**
[Issue #29](https://url.jam06452.uk/ou8zyn): 3D Plotting

**Related Commits:**
[Commit 4a0ab12](https://url.jam06452.uk/dv60mh): Fragment and vertex shader for 3D
[Commit 0a17f71](https://url.jam06452.uk/zsu6bw): Refactor of preprocessor logic
[Commit 614238a](https://url.jam06452.uk/u896ze): Created mesh grid
[Commit 4bfd2ed](https://url.jam06452.uk/fo0yqb): 3D Rendering done
[Commit 0a17f71](https://url.jam06452.uk/a8oszc): Working camera!