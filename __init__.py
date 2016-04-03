from error import UncompatibleEnvironment
import sys


def envCheck():
    if not (sys.version_info.major == 3 and sys.version_info.minor >= 5):
        raise UncompatibleEnvironment

c = 299792458
G = 6.674 * 10 ** -11
g = 9.80665
epsilon0 = 8.854 * 10 ** -12
