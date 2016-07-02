/*
  Based on files from the boost library. Those files
  are distributed with the following copyright notice:

  (c) 2014-2015 Glen Joseph Fernandes
  <glenjofe -at- gmail.com>

  Distributed under the Boost Software
  License, Version 1.0.
  http://boost.org/LICENSE_1_0.txt
*/

#ifndef FREUD_ALIGNMENT
#define FREUD_ALIGNMENT

#include <cstddef>

namespace freud {
namespace detail {

template <class T, T Value>
struct integral_constant {
    typedef T value_type;
    typedef integral_constant type;

    operator value_type() const { return Value; }

    static const T value = Value;
};

template <std::size_t A, std::size_t B>
struct min_size : integral_constant<std::size_t, (A < B) ? A : B> {};

template <class T>
struct alignof_helper {
    char value;
    T object;
};

template <class T>
struct alignment_of
    : min_size<sizeof(T), sizeof(alignof_helper<T>) - sizeof(T)>::type {};
}
}

#endif
