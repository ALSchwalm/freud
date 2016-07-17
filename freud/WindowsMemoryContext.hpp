#ifndef FREUD_WINDOWS_MEMORY_CONTEXT
#define FREUD_WINDOWS_MEMORY_CONTEXT

#include "freud/MemoryContext.hpp"
#include <windows.h>

namespace freud {

class WindowsMemoryContext : public BaseMemoryContext<WindowsMemoryContext> {
public:
    WindowsMemoryContext(unsigned long pid) : BaseMemoryContext(), m_pid(pid) {
        update_regions();
    }

    ~WindowsMemoryContext() {}

    bool read(address_t address, std::vector<char>& buffer) {
        return read(address, buffer, region_containing(address));
    }

    bool
    read(address_t address, std::vector<char>& buffer,
         std::vector<BaseMemoryContext::MemoryRegion>::const_iterator iter) {
        SIZE_T bytes_read;
        bool res =
            ReadProcessMemory(m_proc_handle, (LPCVOID)address, &*buffer.begin(),
                              buffer.size(), &bytes_read);
        return res;
    }

    void update_regions() {
        m_regions.clear();
        MEMORY_BASIC_INFORMATION mem_info;
        SYSTEM_INFO system_info;
        m_proc_handle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, m_pid);

        GetSystemInfo(&system_info);

        for (LPVOID address = system_info.lpMinimumApplicationAddress;
             address < system_info.lpMaximumApplicationAddress;
             address = (LPVOID)((DWORD)mem_info.BaseAddress +
                                (DWORD)mem_info.RegionSize)) {

            VirtualQueryEx(m_proc_handle, address, &mem_info, sizeof(mem_info));

            if (mem_info.State & MEM_FREE || mem_info.State & MEM_RESERVE) {
                continue;
            }

            MemoryRegion region = {"", (address_t)mem_info.BaseAddress,
                                   (address_t)mem_info.BaseAddress +
                                       mem_info.RegionSize};
            m_regions.push_back(region);
        }
    }

private:
    unsigned long m_pid;
    HANDLE m_proc_handle;
};

typedef WindowsMemoryContext MemoryContext;
}

#endif
