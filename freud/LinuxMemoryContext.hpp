#ifndef FREUD_LINUX_MEMORY_CONTEXT
#define FREUD_LINUX_MEMORY_CONTEXT

#include "freud/MemoryContext.hpp"

namespace freud {

class LinuxMemoryContext : public BaseMemoryContext<LinuxMemoryContext> {
public:
    LinuxMemoryContext(uint64_t pid, bool heap_only = false)
        : BaseMemoryContext(),
          m_mem((boost::format("/proc/%d/mem") % pid).str().c_str()),
          m_pid(pid),
          m_heap_only(heap_only) {
        update_regions();
    }
    virtual ~LinuxMemoryContext() {}

    bool read(uint64_t address, std::vector<char>& buffer) {
        return read(address, buffer, region_containing(address));
    }

    bool
    read(uint64_t address, std::vector<char>& buffer,
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
            bool res =
                read_without_cache(iter->start_address, m_cached_region.second);
            read_from_cache(address, buffer);

            return res;
        } else if (address >= cached.start_address &&
                   address < cached.end_address) {
            // read from cached bytes

            read_from_cache(address, buffer);
            return true;
        }
        return false;
    }

    void update_regions() {
        m_regions.clear();
        std::string path = (boost::format("/proc/%d/maps") % m_pid).str();
        std::ifstream maps_file(path.c_str());

        std::string region_info;
        boost::regex expr(
            "(\\S+)-(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)?");
        boost::smatch match;
        while (std::getline(maps_file, region_info)) {
            if (boost::regex_search(region_info, match, expr)) {
                uint64_t start_addr =
                    std::strtoull(match[1].str().c_str(), NULL, 16);
                uint64_t end_addr =
                    std::strtoull(match[2].str().c_str(), NULL, 16);

                MemoryRegion region = {match[7], start_addr, end_addr};
                if (m_heap_only && match[7] == "[heap]") {
                    m_regions.push_back(region);
                    break;
                } else if (!m_heap_only) {
                    m_regions.push_back(region);
                }
            }
        }
    }

private:
    std::ifstream m_mem;
    uint64_t m_pid;
    bool m_heap_only;
    std::pair<MemoryRegion, std::vector<char>> m_cached_region;

    void read_from_cache(uint64_t address, std::vector<char>& buffer) {
        uint64_t region_offset = address - m_cached_region.first.start_address;
        std::copy(m_cached_region.second.begin() + region_offset,
                  m_cached_region.second.begin() + region_offset +
                      buffer.size(),
                  buffer.begin());
    }

    bool read_without_cache(uint64_t address, std::vector<char>& buffer) {
        m_mem.seekg(address);

        // TODO: handle errors
        m_mem.read(&*buffer.begin(), buffer.size());
        bool result = m_mem.good();
        if (!result) {
            m_mem.clear();
        }
        return result;
    }
};

typedef LinuxMemoryContext MemoryContext;
}
#endif
