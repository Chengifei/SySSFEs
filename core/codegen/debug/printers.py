# Copyright 2017-2018 by Yifei Zheng
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import gdb
import re
import sys
sys.path.append('/usr/share/gcc-7.3.1/python/')
import libstdcxx.v6.printers as cxxprinters
from boost.flat_containers import FlatMap165Printer as BoostFlatMapBase
import pdb

class variable_designationPrinter:
    def __init__(self, val):
        self.id = val["id"]
        self.order = val["order"]["_M_elems"]

    def to_string(self):
        order=((str(i + 1), int(self.order[i])) for i in range(8))
        prime = "'"
        return f'<{self.id} [{" ".join(i + prime*j for i, j in order if j)}]>'

class UpdatePairPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return str(self.val["first"].dereference()) + " " + str(self.val["second"])

class PackedVarsPrinter:
    def __init__(self, val):
        self.order = cxxprinters.StdVectorPrinter("", val["orders"]).children()
        self.var = cxxprinters.StdVectorPrinter("", val["states"]).children()

    @staticmethod
    def generator(it1, it2):
        prime = "'"
        for i, j in zip(it1, it2):
            order = i[1]["_M_elems"]
            order = [(str(i + 1), int(order[i])) for i in range(8)]
            order = " ".join(i + prime*j for i, j in order if j)
            if not order:
                order = "base"
            yield order, j[1]

    def children(self):
        return self.generator(self.order, self.var)

    def to_string(self):
        return 'packed_vars'

class RulePrinter:
    def __init__(self, val):
        self.val = val

    def children(self):
        vec = self.val[self.val.type.fields()[0]]
        return cxxprinters.StdVectorPrinter("Rule", vec).children()

    def to_string(self):
        if int(self.val["enabled"]):
            return "Rule"
        else:
            return "*Rule*"

    def display_hint(self):
        return 'array'

class SlnPrinter:
    def __init__(self, val):
        self.rule = val["rule"]['_M_t']['_M_t']['_M_head_impl']
        self.rule_end = val["rule_end"]
        self.vars = val["var"]['_M_t']['_M_t']['_M_head_impl']

    def children(self):
        while self.rule != self.rule_end:
            yield str(self.vars.dereference()), self.rule.dereference().dereference()
            self.rule += 1
            self.vars += 1

    def to_string(self):
        return "step"

def dispatcher(val):
    if "step" == val.type.name:
        return SlnPrinter(val)
    elif "Rule" == val.type.name:
        return RulePrinter(val)
    elif "variable_designation" == val.type.name:
        return variable_designationPrinter(val)
    elif "std::pair<std::vector<variable_designation, std::allocator<variable_designation> >*, variable_designation>" == val.type.name:
        return UpdatePairPrinter(val)
    elif "variable_pool::packed_vars" == val.type.name:
        return PackedVarsPrinter(val)
    return None


def ptrvec(val):
    printer = cxxprinters.StdVectorPrinter("", val).children()
    return "{" + ", ".join(str(val.dereference()) for _, val in printer) + "}"


class ppool(gdb.Command):
    def __init__(self):
        gdb.Command.__init__ (self,
                              "ppool",
                              gdb.COMMAND_DATA,
                              gdb.COMPLETE_EXPRESSION)

    @staticmethod
    def generator(it):
        for _, i in it:
            yield (i, next(it)[1])

    def invoke(self, args, from_tty):
        val = gdb.parse_and_eval("pool.pool")
        for key, val in self.generator(BoostFlatMapBase(val).children()):
            print(key, val)

ppool()

class prules(gdb.Command):
    def __init__(self):
        gdb.Command.__init__ (self,
                              "prules",
                              gdb.COMMAND_DATA,
                              gdb.COMPLETE_EXPRESSION)

    def invoke(self, args, from_tty):
        val = gdb.parse_and_eval("pack")
        for _, val in cxxprinters.StdVectorPrinter("", val).children():
            print(val)

prules()

def register():
    gdb.pretty_printers.append(dispatcher)

