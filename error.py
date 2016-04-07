class MathError(Exception):
    pass


class UnspecifiedVariable(MathError):
    pass


class UnderConstruction(Exception):
    pass


class CollisionDetected(UserWarning):
    pass


class DuplicationWarning(UserWarning):
    pass


class InvalidKeyword(SyntaxError):
    pass


class UncompatibleEnvironment(OSError):
    pass
