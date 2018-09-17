#!/usr/bin/python3
import llvm
import os
import mmap
import re
import click
import sys
sys.path.insert(0, '../../common')
import templating
import contextlib

@contextlib.contextmanager
def write_include_guard(file, flag, name=None):
    # flag dictates whether it's written or not
    if flag:
        if name:
            file.write('#ifndef ' + name)
            file.write('#define ' + name)
            yield
            file.write('#endif // define ' + name)
        else:
            file.write('#pragma once\n')
            yield
    else: yield

def get_ctype(name):
    Bases = llvm.cTypes.BaseEnum
    DICT = {'real': Bases.REAL, 'buffer': Bases.BUFFER, 'int': Bases.INT}
    return llvm.cTypes(DICT[name], 0, 0)

module = llvm.Module(sys.stderr)

def issue_warning(msg):
    module.warn(msg)

@click.command()
@click.argument('fn', type=click.Path(exists=True))
@click.option('-o', default='a.h')
@click.option('-lib', default='libatom.a')
@click.option('--pragma-once', is_flag=True)
@click.option('--prefix', default='')
@click.option('--force', is_flag=True)
@click.option('--class')
@click.option('--struct')
def main(fn, o, lib, prefix, pragma_once, force, **kwargs):
    if kwargs:
        issue_warning('Options \x1b[91;1m' + ', '.join(kwargs) + '\x1b[0m ignored.')
    with open(fn) as f:
        lines = []
        for _, line in enumerate(f):
            if line.startswith(' '):
                lines.append(1)
            if line.strip() == 'object':
                mark1 = len(lines)
            elif line.strip() == 'init':
                mark2 = len(lines)
            elif line.strip() == 'rule':
                mark3 = len(lines)
            if line.strip():
                lines.append((_, line.strip()))

    assert mark1 < mark2 < mark3

    objdef = lines[mark1 + 1:mark2]
    initdefs = lines[mark2 + 1:mark3]
    ruledefs = lines[mark3 + 1:]
    assert objdef[0] == 1 and ruledefs[0] == 1 and initdefs[0] == 1

    fields = [(i[0], i[1].split(' ')) for i in objdef if type(i) == tuple]
    inits = [(i[0], i[1].split(' ', 1)) for i in initdefs if type(i) == tuple]
    rules = [(i[0], i[1].split('=')) for i in ruledefs if type(i) == tuple]

    # prepare arguments for C++ call
    for line, (i, j) in fields:
        module.add_field(get_ctype(i), j)
    for line, (i, j) in inits:
        if i == 'object':
            i, j = j.split('=', 1)
            module.add_obj(i.strip(), *j.split(','))
        else: # i == 'ctrl'
            i, j = j.split('=', 1)
            module.add_ctrl(i.strip(), *j.split(':', 2))
    for line, (i, j) in rules:
        module.add_rule((line, 0), f'({i}) - ({j})')
    llvm.write(module, None, lib.encode('ascii'))
    # write C header
    header = templating.template('../../backends/c/template')
    # FIXME: prepare the text before write to file
    with open(o, 'w' if force else 'x') as f, write_include_guard(f, pragma_once):
        # enumerate is not necessary here, while it may be necessary later if we
        # add support for named rules
        consume = templating.consume
        f.writelines(consume(header, ['struct object {']))
        f.writelines(f'    {type.get_C_type() % name.decode("UTF8")};\n' for type, name in fields)
        f.write('};\n')
        f.writelines(f'double {prefix}_solver_rule_{i}();\n' for i, j in enumerate(rules))
        f.writelines(i for i in header)

if __name__ == '__main__':
    main()
