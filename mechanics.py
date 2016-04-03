'''This is a Phyco module deal with dynamics.
For now, it supports ,specifically, collision detection and 2-D dynamics.'''
from phycomath import linalg, genmath
import geom
from __init__ import G, epsilon0
from math import pi
from error import UnderConstruction
from string import Template


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

    reprTemplate = Template('${name}:mass=${mass},pos=${pos},v=${v}')

    def newtonianGravity(self, other, r):
        return G * self.mass * other.mass / r / r

    def coulombForce(self, other, r):
        return self.charge * other.charge / 4 / pi / epsilon0 / r / r

    def __init__(self, name, mass, initPos=linalg.vector(), point=True, mode='rigid',
                 charge=False, initv=linalg.vector(), material=None):
        self.name = name
        self.mass = mass
        self.ability = False
        self.pos = initPos
        self.velocity = initv
        self.mode = mode
        if point:
            self.shape = 0
        self.charge = 0
        if charge:
            self.charge = charge
        self.material = material

    def getforce(self, obj):
        if self.charge:
            return self.coulombForce(obj)
        return linalg.vector()

    def __repr__(self):
        return self.name

    def __str__(self):
        return obj.reprTemplate.substitute(name=self.name,
                                           mass=self.mass,
                                           pos=self.pos,
                                           v=self.velocity)


class constraint(genmath.relexpr):
    pass
