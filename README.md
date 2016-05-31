# freud


`freud` is a python library for locating structures in process memory.

# Installation


Install the latest version from pypi with `pip install freud`, or get the
development version by cloning this repository and running `python setup.py install`.
Note that `pypy` is dramatically faster than cpython when running `freud`, so
if possible, consider installing it.

# Usage


Note that `freud` borrows some interface decisions from the excellent `scapy`
project, so some of the following may look a bit familiar to people familiar
with that library.

## MemoryObjects


`MemoryObject`s are the basis for most operations in `freud`. These represent
some structure in memory, having fields with specific characteristics. For example:

    class ExampleObject(MemoryObject):
        fields_desc = [
            UnsignedInt("version"),
            Array(Byte, "contents", 6)
        ]

This class definition describes a structure in memory having an `unsigned int`
version field, and another that has type `char[6]`. However, these two fields
do not provide any constraints. What if we knew that the version field would
always have value `1` in the target program? We can write this like:

    class ExampleObject(MemoryObject):
        fields_desc = [
            Value(UnsignedInt, "version", 1),
            Array(Byte, "contents", 6)
        ]

So the first field is really a `Value` of type `UnsignedInt`. This will constrain
matches to only those bytes in target memory that can be used to create this
structure where the portion of the bytes used for the version have the value 1
as an `unsigned int`. So:

    01 02 03 04 01 02 03 04 05 06   Does not match (version would be 0x01020304=16909060)
    00 00 00 01 01 02 03 04 05 06   Matches (version is 0x00000001=1)

Note that the values for `contents` do not matter in the above example, because
`Array` does not provide any constraints.

## MemoryContexts


A `MemoryContext` is a type representing a collection of bytes mapping to a processes'
memory. These contexts can be directly created from a process ID (currently linux only)
or by registering some bytes with the context.

    ctx = MemoryContext.from_linux_pid(1234)   # loading from a PID

    # Or registering manually
    ctx = MemoryContext()
    with open("myfile", "rb") as f:
      contents = f.read()
      ctx.register_region(contents, 0x1000, 0x1000+len(contents))

You can then locate occurrences of some `MemoryObject` within the context with
`find_all`.

    for addr, obj in ctx.find_all(ExampleObject):
        print("Found ExampleObject at {}: contents=".format(hex(addr), obj.contents))

## Available Fields


The following fields are built-in:

- The primitives: `LongLong`, `Long`, `Int`, and `Short` (and `Unsigned` versions of each) as well as `Byte`.
- `Value(name, value)`: A field representing a specific value
- `Array(type, name, count)`: Represents and array of `type` of length `count`
- `Enum(type, name, options)`: Represents values that must be one of a list of values.


     # Enum Example
    class ExampleObject(MemoryObject):
        fields_desc = [
            Enum(Byte, "version", {0x01: "Version 1", 0x02: "Version 2"})
        ]

- `Bounded(type, name, min, max)`: A field representing a value with an upper/lower bound
- `BitMask(type, name, masks)`: A field representing a value that is a bitmask composed of the given masks


    # BitMask Example
    class ExampleObject(MemoryObject):
        fields_desc = [
            BitMask(Byte, "flags", { 0x001: "active", 0x100: "ready", 0x010: "waiting"})
        ]

    # This object will match when flags has values 0x001, 0x010, 0x011, 0x101,
    # 0x110, 0x111, or 0x100. That is, when the value is any bitwise OR combination
    # of the provided masks.

- `Ptr(type, name, region_name=None)`: A pointer in to memory registered with the memory context being searched


## Custom Fields


A `Field` is really any type that has the following characteristics:

- A `fmt` attribute that specifies the layout of the field (see
[the python struct docs](https://docs.python.org/2/library/struct.html))
- A `verify(value, ctx)` method. This should return `True` if the provided value represents a valid value for the field
- A `format(value)` method. This should return a string used to print the `Field`
- Optionally, a `from_bytes(bytes)` method. This is useful if there is no `struct`
format character that is appropriate. You can instead use 's' and write this method to convert the bytes to a meaningful
value (which will then be passed to `verify`)
