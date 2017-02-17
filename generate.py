#!/usr/bin/python
import re

with open('bytecodes.hpp') as source, open('bytecodes.py', 'w') as output:
    output.write('instrToBin = {\n')
    pattern = re.compile('(\w+) = (0x\w\w)')
    for line in source:
        result = re.findall(pattern, line)
        for i in result:
            output.write(f"    '{i[0]}': {i[1]},\n")
    output.write('}\n')
