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

    def __init__(self, objs = None, field = None, step = 10 ** -2, end = 1, var = []):
        self.objs = objs
        # self.__comb = combinations(self.objs, 2)
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
        x = dict(zip(self.objs, [obj.pos for obj in self.objs]))
        v = dict(zip(self.objs, [obj.velocity for obj in self.objs]))
        self.X = [x.copy()]
        self.V = [v.copy()]
        # compute initial distance between each other
        d = dict(zip(combinations(self.objs, 2), [abs(x[comb[0]] - x[comb[1]]) \
                                   for comb in combinations(self.objs, 2)]))
        self.cache = (_cache([x]), _cache([d]))  # initialize cache

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

            # update distance, preparing for collision check
            d = dict(zip(combinations(self.objs, 2), [abs(x[comb[0]] - x[comb[1]])\
                                       for comb in combinations(self.objs, 2)]))
            self.cache[0].append(x)  # push newly computed data to cache
            self.cache[1].append(d)
            self.__check(d, v, x)

    def __check(self, d, v, x):
        # check constraints
        # pass
        # check collision
        D = self.cache[1].diff()
        for a in combinations(self.objs, 2):  # enumerate to find collision
            o1, o2 = a[0], a[1]
            if d[a] < max(abs(v[o1]), abs(v[o2])) * self.step and D[a] < 0:
                # two bodies are at critical distance, check if they're coplannar
                if self.__isCoplannar(x[o1], x[o2], v[o1], v[o2]):
                    # two trajectories are coplannar, check if they intersect
                    if self.__isIntersect(x[o1], self.cache[0][o1], x[o2], self.cache[0][o2]):
                        # collision detected
                        v[o1], v[o2] = mechanics.colSolver([o1.mass, o2.mass], [v[o1], v[o2]])
        self.X.append(x.copy())
        self.V.append(v.copy())

    def __isCoplannar(self, point1, point2, d1, d2):
        # could probably be included in alg.py
        """Check if two trajectories are coplannar

        If two segments are coplannar then any segment
        between two points on the lines are in their
        plane and vice versa. It uses scalar triple
        product to check if two segments are coplannar.

        """
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
                i[obj] = i[obj].list()
            i = str(i) + '\n'
        return str(X)


class _cache():
    """Imitating list to store fixed length of data

    Cache object is list which is intended to store fixed
    number of elements. Assignment or appending elements
    to saturated cache would replace the first one. It's
    not been optimized to store large amount of data. All
    of your input data should be externally verified to
    be of the same length.

    """

    def __init__(self, iterable, l = 2):
        """
        Though the usage of the class is not limited to dict,
        if you want to use this class in other ways, replace this.
        """
        self.__len = l  # NOTE: It's not the length of iterable!
        self.__list = [None] * (self.__len - len(iterable)) + list(iterable)
        self.__keys = iterable[0].keys()

    def append(self, object):
        self.__list.append(object)
        self.__list = self.__list[1:]

    def __iter__(self):
        for i in self.__list:
            yield i

    def __getitem__(self, index):
        return self.__list[-1][index]

    def diff(self):
        """compare objects stored in cache

        Substract last two entry of cache. It won't
        check if two elements are of the same length!
        """
        return {i:self.__list[-1][i] - self.__list[-2][i] for i in self.__keys}

    def __str__(self):
        return str(self.__list)
