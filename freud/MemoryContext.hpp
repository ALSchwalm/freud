#ifndef FREUD_MEMORY_CONTEXT
#define FREUD_MEMORY_CONTEXT

#include <fstream>
#include <list>
#include <string>
#include <vector>

#include <boost/format.hpp>
#include <boost/regex.hpp>

namespace freud {

template <typename T>
class MemoryContextIterator;

class MemoryContext {
public:
    template <typename MemObject>
    MemoryContextIterator<MemObject> iter_matches() {
        return MemoryContextIterator<MemObject>(*this);
    }

    template <typename MemObject>
    MemoryContextIterator<MemObject> end() const {
        return MemoryContextIterator<MemObject>();
    }

    virtual bool read(uint64_t address, std::vector<char>& buffer) = 0;

protected:
    struct MemoryRegion {
        std::string name;
        uint64_t start_address;
        uint64_t end_address;
    };

    std::list<MemoryRegion> m_regions;

    template <typename T>
    friend class MemoryContextIterator;
};

class LinuxMemoryContext : public MemoryContext {
public:
    LinuxMemoryContext(unsigned int pid, bool heap_only = false)
        : MemoryContext(),
          m_mem((boost::format("/proc/%d/mem") % pid).str().c_str()) {
        std::string path = (boost::format("/proc/%d/maps") % pid).str();
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
                if (heap_only && match[7] == "[heap]") {
                    m_regions.push_back(region);
                    break;
                } else {
                    m_regions.push_back(region);
                }
            }
        }
    }

    virtual bool read(uint64_t address, std::vector<char>& buffer) {
        m_mem.seekg(address);

        // TODO: handle errors
        m_mem.read(&*buffer.begin(), buffer.size());
        bool result = m_mem.good();
        if (!result) {
            m_mem.clear();
        }
        return result;
    }

private:
    std::ifstream m_mem;
};
}

#endif
