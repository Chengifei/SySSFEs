import gdb
import re
import sys
sys.path.append('/usr/share/gcc-7.2.0/python/')
import libstdcxx.v6.printers as cxxprinters
import pdb

class NVarPrinter:
    def __init__(self, val):
        self.name = val["_name"]
        self.order = val["_order"]["_M_elems"]
        self.properties = []
        if val["_can_start"]:
            self.properties.append("can_start")
        if val["_need_update"]:
            self.properties.append("need_update")
        if val["updated"]:
            self.properties.append("updated")
        if val["as_start"]:
            self.properties.append("starting")

    def to_string(self):
        order=[self.order[i] for i in range(8)]
        properties = ', '.join(self.properties)
        return f'{self.name}{properties}({", ".join(str(int(i)) for i in order)})'

class RulePrinter:
    def __init__(self, val):
        self.vars = val["vars"]
        self.unknowns = val["unknowns"]

    def to_string(self):
        printer=cxxprinters.Tr1UnorderedSetPrinter
        vp = (str(val) for _, val in printer('', self.vars).children())
        up = (str(val) for _, val in printer('', self.unknowns).children())
        return f'Rule: vars{{{", ".join(vp)}}}, unknowns {{{", ".join(up)}}}'


def dispatcher(val):
    type = val.type
    if "Variable" == val.type.name:
        return NVarPrinter(val)
    return None


def ptrvec(val):
    printer = cxxprinters.StdVectorPrinter("", val).children()
    return "{" + ", ".join(str(val.dereference()) for _, val in printer) + "}"


class pvars(gdb.Command):
    def __init__(self):
        gdb.Command.__init__ (self,
                              "pvars",
                              gdb.COMMAND_DATA,
                              gdb.COMPLETE_EXPRESSION)


    def invoke(self, args, from_tty):
        val = gdb.parse_and_eval(args)
        print(ptrvec(val))

pvars()

class ppack(gdb.Command):
    def __init__(self):
        gdb.Command.__init__ (self,
                              "ppack",
                              gdb.COMMAND_DATA,
                              gdb.COMPLETE_EXPRESSION)


    def invoke(self, args, from_tty):
        val = gdb.parse_and_eval(args)
        for _, val in cxxprinters.StdVectorPrinter("", val).children():
            print(ptrvec(val))

ppack()

def register():
    gdb.pretty_printers.append(dispatcher)

