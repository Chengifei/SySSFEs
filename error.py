class MathError(Exception):
    pass

class UnderConstruction(Exception):
    pass

class CollisionDetected(UserWarning):
    pass

class DuplicationWarning(UserWarning):
    pass

class InvalidKeyword(SyntaxError):
    pass

class UncompatibleEnvironment(Exception):
    pass