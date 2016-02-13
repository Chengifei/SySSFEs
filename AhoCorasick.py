"""Aho-Corasick Automaton for string pattern-match

This is used both in phycomath.genmath.expr and
integrated interpreter for aliasing.

"""
from pdb import set_trace


class ACtrie():

    def __init__(self, patternset):
        self.set = patternset
        self.maxlen = len(max(patternset, key=len))
        self.minlen = len(min(patternset, key=len))
        self.root = ACnode('ROOT', None)
        # should probably move to be class variable
        self.record = {}
        self.links = set()
        self.nodesByLevel = [{} for i in range(self.maxlen + 1)]
        for i in self.set:
            self.add(i)
        self.nodes = [i[j] for i in self.nodesByLevel for j in i]
        self.linkSuffix()  # link all suffix in AC-trie
        # self.linkDict()  # link all parallel match

    def add(self, _str):  # WARNING:danger of without setting self.max/minlen
        self.record[_str] = 0  # initialize self.record
        if len(_str) == 1:
            parent = self.root
            _node = ACnode(_str[0], parent, True, corrStr=_str)
            _node.link(self.root)
            self.links.add((_node, 'ROOT'))
            self.nodesByLevel[1][_node.repr].add(_node)
        else:
            parent = self.root
            _node = ACnode(_str[0], parent)
            _node.link(self.root)
            self.links.add((_node, 'ROOT'))
            self.nodesByLevel[parent.level + 1][_node.repr] = _node
            parent = _node
            for i in _str[1:-1]:
                _node = ACnode(i, parent)
                if _node.repr not in self.nodesByLevel[parent.level + 1]:
                    self.nodesByLevel[parent.level + 1][_node.repr] = _node
                parent = _node
            _node = ACnode(_str[-1], parent, True, corrStr=_str)
            self.nodesByLevel[parent.level + 1][_node.repr] = _node
        # Heretofore, all parent-child, ROOT-first suffix relations linked

    def linkSuffix(self):  # suffix linker
        for node in self.nodes:
            _node = node.parent.suffix
            set_trace()
            while not node.suffix:
                _target = _node.charInChildren(node.char)
                if _target:
                    node.link(_target)
                    self.links.add((node, _target))
                    break
                if _node == self.root:
                    node.link(self.root)
                    self.links.add((node, 'ROOT'))
                _node = _node.suffix

    def linkDict(self):  # dictionary suffix linker
        for node in self.nodes:
            _target = node.suffix
            while _target != self.root:
                if _target and _target.isEndOfStr:
                    node.link(_target)
                    self.links.add((node, _target))
                    break
                _target = _target.suffix

    def process(self, _str):
        curNode = self.root
        while _str:
            _node = curNode.charInChildren(_str[0])
            if _node:
                curNode = _node
            elif curNode == self.root:
                pass
            elif curNode.suffix:
                curNode = curNode.suffix
            if curNode.isEndOfStr:
                self.record[curNode.corrStr] += 1
            _str = _str[1:]
        return self.record


class ACnode():

    __slots__ = ('char', 'parent', 'isEndOfStr', 'corrStr', 'children',
                 'suffix', 'parallel', 'repr', 'level')

    def __init__(self, char, parent, endOfStr=False, corrStr=None):
        self.char = char
        self.parent = parent
        self.isEndOfStr = endOfStr
        self.corrStr = corrStr
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

    def link(self, other, type=1):
        # 0 for child, 1 for suffix, 2 for dictsuffix
        if type == 1:
            self.suffix = other
        elif type == 0:
            self.children.add(other)
        else:
            self.parallel = other  # bidirectional

    def isOnBranch(self, _str):
        return repr(self).lstrip(_str) != repr(self)

    def charInChildren(self, char):
        for i in self.children:
            if i.char == char:
                return i

    def __repr__(self):
        if self.char != 'ROOT':
            return self.repr
        return ''

    def __eq__(self, other):
        return self.repr == other.repr

    def isSameChar(self, other):
        return self.char == other.char

    def __hash__(self):
        return hash(self.repr)

if __name__ == '__main__':
    # struct = ACtrie({'ab', 'bca'})
    struct = ACtrie({'cot', 'cos', 'cosh'})
    print(struct.links)
