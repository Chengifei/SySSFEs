'''This is a PhycoE module deal with dynamics.
For now, it supports ,specifically, collision detection and 2-D dynamics.'''
import alg
from error import UnderConstruction


def colSolver(m = None, v = None, e = 1):
        """Invoked when collision detected

        Defines an elastic collision on default.
        v should be a list with the same sequence
        as objs
        """
        # change frame of reference
        vref = v[0]  # reference velocity
        vrel = v[1] - v[0]  # relative speed
        vrelu = vrel.unitize()  # direction of relative speed
        vrel = abs(vrel)
        vrelf = vrel * e  # leaving speed
        # deal with 1-D collision
        v1fm = m[1] / (m[0] + m[1]) * (vrel + vrelf)  # magnitude of vaf
        v2fm = (m[1] * vrel - m[0] * vrelf) / (m[0] + m[1])
        v1f = vref + v1fm * vrelu
        v2f = vref + v2fm * vrelu
        return v1f, v2f


class field():

    def __init__(self, x = 0, y = 0, z = 0, dim = 3):
        self.dim = dim
        self.x = compile(str(x), '<string>', mode = 'eval')
        self.y = compile(str(y), '<string>', mode = 'eval')
        self.z = compile(str(z), '<string>', mode = 'eval')
        __slots__ = ('dim', 'x', 'y', 'z')

    def __getitem__(self, index):
        x, y, z = index.x, index.y, index.z
        return alg.vector(eval(self.x), eval(self.y), eval(self.z))

    def __add(self, other):
        return field(eval(self.x) + eval(self.y))

    def curl(self, pos):
        return

    def grad(self, pos):
        return

    def div(self, pos):
        return


class obj():

    def __init__(self, mass = 1, initPos = alg.vector(), point = True, mode = 'rigid',
                 charge = False, initv = alg.vector(), material = None):
        self.mass = mass
        self.ability = False
        self.pos = initPos
        self.mode = mode
        if point:
            self.shape = 0
        if charge:
            self.charge = charge
            self.ability = True
        self.velocity = initv
        self.material = material
        __slots__ = ('velocity', 'mass', 'pos', 'mode',
                     'shape', 'charge', 'material', 'ability')

    def fix(self):
        self.fixed = True

    def shape(self, geom):
        self.shape = geom

    def getforce(self, obj):
        if self.ability:
            return
        return alg.vector()
        raise UnderConstruction


class constraint():
    pass
