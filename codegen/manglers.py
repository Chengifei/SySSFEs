################################################################################
# This file is part of ATOM
# Copyright (c) 2017 by Yifei Zheng
# Unauthorized copy, distribution or modification of this file is prohibited.
#
# This module defines all runtime variable access patterns and recognizations,
# ensuring the uniformity throughout code generation.
################################################################################

import re


def prev(s):
    return s + '_prev'


def derivative(t):
    return f'{t.name}_{t.order}'


def strip_mem(s):
    r = re.compile(r'\$.+\.')
    return r.sub('', s, count=1)


def split_mem(s):
    return s[1:].split('.', 1)


def is_mem(s):
    try:
        split_mem(s)
        return True
    except ValueError:
        return False


def mem_last(s):
    idx, attr = split_mem(s)
    return f'last_data_.get(comb_.get({idx})).{attr}'


def mem_cur(s):
    idx, attr = split_mem(s)
    return f'srd_->get(comb_.get({idx})).{attr}'


def mem_prev(s, steps):
    idx, attr = split_mem(s)
    return f'back_get(history, {steps}).get(comb_.get({idx})).{attr}'


def write_var(expression):
    try:
        return mem_last(expression)
    except ValueError:
        return expression
