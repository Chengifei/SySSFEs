################################################################################
# This file is part of Configurable Simulation Framework (CSF) (formerly SAP)
# Copyright (c) 2017 by Yifei Zheng
# Unauthorized copy, distribution and modification of this file is prohibited.
#
# This is the assembler that converts bytecode into binary format
################################################################################

from collections import OrderedDict as Odict
from itertools import takewhile, chain
from ast import literal_eval
import re
import toBinary

def printErr(string):
    print("\033[91m" + string + "\033[0m")

instrToBin = {
    "NOP" : 0x00, "SQRT" : 0x00,
    "PUSHM" : 0x01, "SIN" : 0x01,
    "PUSHV" : 0x02, "COS" : 0x02,
    "PUSHI" : 0x03, "TAN" : 0x03,
    "POPM" : 0x04, "ARCSIN" : 0x04,
    "POPV" : 0x05, "ARCCOS" : 0x05,
    "VRF" : 0x06, "ARCTAN" : 0x06,
    "CALL" : 0x07, "POW" : 0x07,
    "LN" : 0x08,
    "INT" : 0x09,
    "DIFF" : 0x0a,
    "AEQ" : 0x10,
    "AEQV" : 0x11,
    "CMP" : 0x12,
    "COND" : 0x13,
    "AND" : 0x14,
    "OR" : 0x15,
    "NOT" : 0x16,
    "ADD" : 0x17,
    "SUB" : 0x18,
    "MUL" : 0x19,
    "DIV" : 0x1a,
    "NEG" : 0x1b,
    "ADV" : 0x20,
    "SBV" : 0x21,
    "MULV" : 0x22,
    "DIVV" : 0x23,
    "NEGV" : 0x24,
    "SOVSS" : 0x25,
    "SOVPS" : 0x26,
    "SOVSV" : 0x27,
    "SOVPV" : 0x28,
}


class TypeInfo:

    def __init__(self, name, size, align, members = None, writer = None):
        self.name = name # this seems redundant here ?!
        self.size = size
        assert(bin(align).count("1") is 1) # hack ?!
        self.align = align
        self.members = Odict()
        self.methods = {}
        if members:
            for name, T in members:
                if isinstance(T, list):
                    self.methods[name] = T
                else:
                    self.members[name] = T
        self.writer = writer

    @classmethod
    def fromMemberList(cls, name, L, writer = None):
        return cls(name, *cls.findSA(L), writer)

    @staticmethod
    def findSA(L): # find size and align
        sizeAcc = 0
        align = 0
        offset = []
        for key, i in L:
            if isinstance(i, str):
                try:
                    mask = typeTree[i].align - 1
                    if sizeAcc & mask:
                        sizeAcc = (sizeAcc | mask) + 1
                    offset.append(sizeAcc)
                    sizeAcc += typeTree[i].size
                    if typeTree[i].align > align:
                        align = typeTree[i].align
                except KeyError:
                    printErr(f"Error in type definition, use of unknown type '{i}'")
                    exit()
        L = [(a, (typeTree[b], c)) for (a, b), c in zip(L, offset)]
        mask = align - 1
        if sizeAcc & mask:
            sizeAcc = (sizeAcc | mask) + 1
        return sizeAcc, align, L


    def hasMember(self, name):
        return name in self.members.keys()


    def write(self, args):
        if self.writer:
            return self.writer(args)
        else: # default implementation, only for trivial types, padding bytes goes 0
            ret = b''
            LA = chain((i[1] for i in self.members.values()), [self.size]) # look ahead
            next(LA)
            for (Type, offset), ahead, arg in zip(self.members.values(), LA, args):
                ret += Type.write(arg) + bytes(ahead - offset - Type.size)
            return ret

    def __repr__(self):
        return f'{self.name}: size {self.size}, align {self.align}' \
                 + (', members ' + ', '.join(repr(i) for i in self.members) if self.members else '')


typeTree = {}

def define(name, *args):
    typeTree[name] = TypeInfo.fromMemberList(name, *args)


def builtin(name, sz, align, writer = None):
    typeTree[name] = TypeInfo(name, sz, align, writer = writer)

builtin('double', 8, 8, lambda x: toBinary.double(x))
builtin('int', 4, 4, lambda x: toBinary.integer(x))
builtin('__m256d', 32, 32, lambda x:
    toBinary.double(x[0]) + toBinary.double(x[1]) + toBinary.double(x[2]) + toBinary.double(0))

define('vector',
        [
            ("V", "__m256d"),
            ("cross", ['vector', 'vector']),
            ("proj", ['double', 'vector']),
            ("mag", ['double']),
            ("unit", ['double'])
        ], lambda x: typeTree['__m256d'].write(x))

def readDef(fileiter):
    i = next(fileiter).strip()
    i = i.split(';', 1)[0]
    ret = []
    while i != '.init:':
        if i.startswith('VEC'):
            ret.append((i[4:], 'vector'))
        else:
            ret.append((i, 'double'))
        i = next(fileiter).strip()
        i = i.split(';', 1)[0]
    ret.append(('RF', ['vector']))
    return ret


class FreeVar:

    Counter = 0

    def __init__(self, Type):
        self.Type = typeTree[Type]
        self.id = FreeVar.Counter
        FreeVar.Counter += 1

    @classmethod
    def justAlias(cls, Type):
        new = cls.__new__(cls)
        new.Type = typeTree[Type]
        new.id = None
        return new

    def __repr__(self):
        return f'{self.Type.name}, id {self.id}'


def findTypeAndOffset(start, string):
    L = string.split('.')
    last = start
    for i in L:
        nextType = last.members[i]
        if isinstance(nextType, list):
            nextType = nextType[0]
        last = typeTree[nextType]
    return last

