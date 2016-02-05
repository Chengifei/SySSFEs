"""Physics Simulator by Euler Method"""
from itertools import combinations
from error import UnderConstruction
import alg, geom
import mechanics


class event():

    def __init__(self, mode='occurred', *arg):
        if mode == 'occurred':
            pass
        elif mode == 'pass':
            pass


class sim():

    def __init__(self, objs, name=None, field=None, step=10 ** -1, var=[], const=None):
        # name should be aligned to be compatible with objs
        self.objs = objs
        # self.__comb = combinations(self.objs, 2)
        self.map = dict(zip(objs, name))
        self.field = field
        self.step = step
        self.constraints = set(const)
        __slots__ = ('objs', 'field', 'step', 'end',
                     'constriants', 'cache', 'X', 'V')

    def start(self, end):
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
                    # discard change
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


class _cache():
    """Imitating list to store fixed length of data

    Cache object is list which is intended to store fixed
    number of elements. Assignment or appending elements
    to saturated cache would replace the first one. It's
    not been optimized to store large amount of data. All
    of your input data should be externally verified to
    be of the same length.

    """

    def __init__(self, iterable, l=2):
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
