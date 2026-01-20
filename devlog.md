We can take derivatives of functions now!

For those not familiar with the term, a derivative is a **higher-order function** (meaning it takes a function as a parameter, and outputs another function). This is not a feature natively supported by our shader's language (GLSL), so we have to do all the higher-order logic ourselves in the parser. Which makes sense: if `d(z^2) = 2z`, and the user types `d(z^2)`, we can just send `2z` to the shader.
Our stack is great for evaluating functions numerically, but not optimal for symbolic manipulation. An **Abstract Syntax Tree**, or AST for short, works far better for this, because it simulates the "recursive" nature of expressions. So, we had to convert our RPN queue to an AST, and then back to RPN. Some time put into that!

Now, there is a generic way to calculate any derivative, but it would require us to work with infinitesimally small quantities, which just do not exist in computers. So, if we tried to use it, we'd get a rough approximation at best, and a very innacurate result at worst. This, however, can be fixed with a derivative table, falling back to this numeric method whenever needed

All this higher-order tree manipulation is of intermediate difficult when using raw pointers (`int*`) but quickly becomes _hell_ when using C++'s smart pointers (`std::unique_ptr`). Safe to say, this difficulty is what caused most of these dev hours.

**Related Issues**:
[Issue #8](https://github.com/Sekqies/complex-plotter/issues/8): Computing higher order functions [And it's sub-issues:]
	* Issue #9: Stack into syntax tree
	* Issue #10: Syntax tree into RPN stack
	* Issue #11: Differentiation
		* Issue #12: Operator rules
		* Issue #13: Numerical differentiation
		* Issue #14: Analytic differentiation

**Related Commits:**
[Commit b56b9fa](https://url.jam06452.uk/fyai8z): ast -> stack, stack -> ast
[Commit f410dba](https://url.jam06452.uk/utr804): Finished implementing derivatives