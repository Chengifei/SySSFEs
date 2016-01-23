"""User interface of PhycoE

Separate every argument with space and strictly obey the
grammar or potential data loss could take place.
Accepted keywords are:
create, plot, set, start, print, list, alias, delete, help ,p.
For help of specific command refer to themselves.

"""

import json
import sys
from code import InteractiveConsole
from error import *
import warnings

sys.ps1 = 'Phyco >'
ic = InteractiveConsole


class Interpreter(ic):

    moduledict = {'field':'mechanics', 'obj':'mechanics', 'constraint':'mechanics',
                  'sim':'sim', 'event':'sim'}

    def __init__(self):
        ic.__init__(self)
        self.assigned = {}
        self.params = set()
        ic.push(self, 'import mechanics, sim;from alg import vector')

    def push(self, lines):
        self.pool = lines.split(';')
        for line in self.pool:
            try:
                A = self.__lexer(line)
            except:
                raise SyntaxWarning
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
            return self._create(self.gen)
        elif current in {'plot', 'p'}:
            self._plot(self.gen)
        elif current in {'set', 's'}:
            return self._set(self.gen)
        elif current == 'start':
            return self._start(self.gen)
        elif current == 'print':
            return self._print(self.gen)
        elif current == 'list':
            return self._list(self.gen)
        elif current == 'alias':
            return self._alias(self.gen)
        elif current in {'delete', 'd', 'del'}:
            return self._delete(self.gen)
        elif current in {'help','h'}:
            return self._help(self.gen)
        warnings.warn('Unknown keyword {!s}'.format(current),
                      SyntaxWarning, -1)
        return ''

    def _create(self, line):
        """create type name [attribname attrib [...]]

        All the commands input after names will be directly
        passed to __init__() of their classes. User should
        strictly obey the grammar of __init__(). Refer to
        specific help if needed.

        """
        Type = next(line)
        name = next(line)
        if name in self.assigned:
            warnings.warn('replacing the old "{}"'.format(name),
                        DuplicationWarning)
            # Should be improved to ask for permission.
        if Type in self.moduledict: A = '{name}={module}.{type}({arg})'
        else:
            raise SyntaxError('Unknown Object {!s}'.format(Type))
        self.assigned[name]=Type
        Arg = ''
        for arg in line:
            Arg += arg + ','

        return A.format(name=name, type=Type,
                        module=self.moduledict[Type], arg=Arg)

    def _plot(self, line):
        '''plot x y sizetuple

        plot data

        '''
        # import matplotlib
        raise UnderConstruction

    def _set(self, line):
        """set name attribute [...]

        set attribution of an object

        """
        pyline = ''
        name = next(line) 
        for i in line:
            pyline += name + '.{attr}={value};'.format(attr=i, value=next(line))
        return pyline

    def _start(self, line):
        '''start name

        start simulation, one at a time

        '''
        return '{}.start({})'.format(next(line),next(line))

    def _print(self, line):
        '''print name [attribname]

        print attribute or other data

        '''
        name=next(line)
        try:
            attr=next(line)
        except StopIteration:
            attr=None
        if attr:
            return 'print({}.{})'.format(name,attr)
        return 'print({})'.format(name)

    def _list(self, line):
        '''list [objectname]

        list all objects in engine or list attributes of object

        '''
        print([(i, self.assigned[i]) for i in self.assigned])
        return ''

    def _alias(self, line):
        return ''

    def _delete(self, line):
        '''d [objectname1 [...]]

        delete objects

        '''
        A = 'del '
        for i in line:
            self.assigned.remove(i)
            A += i + ','
        return A

    def _help(self,line):
        exec('print(help(Interpreter._{}))'.format(next(line)))
        return ''

if __name__ == '__main__':
    try:
        count = json.load(open('count.json'))
    except:
        count = {}
    try:
        count['interpreter'] += 1
    except:
        count['interpreter'] = 1
    json.dump(count, open('count.json', 'w'))
    PHYCO = Interpreter()
    PHYCO.interact('PhycoE v0.0.0 ')
