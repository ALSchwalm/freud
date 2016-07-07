#ifndef FREUD_HELPER_FUNCTIONS
#define FREUD_HELPER_FUNCTIONS

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace freud {

/** \brief Format the bytes in the given range into a string of hex characters
 *
 * For example, if a vector of char with values 0x12, 0x34, 0xAB was passed,
 * the returned value would be: "1234AB". Note that no spaces are inserted,
 * and a '0x' is not used to prefix the string.
 *
 * If the iterator's value type (that is, the type of '*begin') is multi-byte,
 * each byte of the multi-byte value is printed.
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

/// Format the bytes in the given array into a string of hex characters
template <typename T, std::size_t Size>
std::string format_as_hex(const T (&arr)[Size]) {
    return format_as_hex(&arr[0], &arr[0] + Size);
}
}

#endif
