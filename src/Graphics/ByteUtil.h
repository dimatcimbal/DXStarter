#pragma once

/**
 * Utility class for byte-related operations.
 */
class ByteUtil {
   public:
    /** Aligns the given size to 256 bytes, which is required for constant buffers in D3D12.
     * @param Size The size to align.
     * @return The aligned size, rounded up to the nearest multiple of 256 bytes.
     */
    static size_t AlignTo256Bytes(size_t Size) {
        return (Size + 255) & ~255;
    }
};
