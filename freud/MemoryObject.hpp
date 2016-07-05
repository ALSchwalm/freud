#ifndef FREUD_MEMORY_OBJECT
#define FREUD_MEMORY_OBJECT

namespace freud {

template <typename T>
class MemoryObject {
public:
    typedef T type;
    static bool verify(const T&) { return true; }
    static void before_check() {}
};
}

#endif
