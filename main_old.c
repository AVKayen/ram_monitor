#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "memory.h"

struct timespec UPDATE_INTERVAL = {
    .tv_sec = 1,
    .tv_nsec = 0,
};

// Converts a memory size in bytes to a human-readable string eg. "1.23 GiB"
void prettify_memory(double bytes, char *result, int result_size)
{
    char *units[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB"};
    int i = 0;
    while (bytes >= 1024)
    {
        bytes /= 1024;
        i++;
    }
    snprintf(result, result_size, "%.3f %s", bytes, units[i]);
}


int main()
{
    struct memory mem = get_memory();
    char buffer[16];

    prettify_memory((double)mem.total, buffer, sizeof(buffer)); // Corrected function name spelling
    printf("Total memory: %s\n", buffer);
    while (1)
    {
        mem = get_memory();
        prettify_memory((double)mem.free, buffer, sizeof(buffer)); // Corrected function name spelling
        printf("Free memory: %s\n", buffer);
        nanosleep(&UPDATE_INTERVAL, NULL);
    }

    return 0;
}