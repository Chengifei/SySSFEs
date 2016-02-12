'''This is a PhycoE module deal with dynamics.
For now, it supports ,specifically, collision detection and 2-D dynamics.'''
import linalg, geom
from __init__ import G, epsilon0
from math import pi
from error import UnderConstruction


def colSolver(m=None, v=None, e=1):
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


class obj():

    __slots__ = ('name', 'mass', 'ability', 'pos', 'velocity',
                 'mode', 'shape', 'charge', 'material')

    def newtonianGravity(self, other, r):
        return G * self.mass * other.mass / r / r

    def coulombForce(self, other, r):
        return self.charge * other.charge / 4 / pi / epsilon0

    def magneticForce(self):
        pass

    def __init__(self, name=None, mass=1, initPos=linalg.vector(), point=True, mode='rigid',
                 charge=False, initv=linalg.vector(), material=None):
        self.name = name
        self.mass = mass
        self.ability = False
        self.pos = initPos
        self.velocity = initv
        self.mode = mode
        if point:
            self.shape = 0
        if charge:
            self.charge = charge
            self.ability = True
        self.material = material

    def getforce(self, obj):
        if self.ability:
            return
        return linalg.vector()
        raise UnderConstruction

    def __repr__(self):
        return self.name


class constraint():
    pass
