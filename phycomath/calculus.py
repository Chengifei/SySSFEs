'''
Created on Feb 9, 2016

@author: yfzheng
'''
from phycomath.linalg import operator

class diff(operator):

    @eval
    def __call__(self, expr):
        pass

    def chain(self, expr):
        '''use chain rule to diff'''
