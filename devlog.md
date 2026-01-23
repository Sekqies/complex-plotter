Shorter devlog - nothing too fancy going on here!

Most of the plotter-related features are already implemented, the only "math-y" thing missing being a few elementary functions, and some non-elementary functions that are convenient (like Lambert-W and the Zeta function). This means that we steer into the bane of every project's existence: making it usable!

I plan on shipping this project as a single executable rather than a zipped folder, and later compiling it into web assembly for a demo. In either case, we can't work with files, because browsers don't have file systems in the traditional sense. Virtual file systems _do_ exist, but there is a much simpler solution: turning files into strings!
Now, this could be done by simply embedding a string like `const std::string vertex_source = R"({our code})"`, but if we wrote code this way, we lose our syntax highlighting and linting! The only solution is to create these strings dynamically at build-time.

And that's pretty much it! We just now refactor any file access we wanted to use our `get_source` function, that secretely fetches the built string in compile-time if it's known. 