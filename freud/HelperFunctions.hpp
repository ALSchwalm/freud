#ifndef FREUD_HELPER_FUNCTIONS
#define FREUD_HELPER_FUNCTIONS

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace freud {

/**
 * Format the bytes in the given range into a string of hex characters
 */
template <typename Iter>
std::string format_as_hex(Iter begin, Iter end) {
    std::stringstream ss;
    char buff[3] = "\0\0";
    for (; begin != end; ++begin) {
        for (unsigned int i = 0; i < sizeof(*begin); ++i) {
            const unsigned char* ptr =
                reinterpret_cast<const unsigned char*>(&*begin) + i;
            sprintf(buff, "%02x", *ptr);
            ss << buff;
        }
    }
    return ss.str();
}

template <typename T, std::size_t Size>
std::string format_as_hex(const T (&arr)[Size]) {
    return format_as_hex(&arr[0], &arr[0] + Size);
}
}

#endif
