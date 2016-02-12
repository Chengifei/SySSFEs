"""Aho-Corasick Automaton for string pattern-match

This is used both in phycomath.genmath.expr and
integrated interpreter for aliasing.

"""
# import collections
# dict = collections.OrderedDict()


class ACtrie():


    def __init__(self, patternset):
        self.set = patternset
        self.maxlen = len(max(patternset, key=len))
        self.minlen = len(min(patternset, key=len))
        self.root = ACnode('ROOT', None)
        # should probably move to be class variable
        self.nodes = set()
        self.links = set()
        self.inPattern = set()
        self.nodesByLevel = [set() for i in range(self.maxlen + 1)]
        for i in self.set:
            self.add(i)
        # refrom self.nodes
        self.nodes = []
        for _set in self.nodesByLevel:
            self.nodes.extend(_set)
        self.linkSuffix()  # link all suffix in AC-trie
        self.linkPara()  # link all parallel match

    def add(self, str):  # WARNING:may be called without setting self.max/minlen
        parent = self.root
        _node = ACnode(str[0], parent, str)
        _node.link(self.root)
        self.links.add((_node, 'ROOT'))
        self.nodes.add(_node)
        self.nodesByLevel[parent.level + 1].add(_node)
        parent = _node
        for i in str[1:-1]:
            _node = ACnode(i, parent)
            if _node not in self.nodes:
                self.nodes.add(_node)
                self.nodesByLevel[parent.level + 1].add(_node)
            parent = _node
        _node = ACnode(str[-1], parent, str)  # mark end of string
        self.nodes.add(_node)
        self.inPattern.add(_node)
        self.nodesByLevel[parent.level + 1].add(_node)
        # Heretofore, all parent-child, ROOT-first suffix relations linked

    def linkSuffix(self):  # suffix linker
        for node in self.nodes:
            _node = node.parent.parent
            while not node.suffix:
                _target = _node.hasChild(node)
                if _target:
                    node.link(_target)
                    self.links.add((node, _target))
                    break
                if _node.suffix:
                    _node = _node.suffix
                else:
                    node.link(self.root)
                    self.links.add((node, 'ROOT'))

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
        self.suffix = None
        self.parallel = None  # one node can have only one but can be linked
        # self.repr = str(self.char) + str(self.parent)  # guaranteed correct
        self.repr = repr(self.parent) + self.char
        try:
            self.level = parent.level + 1
        except AttributeError:
            self.level = 0
        if parent:
            parent.link(self, 0)

    def link(self, other, type=1):  # 0 for child, 1 for suffix, 2 for dictsuffix
        if type == 1:
            self.suffix = other
        elif type == 0:
            self.children.add(other)
        else:
            self.parallel = other  # bidirection

    def isOnBranch(self, str):
        return repr(self).lstrip(str) != repr(self)

    def hasChild(self, node):
        for i in self.children:
            if i.isSameChar(node):
                return i

    def __repr__(self):
        if self.char == 'ROOT':
            return ''
        return self.repr

    def __eq__(self, other):
        return self.repr == other.repr

    def isSameChar(self, other):
        return self.char == other.char

    def __hash__(self):
        return hash(self.repr)

if __name__ == '__main__':
    struct = ACtrie({'aba', 'cab'})
    print(struct.links)
