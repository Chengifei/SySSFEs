'''
Created on Feb 4, 2016

@author: YFZHENG
'''
import collections
dict = collections.OrderedDict()

class ACtrie():

    def __init__(self, patternset):
        self.set = patternset
        T = map(len, patternset)
        self.maxlen = max(T)
        self.root = ACnode('ROOT', None, False)
        # should probably move to be class variable
        self.allnodes = {}  # should probably be a set
        self.nodesbylevel = dict.fromkeys(range(self.maxlen + 1), set())
        for i in self.set:
            self.add(i)

    def add(self, str):
        parent = self.root
        for i in str[:-1]:
            if (i, parent) not in self.allnodes:
                self.allnodes[(i, parent)] = ACnode(i, parent, str)
                self.nodesbylevel[parent.level + 1].add(self.allnodes[(i, parent)])
                parent = self.allnodes[(i, parent)]
        ACnode(str[-1], parent, corrStr = str)
        # Heretofore, all parent-child relation linked
        self.link()  # link all redirection in AC-trie

    def link(self):  # linker
        for i in self.nodesbylevel.reversed():  # make redirection from bottom
            for node in self.nodesbylevel[i]:
                for j in range(self.maxlen + 1):  # from the 1st level (not zeroth)
                    if node.char == j.char:
                        node.link(j)
                    break
                if not node.suffixset:
                    node.link(self.root)

class ACnode():

    def __init__(self, char, parent, corrStr = None):
        self.char = char
        self.parent = parent
        self.corrStr = corrStr  # corresponding string
        self.childset = set()
        self.suffixset = set()
        try:
            self.level = parent.level + 1
        except AttributeError:
            self.level = 0
        if parent:
            parent.link(self)

    def link(self, other, type = 0):  # 0 for child, 1 for suffix
        if type == 0:
            self.childset.add(other)
        elif type == 1:
            self.suffixset.add(other)

    def __eq__(self, other):
        return self.char == other.char and self.parent == other.parent

    def __hash__(self):
        return hash((self.char, self.parent))

if __name__ == '__main__':
    struct = ACtrie({'a', 'ab', 'bab', 'bc', 'bca', 'c', 'caa'})
    print(struct.nodesbylevel)