def findVarScoped(varname):
    if varname == '$' or typeTree['object'].hasMember(varname):
        tmp = varname.split('.', 1)
        varname = tmp[0]
        if len(tmp) is 2:
            toSearch = tmp[1]
            try:
                return findTypeAndOffset('object', toSearch)
            except KeyError:
                printErr(f"Error, use of undefined member {toSearch}")
                exit(1)
        else:
            return typeTree['object'].members[varname]
    else:
        return gVars[varname]

"""
Implicit map used in writePush
indirMap = {
    "NORMAL" : 0x00
    "VAR" : 0x01,
    "MMB" : 0x02,
}

Addressing a variable:
Offset  Size    Meaning
0x00    1       Where the variable is stored (stack or storage)
0x01    4       Index of the object
0x05    4       Offset from index (Omitted for stack)
"""
def writePush(string):
    string = string.strip()
    if string.startswith('VAR'):
        varname = re.search(r'\[(.+)\]', string).group(1)
        var = findVarScoped(varname)
        if isinstance(var, FreeVar):
            ret = bytes([instrToBin['PUSHV'], var.Type.align]) + toBinary.integer(var.id)
        else:
            ret = bytes([instrToBin['PUSHM'], var[0].align]) + toBinary.integer(0) + toBinary.integer(var[1])
    else: # push immediate, encode as <ALIGN> <IMM>
        ret = bytes([instrToBin['PUSHI'], typeTree['double'].align])
        ret += toBinary.double(float(string))
    return ret

def writePop(string):
    varname = re.search(r'\[(.+)\]', string).group(1)
    try:
        var = findVarScoped(varname)
    except KeyError:
        gVars[varname] = FreeVar('double')
        var = gVars[varname]
    if isinstance(var, FreeVar):
        return bytes([instrToBin['POPV'], var.Type.align]) + toBinary.integer(var.id)
    else:
        return bytes([instrToBin['POPM'], var[0].align]) + toBinary.integer(0) + toBinary.integer(var[1])

def writeIter(string):
    string = string.strip()
    num = int(string) + 1
    writeIter.counter += max(num, 1)
    return b''

writeIter.counter = 1

def writeCall(string):
    if '.' in string:
        toSearch, method = string.rsplit('.', 1)
        if '.' in toSearch:
            raise Exception('Unimplmented')
        else:
            Type = variables[toSearch].Type
            try:
                idx = list(Type.methods.keys()).index(method)
                return bytes([instrToBin['CALL']]) + toBinary.integer(idx)
            except ValueError:
                printErr(f"Error: '{Type.name}' has no method '{method}'")
                exit()
    else:
        return bytes([instrToBin['CALL'], instrToBin[string.upper()]])

def writeMark(string):
    return b''

def writeSovs(string):
    return bytes([instrToBin['SOVS']]) + toBinary.integer(typeTree['object'].members[string][1])

registeredWriters = {
                        'PUSH' : writePush,
                        'POP'  : writePop,
                        'CALL' : writeCall,
                        'MARK' : writeMark,
                        'ITER' : writeIter,
                        'SOVS' : writeSovs
                    }

def default(T):
    if T.name == 'double':
        return 0
    else:
        return (0, 0, 0)

def init(IN):
    i = next(IN).strip()
    while i[:4] != 'PUSH':
        assert i[:7] == 'OBJECT\t', i
        gVars[i[7:]] = FreeVar('object')
        Type = typeTree['object']
        members = Type.members
        data = (i.strip()[4:] for i in takewhile(lambda x: x.strip() != 'END', IN))
        initializers = {}
        for val in data:
            lhs, rhs = val.split(',', 1)
            initializers[lhs] = literal_eval(rhs)
        data = []
        for name, T in members.items():
            data.append(initializers.get(name, default(T[0])))
        yield Type.write(data)
        i = next(IN).strip()
    while i != '.rule:':
        yield writePush(i.split('\t', 1)[1])
        i = next(IN).strip()
        i = i.split(';', 1)[0]


def rule(IN):
    i = next(IN).strip()
    while True:
        if '\t' in i:
            instr, rest = i.strip().split('\t', 1)
        else:
            instr, rest = i, ''
        if instr in registeredWriters:
            yield registeredWriters[instr](rest)
        else:
            try:
                yield bytes([instrToBin[instr]])
                yield rest.encode('ASCII')
            except KeyError:
                printErr(f"Error, unknown instruction '{instr}', the output is broken")
                exit()
        i = next(IN).strip()


if len(typeTree) > 127:
    printErr("Warning: more than 127 type defined, caution code overflow")

with open('bytecode.dbg') as IN:
    OUT = []
    gVars = {'t' : FreeVar('double')}
    try:
        IN = iter(IN)
        i = next(IN).strip()
        i = i.split(';', 1)[0]
        while not i:
            i = next(IN).strip()
            i = i.split(';', 1)[0]
        num = int(i[4:])
        OUT.append(toBinary.integer(num))
        i = next(IN).strip()
        i = i.split(';', 1)[0]
        define('object', readDef(IN))
        object_t = typeTree['object']
        OUT.append(toBinary.integer(object_t.align))
        OUT.append(toBinary.integer(object_t.size))
        OUT.append(b''.join(init(IN)))
        OUT.extend(rule(IN))
    except StopIteration:
        raise EOFError() from None
    OUT.insert(3, toBinary.integer(len(OUT[3]) - object_t.size * num))
    OUT.insert(4, toBinary.integer(writeIter.counter))


with open('a.out', 'wb') as out:
    out.writelines(OUT)
