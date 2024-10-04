#include "memory.h"

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)

#include <sysinfoapi.h>
MemoryInfo get_memory()
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    MemoryInfo mem;
    mem.total = status.ullTotalPhys;
    mem.free = status.ullAvailPhys;
    return mem;
}
#elif defined(__linux__)

#include <sys/sysinfo.h>
MemoryInfo get_memory()
{
    struct sysinfo info;
    sysinfo(&info);
    MemoryInfo mem;
    mem.total = info.totalram;
    mem.free = info.freeram;
    return mem;
}

#endif
