def warn_ask(msg, *args):
    print(msg, *args, end=' ')
    while True:
        r = input('(Y/N)').lstrip()
        if r == 'Y':
            return True
        elif r == 'N':
            return False
