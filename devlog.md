Unfortunately, it's time to do our laundry.

With 3D rendering complete, all of the essential features of the _plotter_ are done. This means that, had we wanted to, we could ship this as a fully-fledged math engine and leave the job of actually turning it into an application to someone else. But we can't be doing that! Therefore, it's time for us to start making way to shipping the project.

First of all, I wanted this plotter to be complete, meaning that it implements _every_ elementary function (those being, in simple terms, a set of well-behaved functions that mathematicians use). This is simply a manner of writing the already existing real and imaginary components of these functions (and their derivatives), which are well-known and defined. 
Is this essentially just writing boilerplate? Yes! Thankfully, this is not my first rodeo implementing these functions, so I could port a good amount of code from an old project. This is all done now: all elementary functions and their derivatives have been implemented!

Now, for the engine-specific things: so far we have been writing shaders into files and reading them at runtime. This works, but adds unnecesary file I/O operations, and makes it impossible to port our entire project as one .exe file. So we had to remove that!
One way to do this would be by writing our GLSL code in strings, but this removes linting. Instead, I implemented a dynamic source string builder at build time. 

Attached, the new functions!


**Related Commits**

[Commit 7923ee3](https://url.jam06452.uk/1vzsu0m): File conversion to strings at build-time
[Commit 5ba94c0](https://url.jam06452.uk/7erthk): Complete removal of file reading logic
[Commit 0a56357](https://url.jam06452.uk/1g1lfwm): Finished implementing elementary functions
[Commit 48d08fc](https://url.jam06452.uk/15x1axn): Derivatives of elementary functions

**Related Issues**
[Issue #30](https://url.jam06452.uk/fvwf6y): Switch from shader files to strings