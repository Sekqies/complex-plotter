These seven hours of devlogging time are mostly due to three things: partial derivatives, non-elementary functions and getting this project to work on the web. As you'll see, we got two out of three on this one!

First, our tool supports derivatives relative to z (denoted as `d/dz`). The rules for it are simple: anything that _isn't_ dependant on `z` is 0, and for things entirely dependant on it, we apply some rules according to a table.
The problem is that `z = x + iy`, and each variable `x,y` is partially dependant on `z`. They can't be equal to 0, since `d/dz(x + iy) = d/dz(z) != d/dz(0)`. To fix this, we set some "sanity" rules, and set `d/dz(x) = 0.5` and `d/dz(y) = -0.5i`. This fixes our problem!

Second, some users requested non-elementary functions (that is, functions we can only ever approximate, since their definitions are infinite). A good time was spent finding good, fast approximations, and most importantly fixing their 'explosions' to infinite, which at times resulted in their values becoming NaN. This is partially solved by first evaluating the function's logarithm (which grows far, faar slower), and exponentiating it back for its regular value.

Third, the web thing. I wanted to get two things working:
1. Exporting plots as images. This worked _graciously_ well. It took me less than half an hour to get it working.
2. Copy and pasting in the browser. I have wasted at least five hours of my life trying to get this to work. I've implemented every solution I've found in the internet to solve it. I reverse engineered an online library to see how it works and put it in my project. I can't get it to work. This is heavily due to the 'security' layer in web assembly that makes it very, very difficult to get text from the OS into the browser. After many hours, I officially gave up.

Attached, some plots of our new functions!

**Commits**
3f015c0: Add exporting plots as images
f3271a2: Added popup when exporting
7255bf4: Implemented the zeta and gamma function