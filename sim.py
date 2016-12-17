"""Physics Simulator by Euler Method"""
from itertools import combinations
from math import isclose, fsum, ceil
from geom import segment
import mechanics
from phycomath import linalg


class event():

    def __init__(self, obj, name=None, *arg):
        pass


class sim():

    outputtable = str.maketrans('', '', "'{}")
    heuristiclist = [i for i in range(2, 21, 2)]

    def __init__(self, name, objs=set(), step=10 ** -1, const=set(), tol=10 ** -3):
        self.objs = objs
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
        self.t = self.__step
        self.x = [(0, dict(zip(self.objs, [obj.pos for obj in self.objs])))]
        self.v = [(0, dict(zip(self.objs, [obj.velocity for obj in self.objs])))]

        while self.t < end:
            x, v = self.compute()
            det = self.check(x, v)
            if det is True:
                self.x.append((self.t, x))
                self.v.append((self.t, v))
            elif det is False:
                self.cflag = True
                if not self.heuristic():
                    self.heuabort = True
                continue
            else:
                self.cflag, self.heuabort = False, False
                self.__step = self.step
                self.x.append((self.t, x))
                self.v.append((self.t, det))
            self.t += self.__step

    def check(self, x, v):
        # checking constraints
        if not all(self.constraints):
            return False
        # finding collision
        for o1, o2 in combinations(self.objs, 2):  # enumerate to find collision
            S1 = segment(x[o1], x[o1] + v[o1] * self.__step)
            S2 = segment(x[o2], x[o2] + v[o2] * self.__step)
            vmax = max(abs(v[o1]), abs(v[o2]))
            if S1.distance(S2) <= vmax * self.__step:
                # coplanarity critical
                det = S1.isIntersect(S2)
                if det:
                    if det is 2 or vmax * self.__step <= self.tol or self.heuabort:
                        v[o1], v[o2] = mechanics.colSolver([o1.mass, o2.mass], [v[o1], v[o2]])
                        return v
                    else:
                        return False
        return True

    def compute(self):
        x, v = self.x[-1][1].copy(), self.v[-1][1].copy()
        for Obj in self.objs:
            # compute resultant force
            F = sum((Obj.getforce(_Obj) for _Obj in self.objs), linalg.vector(0,0,0))
            a = F / Obj.mass
            # should have been improved
            x[Obj] += (v[Obj] + a.mul(self.__step) / 2).mul(self.__step)
            v[Obj] += a.mul(self.__step)
        return x, v

    def heuristic(self):
        self.t -= self.__step
        if self.heuristiclist:
            self.__step = self.step / self.heuristiclist.pop(0)
            self.t += self.__step
            return True
        self.heuristiclist = sim.heuristiclist

    def __str__(self):
        if (hasattr(self, 'x')):
            output = {}
            print(self.x)
            for obj in self.objs:
                output[obj] = [_f[0] for _f in self.x]
            X = str(output[self.objs[0]]).replace("}), (", '})\n(')
            # X = X.translate(sim.outputtable)
            return X + '\n'
        else:
            return 'This sim has never run\n' + \
            (str({i.name for i in self.objs}) if self.objs else 'empty')


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
