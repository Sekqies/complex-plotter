# Features & Guide
This document is an exhaustive list of features implemented and non-exhaustive guide on how to use it.

## 2D Plotting
Plots complex functions $f:\mathbb{C} \to \mathbb{C}$ with standard domain coloring. This mode allows for user interactions to explore the graph, such as:
- **Panning:** Click and drag to pan around the function's plot. 
- **Zooming:** Use the scroll wheel to zoom. Has high precision up to `10^{-8}`
- **Hovering:** Hovering with a cursor over a point will show the input value $z$ and its output $f(z)$
![Interactions](<assets/gifs/Zooming & Panning.gif>)
![Hovering](assets/gifs/Hovering.gif)


## 3D Plotting
Plots complex functions using a domain coloring map, and also a height map, where the height is mapped as $h = |f(z)|$. Allows for panning and zooming as well as:
- **Moving:** WASD keys to move around the function plot. Shift to go up, Ctrl to go down.
- **Rotating:** Right mouse + Drag to rotate your camera. 
![3D Plotting](assets/gifs/3D.gif)

## Function Parser
The plotter includes a custom recursive descent parser that supports complex-valued expressions. 

### Syntax rules
- **Variables:** Supports `z` (the input variable), `x` (real component), `y` (imaginary component) and `t` (time passed)
- **Constants:** All numeric constants, `i` (the imaginary unit), `pi` and `e` (euler's number)
- **Implicit multiplication:** Expressions such as `2z` or `2(z+2)` will be evaluated as `2*z` and `2*(z+2)`

### Supported functions
This plotter supports _every elementary function_. The full list of supported functions is automatically updated as new ones are added, so always make sure to double check on the app's "Help & Keybinds" section. 
The full list, as of now, of supported functions is:
| Category | Functions |
| :--- | :--- |
| **Arithmetic** | `+`, `-`, `*`, `/`, `^` (power), `%` (modulo) |
| **Exponential** | `exp` ($e^z$), `log` (natural), `sqrt` |
| **Trigonometry** | `sin`, `cos`, `tan`, `sec`, `csc`, `cot` |
| **Inverse Trig** | `asin`, `acos`, `atan`, `asec`, `acsc`, `acot` |
| **Hyperbolic** | `sinh`, `cosh`, `tanh`, `sech`, `csch`, `coth` |
| **Inverse Hyperbolic** | `asinh`, `acosh`, `atanh`, `asech`, `acsch`, `acoth` |
| **Complex Structure** | `abs` (Modulus), `arg` (Phase), `real`, `imag`, `conj` |

![Text parsing](assets/gifs/Parsing.gif)

### Derivatives
This plotter supports symbolic differentiation! This means that an expression like $\frac{d}{dz}(z^2 + \sin(z))$ will be evaluated as $2z + \cos(z)$ All derivatives are assumed to be in the form $\frac{d}{dz}$, and should be written as `derivative(expression)`. 

## Render Modes
This plotter offerts two backend modes for evaluating functions: Interpreted and Compiled. By default, they are automatically applied to their best usecase, this may be changed in the settings.

### Interpreted Mode
Interpreted Mode has instant updates, but becomes slower for large expressions. By default, as you type, it updates the graph immediately. 
![Interpreted Mode](assets/gifs/Interpreted.gif)

### Compiled Mode
Compiled Mode has higher speed, but requires a slightly larger time to be ready than Interpreted Mode. You can activate this by pressing the enter key, or compile button.
![Compiled mode](assets/gifs/Compiled.gif)

## Grid
Allows for a grid mapped to input (serves as a reference to where the components of $z$ are integers), and to output (shows how $f(z)$ warps the function space). 
![Grid Warping](<assets/gifs/Grid Warping.gif>)


## Animations
Time is a supported variable, `t`. Modulo and trigonometric operators are supported, so it is possible to do animations with expressions like `z^(sin(0.5t)*10)` (bounce) or `z^((t % 10) * 10)` (repeat). Try some of these!
- `z * exp(i * t)` 
- `sin(z + t)`
- `(1-sin(t))*z + sin(t)*z^2`
- `tan(z)^(sin(t)) % sin(z)`
![Animation of tan(z)^(sin(t)) % sin(z)](assets/gifs/Animation.gif)
