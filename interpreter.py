#!/usr/bin/python3

import sys
from error import IncompatibleEnvironment
import traceback
import new, delete, rename, printf, ls
import mechanics, sim
from env import *

warn = sys.stderr.write


def envCheck():
    if not (sys.version_info.major >= 3 and sys.version_info.minor >= 5):
        raise IncompatibleEnvironment('Phyco requires at least Python 3.5')


envCheck()
print('PTEK Physics Thought Experiment Kit\n'
      'Copyright (c) 2015-2016 by Yifei Zheng\n'
      'There is NO WARRANTY, to the extent permitted by law. '
      'Any part of the documentation and strings in the program '
      'do not imply any authorization on copyright.')
while True:
    try:
        string = input("PTEK > ")
    except EOFError:
        print()
        exit()
    string = string.split()
    try:
        if string[0] == 'new':
            new.new_exec(*new.parse(string))
        elif string[0] == 'rename':
            rename.rename_exec(*rename.parse(string))
        elif string[0] == 'del':
            delete.del_exec(*delete.parse(string))
        elif string[0] == 'print':
            printf.print_exec(*printf.parse(string))
        elif string[0] == 'ls':
            ls.ls_exec(*ls.parse(string))
        else:
            string = str(eval(' '.join(string), namespace))
            if string != 'None':
                print(string)
    except SyntaxError as e:
        warn("Syntax Error\n")
    except Exception as e:
        exc_info = sys.exc_info()
        traceback.print_exception(*exc_info)
        del exc_info
