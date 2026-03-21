The complex function plotter is done!

A little bit of personal lore here: I made another complex function plotter four years ago, when I was still in highschool. It was clunky, but worked fine enough. Still, I felt it missed some things, which led me to work on this project. 
Now, I have accomplished all things I have set out to do. Bugfixes aside, I believe I can put this project to rest now.

First, I worked on making the high precision plots reactive. They take a while to render, so having them shut down the main thread while the user waits on a blank screen isn't very fun. Now, each thread updates the canvas for every pixel they render. This allows the user to see the progress being made!
Also, some reconfigurations were needed in the web version, since the browser needs special permissions to run Web Workers (the equivalent to threads for browsers)

Then, some quality of life changes: I added an amortization factor so the graph takes longer to converge to white (and black), and fixed a parsing mistake one of my beta testers caught. 

Finally, documentation and benchmarking! I could always tell this tool was _fast_, but never _how fast_. I took upon myself to benchmark my own tool, and some other project's (since they are opensource, I can manually modify the code to do so). The result were great: this project outperforms everything else by a factor of 50.

And that's it for now folks! 

Attached, our high precision plotting in real time!

**Commits**
[9dd1515](https://url.jam06452.uk/3yqc6h): Async image generation
[4266aaa](https://url.jam06452.uk/c71kuv): Add threading for web version
[ded05c1](https://url.jam06452.uk/gvwf7p): Update documentation and perform benchmarking
[2e32e99](https://url.jam06452.uk/zgmyyg): Fixed issue where expressions with negations would be evaluated incorrectly
[2e10128](https://url.jam06452.uk/1rzb8g3): Fixed issue where moving the screen would cause the high precision plot to shift as well.