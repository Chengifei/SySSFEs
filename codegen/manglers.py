################################################################################
# This file is part of ATOM
# Copyright (c) 2017 by Yifei Zheng
# Unauthorized copy, distribution or modification of this file is prohibited.
#
# This file defines all (de)manglers (variables name manipulating functions)
# used for code generation.
################################################################################

def prev(s):
    return s + '_prev'

def derivative(t):
    return f'{t.name}_{t.order}'
