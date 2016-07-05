#ifndef FREUD_MEMORY_CONTEXT
#define FREUD_MEMORY_CONTEXT

#include "freud/Defines.hpp"
#include <fstream>
#include <string>
#include <vector>

namespace freud {

template <typename T>
class MemoryContextIterator;

/**
 * A convenience type. These iterators will compare equal to
 * any other MemoryContextIterator that is at the end of its
 * context.
 */
class MemoryContextEndIterator {};

/// The basic interface for MemoryContext's across all platforms
template <typename Context>
class BaseMemoryContext {
public:
    /// Represents a valid region of addresses in a context
    struct MemoryRegion {
        std::string name;
        address_t start_address;
        address_t end_address;
    };

    ~BaseMemoryContext() {}

    /// Create a non-continuous (single-pass) iterator into this context
    /**
     * 'MemObject' must be a type derived from MemoryObject. Multiple
     * single-pass iterators may traverse a memory context simultaneously.
     */
    template <typename MemObject>
    MemoryContextIterator<MemObject> scan_once() {
        return MemoryContextIterator<MemObject>(
            *reinterpret_cast<Context*>(this));
    }

    /// Create a continuous iterator into this context
    /**
     * 'MemObject' must be a type derived from MemoryObject. Note that
     * incrementing a continuous MemoryContextIterator will invalidate
     * all other iterators into this context.
     */
    template <typename MemObject>
    MemoryContextIterator<MemObject> scan_forever() {
        return MemoryContextIterator<MemObject>(*reinterpret_cast<Context*>(
                                                    this),
                                                true);
    }

    /// Retrieve an iterator to the end of the context
    template <typename MemObject>
    MemoryContextIterator<MemObject> end() const {
        return MemoryContextIterator<MemObject>();
    }

    /// Retrieve an iterator to the end of the context. This iterator may be
    /// compared to a MemoryContextIterator of any MemObject.
    MemoryContextEndIterator end() const { return MemoryContextEndIterator(); }

    /// A collection of MemoryRegions representing the known valid addresses in
    /// this context
    const std::vector<MemoryRegion>& mapped_regions() const {
        return m_regions;
    }

    /// Retrieve an iterator to the MemoryRegion containing the given address
    /**
     * \param address The region containing this address will be returned
     * \returns An iterator to the region contained 'address' or 'end' if
     *          no region contains the provided address.
     */
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
