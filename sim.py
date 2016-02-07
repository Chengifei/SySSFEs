"""Physics Simulator by Euler Method"""
from itertools import combinations
from linalg import vector
from math import isclose, fsum
from geom import segment
from pdb import set_trace
import mechanics

class event():

    def __init__(self, obj, *arg):
        pass

class sim():

    outputtable = str.maketrans('', '', "'{}")
    heuristiclist = [i for i in range(2, 11)]

    def __init__(self, objs, name=None, step=10 ** -1, field=None,
                 const=None, tol=10 ** -2):
        # name should be aligned to be compatible with objs
        self.objs = objs
        self.field = field
        self.tol = tol  # tolerance
        self.step, self.__step = step, step
        self.cflag = False  # critical flag
        self.heuabort = False  # heuristic abortion flag
        if const:
            self.constraints = set(const)
        else:
            self.constraints = set()

    def start(self, end):
        '''sim.start(end) -> None'''
        self.i = 0
        n = int(end / self.step)  # number of steps needed
        self.x = [dict(zip(self.objs, [obj.pos for obj in self.objs]))]
        self.v = [dict(zip(self.objs, [obj.velocity for obj in self.objs]))]

        while self.i < n:
            x, v = self.compute()
            det = self.check(x, v)
            if det == 1:
                self.x.append(x)
                self.v.append(v)
            elif det == 0:
                self.cflag = True
                if not self.heuristic():
                    self.heuabort = True
                self.i -= 1
            else:
                self.cflag, self.heuabort = False, False
                self.__step = self.step
                self.x.append(x)
                self.v.append(det)
            self.i += 1

    def check(self, x, v):
        # check constraints
        f = map(eval, self.constraints)
        if not all(f):
            return False
        for o1, o2 in combinations(self.objs, 2):  # enumerate to find collision
            S1 = segment(x[o1], x[o1] + v[o1] * self.__step)
            S2 = segment(x[o2], x[o2] + v[o2] * self.__step)
            vmax = max(abs(v[o1]), abs(v[o2]))
            if S1.distance(S2) <= vmax * self.__step:
                # coplanarity critical
                det = S1.isIntersect(S2)
                if det:
                    if det == 2 or vmax * self.__step <= self.tol or self.heuabort:
                        v[o1], v[o2] = mechanics.colSolver([o1.mass, o2.mass], [v[o1], v[o2]])
                        return v
                    else:
                        return False
        return True

    def compute(self):
        x, v = self.x[-1].copy(), self.v[-1].copy()
        for Obj in self.objs:
            # F = Obj.getforce(self.objs[1])  # compute resultant force
            F = vector(0, -10)
            a = F / Obj.mass
            # should have been improved
            x[Obj] += (v[Obj] + a * self.__step / 2) * self.__step
            v[Obj] += a * self.__step
        return x, v

    def heuristic(self):
        if self.heuristiclist:
            self.__step = self.step / self.heuristiclist.pop(0)
            return True
        self.heuristiclist = sim.heuristiclist
        return False

    def __str__(self):
        output = {}
        for obj in self.objs:
            output[obj] = [_f[obj] for _f in self.x]
        X = str(output).replace("}, {", '\n')
        X = X.translate(sim.outputtable)
        return X + '\n'


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

    def rollback(self):
        self.__list.insert(None, 0)
        self.__list = self.__list[:-1]

    def __getitem__(self, index):
        return self.__list[-1][index]

    def __str__(self):
        return str(self.__list)
