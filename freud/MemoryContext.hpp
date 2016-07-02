#ifndef FREUD_MEMORY_CONTEXT
#define FREUD_MEMORY_CONTEXT

#include "freud/Defines.hpp"
#include <fstream>
#include <string>
#include <vector>

namespace freud {

template <typename T>
class MemoryContextIterator;

class MemoryContextEndIterator {};

template <typename Context>
class BaseMemoryContext {
public:
    struct MemoryRegion {
        std::string name;
        address_t start_address;
        address_t end_address;
    };

    virtual ~BaseMemoryContext() {}

    template <typename MemObject>
    MemoryContextIterator<MemObject> scan_once() {
        return MemoryContextIterator<MemObject>(
            *reinterpret_cast<Context*>(this));
    }

    template <typename MemObject>
    MemoryContextIterator<MemObject> scan_forever() {
        return MemoryContextIterator<MemObject>(*reinterpret_cast<Context*>(
                                                    this),
                                                true);
    }

    template <typename MemObject>
    MemoryContextIterator<MemObject> end() const {
        return MemoryContextIterator<MemObject>();
    }

    MemoryContextEndIterator end() const { return MemoryContextEndIterator(); }

    const std::vector<MemoryRegion>& mapped_regions() const {
        return m_regions;
    }

    typename std::vector<MemoryRegion>::const_iterator
    region_containing(address_t address) const {
        for (typename std::vector<MemoryRegion>::const_iterator iter =
                 m_regions.begin();
             iter != m_regions.end(); ++iter) {
            if (address >= iter->start_address && address < iter->end_address) {
                return iter;
            }
        }
        return m_regions.end();
    }

protected:
    std::vector<MemoryRegion> m_regions;
};
}

#if defined _WIN32
#include "freud/WindowsMemoryContext.hpp"
#elif defined __gnu_linux__
#include "freud/LinuxMemoryContext.hpp"
#else
#error Unknown platform
#endif

#endif
