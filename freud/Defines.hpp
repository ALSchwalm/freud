#ifndef FREUD_DEFINES
#define FREUD_DEFINES

namespace freud {
/** \brief The address type used throughout freud
 *
 * Note that this type must be large enough to store the
 * largest possible virtual address of the target (so,
 * a 64 bit value for a 64 bit target).
 */
typedef unsigned long address_t;
typedef char byte_t;
}
#endif
