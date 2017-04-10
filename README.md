# ATOM - a friendly general-purpose simulation tool
ATOM is a developing simulation kit written in Python 3 and C++, designed to be simple, friendly and fully tweakable.

## What makes ATOM different from a physics engine?
ATOM is an equation-oriented code generator where physics laws are not enforced. User may control full procedure from virtually any custom interaction to the resulted motion or behavior. Don't worry, you don't have to rebuild the world... Real-world laws are provided as default.

## How about the performance of ATOM?
ATOM does not put performance as its first priority, and thus may not be quite suitable for real-time simulation. (This is a compromise between performance and flexibility.) You can expect a 2~3x speed up with manually optimized code with vectorized code and aggressive threading. In other word, automated parallelization is not implemented. But in a non-parallel environment, ATOM is quite fast already.
