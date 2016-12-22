'''This is the float and symbolic math processing module of PhycoE.
In normal cases, you shouldn't invoke this directly.

'''
import math
from string import Template
from .vector import vector

class matrix():

    def __init__(self, data, immutable=False):
        """define new matrix, data should be a nested
        list or tuple, with each list or tuple represents
        a column vector by default.

        """
        self.col = len(data)
        self.row = len(data[0])
        self.isSquare = self.row == self.col
        if immutable:
            self.data = tuple(data)
        else:
            self.data = data

    def __getitem__(self, *arg):
        if len(arg) == 1:
            return self.__rowSep(arg[0])
        elif len(arg) == 2:
            return self.data[arg[0]][arg[1]]  # !

    def __mul__(self, n):
        return matrix([[self[r][c] * n for c in range(self.col)] for r in range(self.row)])

    def mul(self):
        pass

    def __rowSep(self, ROW):
        row = [self.data[n][ROW] for n in range(self.row)]
        return row

    def det(self, operator=False):
        if self.isSquare:
            if operator and self.row == 3:
                temp = []
                for i in range(3):
                    ROW = [0, 1, 2]
                    ROW.remove(i)
                    t = matrix([self.__rowSep(n)[1:] for n in ROW])
                    temp.append(t)
                return temp[0].det(), -temp[1].det(), temp[2].det()
            if not operator:
                if self.row == 2:
                    return self[0][0] * self[1][1] - self[0][1] * self[1][0]
                else:
                    d = 0
                    for i in range(self.row):
                        ROW = {i for i in range(self.row)}
                        ROW.remove(i)
                        t = matrix([self.__rowSep(n)[1:] for n in ROW])
                        d += t.det()
                    return d
            else:
                raise error.MathError('Unexpected type of operation')
        else:
            raise error.MathError('Not a square matrix')

    def transpose(self):
        return matrix([self.__rowSep(n) for n in range(self.row)])

    def __iter__(self):
        for col in self.data:
            yield col

    def __repr__(self):
        A = self.transpose()
        return '\n'.join([str(col).lstrip('[').rstrip(']') for col in A])


class field():

    __slots__ = ('params', 'x', 'y', 'z', 'name')
    template = Template('field ${name}: {x: ${x}, y: ${y}, z: ${z} }')

    def __init__(self, name, x, y, z='0', *paramlist):
        self.name = name
        self.x = strexpr(x).final
        self.y = strexpr(y).final
        self.z = strexpr(z).final
        print(type(self.x))
        self.params = paramlist

    def __call__(self, pos: vector):
        _values = dict(zip(['x', 'y', 'z'], pos.list()))
        return vector(self.x(_values), self.y(_values), self.z(_values))

    def __add__(self, other):
        return field(None, self.x + other.x, self.y + other.y, self.z + other.z)

    def __sub__(self, other):
        return field(None, self.x - other.x, self.y - other.y, self.z - other.z)

    def curl(self, pos):
        return

    def grad(self, pos):
        return

    def div(self, pos):
        return

    def __repr__(self):
        return field.template.substitute(x=self.x, y=self.y,
                                         z=self.z, name=self.name)
