import struct

class Field(object):
    def __init__(self, name):
        self.name = name

    def verify(self, value):
        """ Test applied to converted bytes """
        return True

    def format(self, value):
        """ Formatting applied to converted bytes during output """
        return value

class LongLong(Field):
    fmt = "q"

class UnsignedLongLong(Field):
    fmt = "Q"

class Long(Field):
    fmt = "l"

class UnsignedLong(Field):
    fmt = "L"

class Int(Field):
    fmt = "i"

class UnsignedInt(Field):
    fmt = "I"

class Short(Field):
    fmt = "h"

class UnsignedShort(Field):
    fmt = "H"

class Byte(Field):
    fmt = "B"


def Array(elemType, name, count):
    class _Array(Field):
        fmt = str(count * struct.calcsize(elemType.fmt)) + "s"
        array_struct = struct.Struct(str(count) + elemType.fmt)

        def __init__(self):
            Field.__init__(self, name)

        def from_bytes(self, s):
            if elemType != Byte:
                return self.__class__.array_struct.unpack(s)
            else:
                return s

        def format(self, value):
            return repr(value)
    return _Array()


class EnumField(Field):
    def __init__(self, name, options):
        self.options = options
        Field.__init__(self, name)

    def verify(self, value):
        if value in self.options:
            return True
        return False

    def format(self, value):
        return self.options[value]


def Enum(basetype, name, options):
    return type("Enum" + basetype.__name__,
                (EnumField, basetype), {})(name, options)


class BoundedField(Field):
    def __init__(self, name, min, max):
        self.min = min
        self.max = max
        Field.__init__(self, name)

    def verify(self, value):
        if value < self.min or value > self.max:
            return False
        return True


def Bounded(basetype, name, min, max):
    return type("Bounded" + basetype.__name__,
                (BoundedField, basetype), {})(name, min, max)


class BitMaskField(Field):
    def __init__(self, name, options):
        self.options = options
        Field.__init__(self, name)

    def verify(self, value):
        from functools import reduce

        opts = [o for o in self.options.keys()
                if ~value & o == 0]
        if opts and value & ~reduce(lambda t, v: t | v, opts) == 0:
            return True
        return False


def BitMask(basetype, name, options):
    return type("BitMask" + basetype.__name__,
                (BitMaskField, basetype), {})(name, options)


class ValueField(Field):
    def __init__(self, name, value):
        self.value = value
        Field.__init__(self, name)

    def verify(self, value):
        return self.value == value

def Value(basetype, name, value):
    return type("Value" + basetype.__name__,
                (ValueField, basetype), {})(name, value)
