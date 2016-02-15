"""Aho-Corasick Automaton for string pattern-match

This is used both in phycomath.genmath.expr and
integrated interpreter for aliasing.

"""
# NEVER ACCESS CHILDREN IN ATTRIBUTE


class ACtrie():

    def __init__(self, patternset):
        self.set = patternset
        self.maxlen = len(max(patternset, key=len)) if patternset else 0
        self.minlen = len(min(patternset, key=len)) if patternset else 0
        self.root = _ACnode('ROOT', None)
        # should probably move to be a class variable
        self.counter = {}
        self.record = {}
        # self.counter = Counter()
        self.links = set()
        self.nodesByLevel = [{} for i in range(self.maxlen + 1)]
        self.nodesByLevel[0][''] = self.root
        for i in self.set:
            self.add(i)
        self.nodes = [i[j] for i in self.nodesByLevel for j in i]
        self.nodes = self.nodes[1:]
        self.linkSuffix()  # link all suffix in AC-trie
        self.linkDict()  # link all parallel match

    def newNode(self, _repr, corrStr=None):
        _len = len(_repr)
        if _repr[:-1] in self.nodesByLevel[_len - 1]:
            parent = self.nodesByLevel[_len - 1][_repr[:-1]]
            if _repr not in self.nodesByLevel[_len]:
                self.nodesByLevel[_len][_repr] = _ACnode(_repr[-1], parent, corrStr)
                if _len == 1:
                    self.nodesByLevel[_len][_repr].link(self.root)
                    self.links.add((_repr, 'ROOT'))

    def getNode(self, _repr):
        try:
            return self.nodesByLevel[len(_repr)][_repr]
        except KeyError:
            return
        except IndexError:
            return

    def init(self):
        for _str in self.set:
            self.counter[_str] = 0
            self.record[_str] = set()

    def add(self, _str):  # WARNING:danger of without setting self.max/minlen
        if len(_str) == 1:
            _node = _ACnode(_str[0], self.root, corrStr=_str)
            _existed = self.nodesByLevel[1]
            if _node.repr not in _existed:
                self.nodesByLevel[1][_node.repr] = _node
                _node.link(self.root)
                self.links.add((_node, 'ROOT'))
        else:
            for i in range(len(_str)):
                self.newNode(_str[:i])
            self.newNode(_str, _str)
        # Heretofore, all parent-child, ROOT-first suffix relations linked

    def linkSuffix(self):  # suffix linker
        for node in self.nodes:
            _node = node.parent.suffix
            while not node.suffix:
                _target = self.getNode(repr(_node) + node.char)
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
                if _target and _target.isEndOfStr and node.suffix != _target:
                    node.link(_target, 2)
                    self.links.add((node, _target))
                    break
                _target = _target.suffix

    def process(self, _str):
        self.init()
        _str = _str + '$'
        curNode = self.root
        pos = 0
        POS = 0
        while _str[pos:]:

            _node = self.getNode(_str[POS:pos + 1])

            if _node:
                curNode = _node
                pos += 1
            elif curNode.suffix:  # curNode is not self.root
                POS = pos - 1
                curNode = curNode.suffix
            elif curNode is self.root:
                pos += 1
                POS = pos - 1
                _node = self.getNode(_str[POS])
                if _node:
                    curNode = _node

            # check for end of pattern
            if curNode.isEndOfStr:
                self.counter[curNode.corrStr] += 1
                self.record[curNode.corrStr].add(pos - len(curNode.corrStr))
                self.parallelMatch(curNode, pos)

        return self.counter

    def parallelMatch(self, node, pos):
        curNode = node
        visited = set()
        if curNode.parallel and curNode not in visited:
            curNode = node.parallel
            visited.add(curNode)
            self.counter[curNode.corrStr] += 1
            self.record[curNode.corrStr].add(pos - len(curNode.corrStr))
            self.parallelMatch(curNode, pos)


class _ACnode():

    __slots__ = ('char', 'parent', 'isEndOfStr', 'corrStr', 'children',
                 'suffix', 'parallel', 'repr', 'level')

    def __init__(self, char, parent, corrStr=None):
        self.char = char
        self.parent = parent
        self.isEndOfStr = bool(corrStr)
        self.corrStr = corrStr
        self.children = set()
        self.suffix = None
        self.parallel = None  # one node can have only one but can be linked
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
