# ATOM - a friendly general-purpose simulation tool
ATOM is a developing simulation kit written in Python 3 and C++, designed to be simple, friendly and fully tweakable.

## What makes ATOM different from a physics engine?
ATOM is an equation-oriented physics engine *generator* where physics laws are not hard-coded. Instead, user may control full procedure of any custom interactions and interpret those to their resulted motion or behavior. Real-world laws are only provided as default.

## What's the difference between ATOM and Modelica then?
Modelica is a great equation-based modeling language. But Modelica is more focused on system coupling and object-oriented simulation where user can define hierarchical system-subsystem relations, whereas ATOM is more focused on simulation of bunch of objects of the same level. (This doesn't mean that you can't do system coupling in ATOM!) ATOM is a consistent code generation framework while modelica is a fully-fledged declarative language. This means user can directly tweak ATOM generated code externally (through patch) or internally (through API).

## How about the performance of ATOM?
ATOM does not put performance as its first priority (but it is an important factor), and thus generated programs may not be that suitable for real-time simulation. You can expect a 2~3x speed up with manually optimized code with vectorizationand aggressive threading, but again this depends on your code. In other word, automated parallelization is not implemented (but is planned! See below). But in a non-parallel environment, ATOM is quite fast already.

If you're interested in the performance of compilation, I got to say I have no data now.

## Development Plan
1. Port middle and back end to C++ with LLVM
2. Adopt numerically stable solvers
3. Define and document APIs
4. Provide system coupling support
5. Cooperates with LLVM vectorizer

