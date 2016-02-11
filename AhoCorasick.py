"""Aho-Corasick Automaton for string pattern-match

This is used both in phycomath.genmath.expr and
integrated interpreter for aliasing.

"""
from pdb import set_trace
# import collections
# dict = collections.OrderedDict()


class ACtrie():


    def __init__(self, patternset):
        self.set = patternset
        self.maxlen = len(max(patternset, key=len))
        self.minlen = len(min(patternset, key=len))
        self.root = ACnode('ROOT', None, False)
        # should probably move to be class variable
        self.nodes = set()
        self.links = set()
        self.inPattern = set()
        self.nodesByLevel = [set() for i in range(self.maxlen + 1)]
        # self.root not included
        self.curnode = self.root
        for i in self.set:
            self.add(i)
        self.linkRedir()  # link all redirection in AC-trie
        self.linkPara()  # link all parallel match

    def add(self, str):  # WARNING:can be called without setting self.max/minlen
        parent = self.root
        for i in str[:-1]:
            _node = ACnode(i, parent)
            if _node not in self.nodes:
                self.nodes.add(_node)
                self.nodesByLevel[parent.level + 1].add(_node)
            parent = _node
        _node = ACnode(str[-1], parent, str)  # mark end of string
        self.nodes.add(_node)
        self.inPattern.add(_node)
        self.nodesByLevel[parent.level + 1].add(_node)
        # Heretofore, all parent-child relation linked

    def linkRedir(self):  # redirection linker
        for node in self.nodesByLevel[1]:
            node.link(self.root)
            self.links.add((node, 'ROOT'))
        for i in range(self.maxlen, 1, -1):  # link redirection from bottom
            for node in self.nodesByLevel[i]:
                for j in range(1, i):  # from the 1st level (not zeroth)
                    for Node in self.nodesByLevel[j]:
                        if node.char == Node.char:
                            node.link(Node)
                            self.links.add((node, Node))
                            break
                    break
                break
            if not node.redirect:
                    self.links.add((node, 'ROOT'))
                    node.link(self.root)

    def linkPara(self):  # parallel match linker
        pass

    def process(self, str):
        level = 0  # at root
        for i in str:
            if i in self.nodesByLevel[level + 1]:
                level += 1
        pass


class ACnode():

    def __init__(self, char, parent, endOfStr=False):
        self.char = char
        self.parent = parent
        self.isEndOfStr = endOfStr  # corresponding string
        self.children = set()
        self.redirect = None  # go to self.redirect if match fails
        self.parallel = None  # one node can have only one but can be linked
        # self.repr = str(self.char) + str(self.parent)  # guaranteed correct
        self.repr = repr(self.parent) + self.char
        try:
            self.level = parent.level + 1
        except AttributeError:
            self.level = 0
        if parent:
            parent.link(self)

    def link(self, other, type=1):  # 0 for child, 1 for redirection
        if type:
            self.redirect = other  # link to
        elif type == 1:
            self.children.add(other)  # link to
        else:
            self.parallel = other  # bidirection

    def __repr__(self):
        if self.char == 'ROOT':
            return ''
        return self.repr

    def __eq__(self, other):
        return self.repr == other.repr

    def isParallel(self, other):
        return self.char == other.char

    def __hash__(self):
        return hash(self.repr)

if __name__ == '__main__':
    # struct = ACtrie({'a', 'ab', 'bab', 'bc', 'bca', 'c', 'caa'})
    struct = ACtrie({'caa', 'bca'})
    print(struct.links)
