#ifndef MEMORY_H
#define MEMORY_H

// Structure to store memory information in bytes, both total and free
typedef struct MemoryInfo
{
    unsigned long long total;
    unsigned long long free;
} MemoryInfo;

MemoryInfo get_memory();

#endif