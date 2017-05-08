def template(filename, delim = '[[>>>]]'):
    with open(filename) as f:
        for i in f:
            idx = i.find(delim)
            if idx != -1:
                py_writer = yield # expect an iterable
                for text in py_writer:
                    yield from i[:idx] + text + i[idx + len(delim):]
            else:
                yield i
