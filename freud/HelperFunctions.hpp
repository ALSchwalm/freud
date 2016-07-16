#ifndef FREUD_HELPER_FUNCTIONS
#define FREUD_HELPER_FUNCTIONS

#include "freud/MemoryContext.hpp"
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

/** \brief Format the bytes in the given array into a string of hex characters
 *
 * See the documentation for the iterator-based `format_as_hex`.
 */
template <typename T, std::size_t Size>
std::string format_as_hex(const T (&arr)[Size]) {
    return format_as_hex(&arr[0], &arr[0] + Size);
}

/** \brief Determine if a give address is valid within a context
 *
 * If the user is verifying a structure with some pointer fields,
 * it is often sufficient to test if the pointer value exists in
 * a mapped region of memory, as this is unlikely to occur by
 * coincidence. This function provides a convenient way to perform
 * this test.
 */
template <typename T>
bool is_valid_address(const MemoryContext& ctx, const T* address) {
    const std::vector<MemoryRegion>& regions = ctx.mapped_regions();
    typename std::vector<MemoryRegion>::const_iterator iter =
        ctx.region_containing(reinterpret_cast<address_t>(address));
    return iter != regions.end();
}
}

#endif
