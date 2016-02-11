'''
Created on Feb 4, 2016

@author: YFZHENG
'''
from pdb import set_trace
# import collections
# dict = collections.OrderedDict()


class ACtrie():

    def __init__(self, patternset):
        self.set = patternset
        _len = map(len, patternset)
        self.maxlen = max(_len)
        self.root = ACnode('ROOT', None, False)
        # should probably move to be class variable
        self.allnodes = set()
        self.nodesbylevel = [set() for i in range(self.maxlen + 1)]
        # self.root not included
        self.curnode = self.root
        for i in self.set:
            self.add(i)

    def add(self, str):
        parent = self.root
        for i in str[:-1]:
            # set_trace()
            _T = ACnode(i, parent)
            if _T not in self.allnodes:
                self.allnodes.add(_T)
                self.nodesbylevel[parent.level + 1].add(_T)
                parent = _T
        self.allnodes.add(ACnode(str[-1], parent, str))  # mark end of string
        # Heretofore, all parent-child relation linked
        self.link()  # link all redirection in AC-trie

    def link(self):  # linker
        for node in self.nodesbylevel[1]:
            node.link(self.root)
        for i in range(self.maxlen, 1, -1):  # link redirection from bottom
            for node in self.nodesbylevel[i]:
                for j in range(1, i):  # from the 1st level (not zeroth)
                    for Node in self.nodesbylevel[j]:
                        if node.char == Node.char:
                            node.link(Node)
                            break
                    break
                if not node.redirect:
                    print(node, 'linked to root')
                    node.link(self.root)

    def proc(self, str):
        level = 0  # at root
        for i in str:
            if i in self.nodesbylevel[level + 1]:
                level += 1
        pass


class ACnode():

    def __init__(self, char, parent, endOfStr=False):
        self.char = char
        self.parent = parent
        self.isEndOfStr = endOfStr  # corresponding string
        self.children = set()
        self.redirect = None
        self.repr = str(self.char) + str(self.parent)
        try:
            self.level = parent.level + 1
        except AttributeError:
            self.level = 0
        if parent:
            parent.link(self)

    def link(self, other, type=1):  # 0 for child, 1 for redirection
        if type:
            self.redirect = other
        else:
            self.children.add(other)

    def __repr__(self):
        if self.char == 'ROOT':
            return ''
        return self.repr

    def __eq__(self, other):
        print(hash(self) - hash(other))
        return self.repr == other.repr

    def __hash__(self):
        return hash(self.repr)

if __name__ == '__main__':
    # struct = ACtrie({'a', 'ab', 'bab', 'bc', 'bca', 'c', 'caa'})
    struct = ACtrie({'a', 'ab', 'bab', 'bc'})
    print(struct.allnodes)
