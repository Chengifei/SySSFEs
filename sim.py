"""Physics Simulator by Euler Method"""
from itertools import combinations
import alg
from geom import segment
import mechanics
import pdb

class event():

    def __init__(self, mode='occurred', *arg):
        if mode == 'occurred':
            pass
        elif mode == 'pass':
            pass

class sim():

    outputtable = str.maketrans('', '', "'{}[]")
    heuristiclist = [i for i in range(2, 11)]

    def __init__(self, objs, name=None, field=None, step=10 ** -1,
                 const=None, allowance=10 ** -2):
        # name should be aligned to be compatible with objs
        self.objs = objs
        self.allowance = allowance
        self.field = field
        self.step, self.__step = step, step
        self.cflag = False
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
                if self.heuristic():
                    self.i -= 1
                else:
                    self.x.append(x)
                    self.v.append(v)
            else:
                self.cflag = False
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
                    if det == 2 or vmax * self.__step <= self.allowance:
                        v[o1], v[o2] = mechanics.colSolver([o1.mass, o2.mass], [v[o1], v[o2]])
                        return v
                    else:
                        return False
        return True

    def compute(self):
        x, v = self.x[-1].copy(), self.v[-1].copy()
        for Obj in self.objs:
            try:
                F = sum([obj.getforce(Obj) for obj in self.objs if obj != Obj])\
                + self.field[x[Obj]]  # compute resultant force
            except TypeError:
                F = alg.vector()
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
        X = str(self.x).replace("}, {", '\n')
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
