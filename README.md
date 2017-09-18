# ATOM - a friendly general-purpose simulation tool
ATOM is a developing simulation kit written in Python 3 and C++, designed to be simple, friendly and fully tweakable.

## What makes ATOM different from a physics engine?
ATOM is an equation-oriented physics engine *generator* where physics laws are not hard-coded. User may control full procedure from virtually any custom interaction to the resulted motion or behavior. Real-world laws are only provided as default.

## What's the difference between ATOM and Modelica then?
Modelica is a great equation-based modeling language. But Modelica is more focused on system coupling and object-oriented simulation where user can define hierarchical system-subsystem relations, whereas ATOM is more focused on simulation of bunch of objects of the same level. ATOM is a consistent code generator while modelica is a fully-fledged declarative language. This means user can directly tweak ATOM generated code (preferably through patch).

## How about the performance of ATOM?
ATOM does not put performance as its first priority (but it is an important factor), and thus may not be that suitable for real-time simulation. You can expect a 2~3x speed up with manually optimized code with vectorized code and aggressive threading. In other word, automated parallelization is not implemented. But in a non-parallel environment, ATOM is quite fast already.
