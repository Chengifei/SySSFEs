from error import IncompatibleEnvironment


def envCheck():
    import sys
    if not (sys.version_info.major >= 3 and sys.version_info.minor >= 5):
        raise IncompatibleEnvironment('Phyco requires at least Python 3.5')
    del sys

c = 299792458
G = 6.674 * 10 ** -11
g = 9.80665
epsilon0 = 8.854 * 10 ** -12
