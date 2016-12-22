'''This is a Phyco module deal with dynamics.
For now, it supports ,specifically, collision detection and 2-D dynamics.'''
from phycomath import linalg, genmath
import geom
from math import pi
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

    __slots__ = ('name', 'mass', 'pos', 'velocity',
                 'mode', 'shape', 'charge', 'material', 'forces')

    reprTemplate = Template('${name}: mass = ${mass}, '
                            'pos = ${pos}, velocity = ${v}')

    def __init__(self, name, mass=0, pos=linalg.vector(0, 0, 0), point=True, mode='rigid',
                 charge=False, initv=linalg.vector(0, 0, 0), material=None):
        self.name = name
        self.mass = mass
        self.pos = pos
        self.velocity = initv
        self.mode = mode
        if point:
            self.shape = 0
        self.charge = 0
        if charge:
            self.charge = charge
        self.material = material
        self.forces = []

    def getforce(self, obj=None):
        # sample implementation
        return sum(i.exec({'m':self.mass, 'g': 9.8}) for i in self.forces)

    def addforce(self, string):
        self.forces.append(genmath.parse(string))

    def __str__(self):
        return obj.reprTemplate.substitute(name=self.name,
                                           mass=self.mass,
                                           pos=self.pos,
                                           v=self.velocity)

