class MathError(Exception):
    """Base class for any error in mathematics"""
    pass


class UnspecifiedVariable(MathError):
    """Raised if any symbolic variable in use haven't been
    assigned before the call."""
    pass


class UnderConstruction(Exception):
    pass


class CollisionDetected(UserWarning):
    pass


class InvalidKeyword(SyntaxError):
    """Raised when user input a undefined keyword"""
    pass


class IncompatibleEnvironment(OSError):
    """Phyco requires Python 3.5 or newer version that are
    backward compatible with. Raised when user has a lower
    version or other due to the system environment."""
    pass
