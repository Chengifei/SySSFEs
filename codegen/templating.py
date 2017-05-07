def template(filename, delim = '[[>>>]]'):
    with open(filename) as f:
        for i in f:
            i = i.lstrip()
            if i.startswith(delim):
                py_writer = yield # expect an iterable
                yield from py_writer
            else:
                yield i
