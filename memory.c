#include "memory.h"

#if defined(_WIN32) || defined(_WIN64)

#include <sysinfoapi.h>
struct memory get_memory()
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    struct memory mem;
    mem.total = status.ullTotalPhys;
    mem.free = status.ullAvailPhys;
    return mem;
}
#elif defined(__linux__)

#include <sys/sysinfo.h>
struct memory get_memory()
{
    struct sysinfo info;
    sysinfo(&info);
    struct memory mem;
    mem.total = info.totalram;
    mem.free = info.freeram;
    return mem;
}

#endif
