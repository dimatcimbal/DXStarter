#include "UploadBuffer.h"

#include <cstring>

#include "Logging/Logging.h"

BufferRange UploadBuffer::Map(size_t Offset, size_t Size) {
    return BufferRange{Offset, Size, this};
}

BufferRange UploadBuffer::Map() {
    return this->Map(0, mSize);
}

bool UploadBuffer::UploadBytes(size_t Size, const void* data) {
    BufferRange BufferRange = Map(0, Size);
    return BufferRange.UploadBytes(Size, data);
}

bool BufferRange::UploadBytes(size_t size, const void* data) const {
    if (size > mSize) {
        LOG_ERROR("\t\tData size (%zu) exceeds upload buffer size (%zu).\n", size, mSize);
        return false;
    }

    memcpy(mPtr, data, size);
    return true;
}