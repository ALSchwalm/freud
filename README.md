Freud
=====

`freud` is a C++ header-only library for finding structures in a running
process's memory.

Installation
------------

As a header-only library, no installation is required. `freud` supports
any C++98 or newer compiler. To use it, simply clone this project or
download the zip file and add the 'freud' directory to your C++ project's
include path.

Usage
-----

Using `freud` has three major components:

- Defining a `MemoryObject`
- Creating a `MemoryContext`
- Scanning the context for your objects

These steps are detailed below:

Defining a MemoryObject
-----------------------

A `MemoryObject` represents some series of constraints used to verify
that a particular collection of bytes are an instance of some type.
For example, suppose that you know you are looking for a structure that
has the following layout:

    struct Position {
        int x;
        int y;
    }

This alone would not be enough to locate the structure. We need some
type of constraint or element to use to narrow down the search. In this
case, suppose we know that the 'x' position is equal to 10 and we want
to find the 'y' position. Then we could define a `MemoryObject` like
this:

    class PositionMatcher : public MemoryObject<Position> {
        static bool verify(const Position& p) {
            return p.x == 10;
        }
    }

The core of this definition is the `verify` member function. This function
will be passed an object created from bytes in a `MemoryContext`. The user
must determine whether the provided object has the properties they expect.
In this case, we test whether the 'x' member has the expected value.


Creating a `MemoryContext`
--------------------------

A `MemoryContext` represents a generic interface to some collection of memory.
Typically, this will be a running process's memory. This interface can be used
to read bytes from another process's virtual address space.

In general, a `MemoryContext` can be created from whatever the process ID
equivalent is on a given platform. Suppose that the 'Xorg' process on my target
has a PID of 659. Then I can create a context for that processes with

    MemoryContext ctx(659);

Note that reading from a `MemoryContext` may require elevated permissions on
some platforms. Specifically, the user must be root on Linux. However, Windows
processes may read each other's memory provided that have the same permissions.

Scanning the context for your objects
-------------------------------------

A `MemoryContext` will provide two member functions for scanning the memory
it wraps: `scan_once` and `scan_forever`. These return a `MemoryContextIterator`,
which is a type representing a pointer into some process memory. When
dereferenced, the iterator will return a reference to an instance of the
target structure that passes the 'verify' test of the matcher. For example,
a user could print all of the `Positions` with the following code:

    MemoryContextIterator<PositionMatcher> iter = ctx.scan_once<PositionMatcher>();

    for(; iter != ctx.end(); ++iter) {
        std::cout << "Position: x=" << iter->x << ", y=" << iter->y << "\n";
    }

The `scan_forever` function is similar. However the `MemoryContextIterator`
returned by this function will never equal `ctx.end()`. Instead, it will
wrap around and start scanning memory again when it reaches the end. This is
useful for making a real-time scraping program.

Note however, that a given `MemoryContext` object should only have one
continuous (i.e., returned from `scan_forever`) object at a time. Incrementing
an iterator of this kind will invalidate all other iterators into the context.
Multiple single pass (i.e., returned from `scan_once`) iterators may be used
at the same time. Additionally, multiple contexts may refer to the same process.

Putting it all together
-----------------------

A complete example to print all of the `Position` objects would therefore
look like this:

    #include "freud/freud.hpp"
    #include <iostream> // for streams
    #include <cstdlib>  // for atio

    struct Position {
        int x;
        int y;
    };

    class PositionMatcher : public MemoryObject<Position> {
        static bool verify(const Position& p) {
            return p.x == 10;
        }
    };

    int main(int argc, char** argv) {
        MemoryContext ctx(atoi(argv[1]));

        MemoryContextIterator<PositionMatcher> iter =
            ctx.scan_once<PositionMatcher>();

        for(; iter != ctx.end(); ++iter) {
            std::cout << "Position: x=" << iter->x << ", y=" << iter->y << "\n";
        }
    }


Contributing
------------

This project is currently in a somewhat unstable state, so things may change.
That being said, contributions are welcome.

Library
-------

This project is released under the terms of the MIT license. See the LICENSE
file for details.
