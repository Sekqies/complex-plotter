# physics-engine
---
# Style Guide
There are some small conventions in place for contributing code to this project. Namely:

## Naming things
Variables and functions should be named in snake case as such: `do_something()` or `be_something`.
Classes should be named in pascal case, like `ExecuterOfSomething`.
Comments and code should be written in English.

## Indentation
One indentation should be equal to four spaces. For ease of writing code, you may alter your IDE in such a way that pressing tab will equal to four spaces.


## Getters and Setters
Introduce getters and setters in classes only when there is any underlying logic or checks that have to be done before getting or setting a variable. For instance, if you find yourself in a situation as such:
```c++
class Polygon{
    public:
        vector<int> get_vertices(){
            return this->vertices;
        }
        void set_vertices(vector<int> val){
          this->vertices = val;
        }
    private:
        vector<int> vertices;
}
```
Consider instead making `vertices` a public field, as getting and setting vertices produces no side effects.

## Usage of `const`
**Default your variable and function declarations to `const`.** The absence of `const` in, for instance, a function declaration, should indicate that the variable being passed may be mutated. For instance, in the function:
```c++
double sum(vector<double>& ar){
    double out = 0.0;
    for(double& element : ar){
        out += element;
    }
    return out;
}
```
Notice how the value of `ar` never changes. This function could therefore be rewritten as 
```c++
double sum(const vector<double>& ar){
    double out = 0.0;
    for(const double& element : ar){
        out += element;
    }
    return out
}
```

## Usage of `constexpr` and macros
**Prefer constexpr over macros, whenever possible.** If you want to declare that a variable or function's value is known at compile time, use `constexpr`. Things like
```c++
#define PI 3.14159
#define TIMES_PI(x) x * PI
```
Can, and should, be written as 
```c++
constexpr double PI = 3.14159;
constexpr double TIMES_PI(double x){
    return x * PI;
}
```
As it allows the compiler to be type-safe, and prevents substitution errors in macros. 
You may use simple macros for simple tasks that would otherwise be cumbersome to write. **If you do use macros, remember to wrap everything around parentheses.** For instance:
- ✅ `#define null(x) ((x) == NULL)` - simple enough task that is made easier with a macro
- ❌ `#define null(x) x == NULL` - missing parentheses
- ❌ `#define add(x,y) ((x) + (y))` - you should be doing this in a `constexpr` function


## References and pointers
Use references over pointers whenever possible. If you require pointers for pointer arithmetic, you may use them. You may use C-styled pointers, but generally prefer `std::unique_ptr` and `std::shared_ptr` as they are (more) memory safe. Your order of priority should be:
1. References (e.g `const double maximum_element(const double& ar)`)
2. Smart pointers (e.g `const double maximum_element(std::unique_ptr<const double>& ar`)
3. Raw pointers (e.g `const double maximum_element(const double* ar)`)
Note that passing a unique_ptr by value transfers ownership. Passing unique_ptr by reference temporarily borrows it, as instended for function with no side effects.


## Using namespaces
Do not add `using namespace std` on a global scope of a file. If you find cumbersome to use, for instance `std::cout` in your files, you may add a `using` declaration with the members of the `std` library you wish, likewise:
```c++
using std::cout, std::cin, std::string, std::max, std::min;
```

## Header files
Function, classes and variable definitions should be written in `.hpp` header files. Their actual implementations should be written in a separate `.cpp` file, those of which should be put in distinct directories. If it is shown that C++20's modules provide a good enough substitution, we may transition into using `export` and `import`
