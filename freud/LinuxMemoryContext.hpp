#ifndef FREUD_LINUX_MEMORY_CONTEXT
#define FREUD_LINUX_MEMORY_CONTEXT

#include "freud/MemoryContext.hpp"
#include <cstdlib>
#include <sstream>

namespace freud {

class LinuxMemoryContext : public BaseMemoryContext<LinuxMemoryContext> {
public:
    LinuxMemoryContext(unsigned long pid, bool heap_only = false)
        : BaseMemoryContext(), m_mem(), m_pid(pid), m_heap_only(heap_only) {
        std::ostringstream ss;
        ss << "/proc/" << pid << "/mem";
        m_mem.open(ss.str().c_str());
        update_regions();
    }

    ~LinuxMemoryContext() {}

    bool read(address_t address, std::vector<char>& buffer) {
        return read(address, buffer, region_containing(address));
    }

    bool
    read(address_t address, std::vector<char>& buffer,
         std::vector<BaseMemoryContext::MemoryRegion>::const_iterator iter) {
        MemoryRegion& cached = m_cached_region.first;

        // If we don't know of a region containing the address, just
        // try to read it directly
        if (iter == m_regions.end()) {
            return read_without_cache(address, buffer);
        }

        if (m_cached_region.second.size() == 0 ||
            address < cached.start_address || address >= cached.end_address) {
            // cache the region containing address

            m_cached_region.first = *iter;
            m_cached_region.second.resize(iter->end_address -
                                          iter->start_address);

            if (!read_without_cache(iter->start_address,
                                    m_cached_region.second)) {
                // This has the effect of skipping regions that have shrunk
                // since the mapped regions were last updated. This might
                // not work correctly for some applications, but we can't
                // update_regions without invalidating other iterators.
                return false;
            }
        }

        if (address >= cached.start_address && address < cached.end_address) {
            // read from cached bytes

            read_from_cache(address, buffer);
            return true;
        }
        return false;
    }

    void update_regions() {
        m_regions.clear();
        m_cached_region.second.resize(0);
        std::ostringstream ss;
        ss << "/proc/" << m_pid << "/maps";
        std::ifstream maps_file(ss.str().c_str());

        std::string region_info;
        while (std::getline(maps_file, region_info)) {
            std::string addrs = region_info.substr(0, region_info.find(" "));

            std::string start_addr_s = addrs.substr(0, addrs.find("-"));
            std::string end_addr_s = addrs.substr(addrs.find("-") + 1);

            address_t start_addr = std::strtoul(start_addr_s.c_str(), NULL, 16);
            address_t end_addr = std::strtoul(end_addr_s.c_str(), NULL, 16);

            std::string name = region_info.substr(region_info.rfind(" ") + 1);

            MemoryRegion region = {name, start_addr, end_addr};

            if (m_heap_only && name == "[heap]") {
                m_regions.push_back(region);
                break;
            } else if (!m_heap_only) {
                m_regions.push_back(region);
            }
        }
    }

private:
    std::ifstream m_mem;
    unsigned long m_pid;
    bool m_heap_only;
    std::pair<MemoryRegion, std::vector<char> //
              >
        m_cached_region;

    void read_from_cache(address_t address, std::vector<char>& buffer) {
        address_t region_offset = address - m_cached_region.first.start_address;
        std::copy(m_cached_region.second.begin() + region_offset,
                  m_cached_region.second.begin() + region_offset +
                      buffer.size(),
                  buffer.begin());
    }

    bool read_without_cache(address_t address, std::vector<char>& buffer) {
        m_mem.seekg(address);

        m_mem.read(&*buffer.begin(), buffer.size());
        bool result = m_mem.good();
        if (!result) {
            // FIXME: this probably isn't always going to work
            m_mem.clear();
        }
        return result;
    }
};

typedef LinuxMemoryContext MemoryContext;
}
#endif
