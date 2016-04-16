"""User interface of PhycoE

Separate every argument with space and strictly obey the
grammar or potential data loss could take place.
Accepted keywords are:
create, plot, set, start, print, list, alias, delete, help ,p.
For help of specific command refer to themselves.

"""
from __future__ import generator_stop
# import re
import sys
from code import InteractiveConsole
from error import UnderConstruction
from AhoCorasick import ACProcessor as ACtrie
from __init__ import envCheck
from pdb import set_trace

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
        self.aliastrie = ACtrie({}, reduced=True)
        self.aliasstatus = False
        self.pool = []
        ic.push(self, 'import mechanics, sim')
        ic.push(self, 'from phycomath import linalg')
        ic.push(self, 'vector=linalg.vector')
        self.localcopy = list(self.locals)

    def push(self, lines):
        self.pool = lines.split(';')
        self.pool = [_line for _line in self.pool if _line]
        for line in self.pool:
            try:
                pyline = self.__lexer(line)
                if self.aliasstatus:
                    pyline = self.__translate(pyline)
            except Exception as e1:
                try:
                    warn('Wrong Syntax: %s\n' % pyline)
                    print(e1)
                except Exception as e2:
                    warn('Wrong Syntax due to unknown error\n')
                    print(e2)
                finally:
                    pyline = ''

            ic.push(self, pyline)

    def __lexer(self, line):
        """Raw command processor

        It is a lexer which will split your command
        and process and send it to python.

        """
        self.gen = (i for i in line.split())
        command = next(self.gen)
        if command == 'p':
            return ' '.join(self.gen)
        elif command in {'create', 'new'}:
            return self.create(self.gen)
        elif command in {'plot'}:
            self.plot(self.gen)
        elif command in {'set', 's'}:
            return self.set(self.gen)
        elif command == 'start':
            return self.start(self.gen)
        elif command == 'print':
            return self.print(self.gen)
        elif command == 'list':
            return self.list(self.gen)
        elif command == 'alias':
            return self.alias(self.gen)
        elif command in {'delete', 'd', 'del'}:
            return self.delete(self.gen)
        elif command in {'help', 'h'}:
            return self.help(self.gen)
        return ''

    def create(self, line):
        """create type _name [attribname attrib [...]]

        All the commands input after _name will be directly
        passed to __init__() of their own classes. User should
        strictly obey the grammar of their constructors. Refer
        to specific help if needed.

        """
        Type = next(line)
        name = next(line)
        namelist = name.split(',')
        _name = namelist.pop(0)
        if '"' in _name:
            warn('Invalid _name')
        if _name in self.assigned:
            warn("You're about to replacing the old '{}'".format(_name))
            if not self.__confirm():
                warn('Aborting, everything left untouched')
                return ''
        if Type in self.moduledict:
            template = '{name}={module}.{type}({arg})'
        else:
            warn('Unknown Object {!s}\n'.format(Type))
        Arg = 'name="{name}",'.format(name=_name) + ','.join(line)

        B = template.format(name=_name, type=Type,
                            module=self.moduledict[Type], arg=Arg) + ';'

        for _name in namelist:
            B += template.format(name=_name, type=Type,
                                 module=self.moduledict[Type], arg=Arg) + ';'

        return B

    def plot(self, line):
        '''plot x y sizetuple
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
        except RuntimeError:
            attr = None
        if attr:
            return 'print({}.{})'.format(name, attr)
        return 'print({})'.format(name)

    def list(self, line):
        '''list [objectname]

        list all objects in engine or list attributes of object

        '''
        try:
            print({i: self.locals[i].__class__
                   for i in self.locals
                   if i not in self.localcopy and
                   isinstance(i, self.locals[next(line)])})
        except KeyError:
            warn('Requesting nonexisting type\n')
        except StopIteration:
            print({i: self.locals[i].__class__
                   for i in self.locals if i not in self.localcopy})
        return ''

    def alias(self, line):
        for _eqn in line:
            _eqn = _eqn.split('=')
            _ = set(self.aliastrie.patterns)
            _.add(_eqn[0])
            self.aliastrie = ACtrie(_, reduced=True)
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
        self.aliastrie(line)
        print(self.aliastrie.record)
        return ''

    def __confirm(self):
        while True:
            IN = input('y/n ')
            IN = IN.casefold()
            if IN == 'y':
                return True
            elif IN == 'n':
                return False

if __name__ == '__main__':
    envCheck()
    PHYCO = Interpreter()
    PHYCO.interact('PhycoE v0.0.0')
