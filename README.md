# ATOM - a physics simulation compiler

## What is ATOM?
ATOM is physics simulation compiler, which compiles your specification of a particular physics system into other programming languages of your choice or into executable directly. Currently development is focused on middle end so the only front end being implemented is Python interface and the only back-end is C++.

## What makes ATOM different from a physics engine?
ATOM is not a physics engine. Rather it's an equation-oriented physics engine *generator* where the user controls the full procedure of interactions and interprets them to their effects.

## What are some differences between ATOM and Modelica?
Modelica is a great equation-based modeling language. But Modelica is more focused on system coupling and object-oriented simulation where user can define hierarchical system-subsystem relations, whereas ATOM is more focused on simulation of bunch of objects of the same level. (This doesn't mean that you can't do system coupling in ATOM!) ATOM is a consistent code generation framework while modelica is a fully-fledged declarative language. This means user can tweak code generation through passes.

## How about the performance of ATOM?
ATOM does not put performance as its first priority (but it is an important factor), and thus generated programs may not be that suitable for real-time simulation. You can expect a 2~3x speed up with manually optimized code with vectorizationand aggressive threading, but again this depends on your code. In other word, automated parallelization is not implemented (but is planned! See below). But in a non-parallel environment, ATOM is quite fast already.

If you're interested in the performance of compilation, I got to say I have no data now.

## Development Plan
1. Port back end to C++ with LLVM
2. Adopt numerically stable solvers
3. Define and document APIs
4. Provide system coupling support
5. Cooperates with LLVM vectorizer

