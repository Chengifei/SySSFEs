"""User interface of PhycoE

Separate every argument with space and strictly obey the
grammar or potential data loss could take place.
Accepted keywords are:
create, plot, set, start, print, list, alias, delete, help ,p.
For help of specific command refer to themselves.

"""
import re
import sys
from code import InteractiveConsole
from error import UnderConstruction
from AhoCorasick import ACtrie

sys.ps1 = 'Phyco >'
warn = sys.stderr.write
ic = InteractiveConsole


class Interpreter(ic):

    moduledict = {'field': 'linalg', 'obj': 'mechanics', 'constraint': 'mechanics',
                  'sim': 'sim', 'event': 'sim'}

    def __init__(self):
        ic.__init__(self)
        self.assigned = {}
        self.params = set()
        self.aliastrie = ACtrie({})
        self.aliasstatus = False
        self.pool = []
        ic.push(self, 'import mechanics, sim')
        ic.push(self, 'from phycomath import linalg;from phycomath.genmath import strexpr')
        self.localcopy = list(self.locals)

    def push(self, lines):
        self.pool = lines.split(';')
        self.pool = [_line for _line in self.pool if _line]
        for line in self.pool:
            try:
                A = self.__lexer(line)
                if self.aliasstatus:
                    A = self.__translate(A)
            except:
                try:
                    warn('Wrong Syntax: %s\n' % A)
                except:
                    warn('Wrong Syntax due to unknown error\n')
                finally:
                    A = ''

            ic.push(self, A)

    def __lexer(self, line):
        """Raw command processor

        It is a lexer which will split your command
        and process and send it to python.

        """
        self.gen = (i for i in line.split())
        current = next(self.gen)
        if current == 'p':
            return ' '.join(self.gen)
        elif current in {'create', 'c'}:
            return self.create(self.gen)
        elif current in {'plot', 'p'}:
            self.plot(self.gen)
        elif current in {'set', 's'}:
            return self.set(self.gen)
        elif current == 'start':
            return self.start(self.gen)
        elif current == 'print':
            return self.print(self.gen)
        elif current == 'list':
            return self.list(self.gen)
        elif current == 'alias':
            return self.alias(self.gen)
        elif current in {'delete', 'd', 'del'}:
            return self.delete(self.gen)
        elif current in {'help', 'h'}:
            return self.help(self.gen)
        warn('Wrong Syntax:{}\n'.format(line))
        return ''

    def create(self, line):
        """create type name [attribname attrib [...]]

        All the commands input after name will be directly
        passed to __init__() of their own classes. User should
        strictly obey the grammar of their constructors. Refer
        to specific help if needed.

        """
        Type = next(line)
        name = next(line)
        if '"' in name:
            warn('Invalid name')
        if name in self.assigned:
            warn("You're about to replacing the old '{}'".format(name))
            if not self.__confirm():
                return ''
        if Type in self.moduledict:
            A = '{name}={module}.{type}({arg})'
        else:
            warn('Unknown Object {!s}\n'.format(Type))
        Arg = 'name="{name}",'.format(name=name) + ','.join(line)

        return A.format(name=name, type=Type,
                        module=self.moduledict[Type], arg=Arg)

    def assign(self, objdict):
        self.assigned.update(objdict)

    def plot(self, line):
        '''plot x y sizetuple

        plot data

        '''
        # import matplotlib
        raise UnderConstruction

    def set(self, line):
        """set name attribute [...]

        set attribution of an object

        """
        pyline = ''
        name = next(line)
        for i in line:
            pyline += name + '.{};'.format(i)
        return pyline

    def start(self, line):
        '''start name

        start simulation, one at a time

        '''
        return '{}.start({})'.format(next(line), next(line))

    def print(self, line):
        '''print name [attribname]

        print attribute or other data

        '''
        name = next(line)
        try:
            attr = next(line)
        except StopIteration:
            attr = None
        if attr:
            return 'print({}.{})'.format(name, attr)
        return 'print({})'.format(name)

    def list(self, line):
        '''list [objectname]

        list all objects in engine or list attributes of object

        '''
        print({i: self.locals[i].__class__ for i in self.locals if i not in self.localcopy})
        return ''

    def alias(self, line):
        for _eqn in line:
            self.aliastrie.add(_eqn)
        self.aliasstatus = True
        return ''

    def delete(self, line):
        '''d [objectname1 [...]]

        delete objects

        '''
        A = 'del '
        for i in line:
            del self.assigned[i]
            A += i + ','
        return A

    def help(self, line):
        exec('print(help(Interpreter._{}))'.format(next(line)))
        return ''

    def __translate(self, line):
        pass

    def __confirm(self):
        while True:
            IN = input('y/n ')
            IN = IN.casefold()
            if IN == 'y':
                return True
            elif IN == 'n':
                return False

if __name__ == '__main__':
    PHYCO = Interpreter()
    PHYCO.interact('PhycoE v0.0.0 ')
