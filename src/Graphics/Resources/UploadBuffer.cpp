#include "UploadBuffer.h"

#include <cstring>

#include "Logging/Logging.h"

BufferRange UploadBuffer::Map(size_t Offset, size_t Size) {
    return BufferRange{Offset, Size, this};
}

bool UploadBuffer::UploadBytes(const void* data, size_t size) {
    if (size > mSize) {
        LOG_ERROR("\t\tData size (%zu) exceeds upload buffer size (%zu).\n", size, mSize);
        return false;
    }

    BufferRange range{this->Map(0, size)};
    memcpy(range.GetPtr(), data, size);
    // Unmaps automatically on exiting the scope

    return true;
}