#ifndef FREUD_HELPER_FUNCTIONS
#define FREUD_HELPER_FUNCTIONS

#include <boost/format.hpp>
#include <sstream>
#include <string>

namespace freud {

/**
 * Format the bytes in the given range into a string of hex characters
 */
template <typename Iter>
std::string format_as_hex(Iter begin, Iter end) {
    std::ostringstream ss;
    for (; begin != end; ++begin) {
        for (unsigned int i = 0; i < sizeof(*begin); ++i) {
            const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&*begin) + i;
            ss << boost::format("%02x") % static_cast<unsigned>(*ptr);
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
