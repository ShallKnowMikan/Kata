#pragma once
#include "sugar.h"

class ArenaAllocator {
    int size {0};
    Byte* buffer;
    Byte* offset;

public:
    explicit ArenaAllocator(const int bytes) : size(bytes) {
        buffer = static_cast<Byte *>(malloc(size));
        offset = buffer;
    }
    explicit ArenaAllocator(const size_t& bytes) = delete;

    ArenaAllocator operator=(const ArenaAllocator& other) = delete;

    ~ArenaAllocator() {
        free(buffer);
    }

    template<typename T>
    T* allocate() {
        void* currentOffset = offset;
        offset += sizeof(T);
        return static_cast<T*>(currentOffset);
    }

};
