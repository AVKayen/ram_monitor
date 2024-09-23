#ifndef MEMORY_H
#define MEMORY_H

// Structure to store memory information in bytes
struct memory
{
    unsigned long long total;
    unsigned long long free;
};

struct memory get_memory();

#endif