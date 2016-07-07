#ifndef FREUD_MEMORY_OBJECT
#define FREUD_MEMORY_OBJECT

namespace freud {

/** \brief The base class for all MemoryObjects
 *
 * All MemoryObjects used in `freud` should derived from this type. For
 * example, if a user wanted to locate a structure with the following
 * layout:
 *
 * \code{.c}
 * struct Position {
 *   int x;
 *   int y;
 * }
 * \endcode
 *
 * If the user knew that 'x' would always equal '10', they could create
 * a MemoryObject with this constraint as follows:
 *
 * \code{.c}
 * class PositionMatcher : public MemoryObject<Position> {
 *   static bool verify(const Position& p) {
 *     return p.x == 10;
 *   }
 * }
 * \endcode
 */
template <typename T>
class MemoryObject {
public:
    typedef T type;
    static bool verify(const T&) { return true; }
    static void before_check() {}
};
}

#endif
