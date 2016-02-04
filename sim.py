"""Physics Simulator by Finite Difference Method (FDM)"""
from itertools import combinations
from error import UnderConstruction
import sys
import alg, geom
import mechanics


class event():

    def __init__(self, mode = 'occurred', *arg):
        if mode == 'occurred':
            pass
        elif mode == 'pass':
            pass


class sim():
    """FDM solver"""

    def __init__(self, objs, name = None, field = None, step = 10 ** -1, var = []):
        # name should be aligned to be compatible with objs
        self.objs = objs
        # self.__comb = combinations(self.objs, 2)
        self.map = dict(zip(objs, name))
        self.field = field
        self.step = step
        self.constraints = {}
        __slots__ = ('objs', 'comb', 'field', 'step', 'end',
                     'constriants', 'cache', 'X', 'V')

    def start(self, end):
        """
        WARNING: Using dict to store all combinations of two objects,
        data loss would happen when too many objects encountered.
        """
        dt = self.step
        i = int(end / dt)  # number of steps needed
        print (i)
        x = dict(zip(self.objs, [obj.pos for obj in self.objs]))
        v = dict(zip(self.objs, [obj.velocity for obj in self.objs]))
        self.X = [x.copy()]
        self.V = [v.copy()]
        # compute initial distance between each other

        for i in range(i):
            for Obj in self.objs:
                pos = x[Obj]
                try:
                    F = sum([obj.getforce(Obj) for obj in self.objs if obj != Obj])\
                    + self.field[pos]  # compute resultant force
                except:
                    F = alg.vector()
                a = F / Obj.mass
                x[Obj] += (v[Obj] + a * self.step / 2) * dt
                # should have been improved
                v[Obj] += a * dt

            self.__check(v, x)

    def __check(self, v, x):
        # check constraints
        # pass
        # check collision
        for a in combinations(self.objs, 2):  # enumerate to find collision
            o1, o2 = a[0], a[1]
            S1 = geom.segment(x[o1], x[o1] + v[o1] * self.step)
            S2 = geom.segment(x[o2], x[o2] + v[o2] * self.step)
            if S1.distance(S2) <= max(abs(v[o1]), abs(v[o2])) * self.step:  # coplanarity
                if S1.isIntersect(S2):
                    # collision detected
                    v[o1], v[o2] = mechanics.colSolver([o1.mass, o2.mass], [v[o1], v[o2]])
        self.X.append(x.copy())
        self.V.append(v.copy())

    def __isCoplannar(self, point1, point2, d1, d2):
        """Check if two trajectories are coplanar"""
        D = point1 - point2
        A = alg.matrix([d1, D, d2])
        if A.det() == 0:
            return True
        return False

    def __isIntersect(self, p1, p2, p3, p4):
        """Check if p1p2 and p3p4 intersects

        Iif two segments intersects

        """
        AB = p1 - p2
        AC = p1 - p3
        AD = p1 - p4
        if AB.cross(AC).dot(AB.cross(AD)) <= 0:
            return True
        return False

    def __str__(self):
        X = self.X
        for i in X:
            for obj in self.objs:
                i[self.map[obj]] = i[obj].list()
                del i[obj]
        return str(X)
