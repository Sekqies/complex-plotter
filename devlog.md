A month later, we're in the world wide web!

From the beginning of the project, I kept telling myself that I would, eventually, port it to Web Assembly. Many architectural choices were made with this in mind (not using compute shaders, sticking to version 330 features, using ImGUI for the frontend, not using a file system, etc). 
I just knew that this would be sort of a headache, and kept delaying it, but it comes a time at our lives where we have to accept that not everyone wants to download a sketchy binary from github. So we had to port it to web assembly!

For those not familiar with the tech, WebAssembly (wasm, for short), is essentially the low-level language of the web. The advantage is that it's faster than javascript, and many languages can compile down to it - and that includes C++!
Our graphics core is entirely made in OpenGL, but thankfully, a tool named Emscripten also handles this conversion by turning OpenGL into WebGL (the web equivalent). So, we just had to rewrite our code to handle things specific to the web.

Our main change was the textures that we were using to send data to our shaders. Originally, this was done through `samplerBuffer`s, which don't exist in the version OpenGL runs. This made us have to change our data to 2D textures, `sampler2D`. This requires a little more math to get data from the texture, but it's small enough to be a change applied both to the desktop and web version (to avoid refactoring headache).

The rest is changing the main loop logic (to be step-based), the event callbacks, and creeating a basic UI. Attached, our code running in the website! 

Take a look for yourself in: https://sekqies.github.io/complex-plotter/

**Commits**
[04e5437](https://url.jam06452.uk/1w417ue): Working web assembly build - ImGUI not working
[181a62c](https://url.jam06452.uk/up08en): Finished web assembly porting
[5ea3105](https://url.jam06452.uk/1s6zjmu): Add web deploy
...and 7 other commits fixing github