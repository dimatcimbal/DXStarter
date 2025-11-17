#include "ByteBuffer.h"

#include <fstream>

#include "Logging/Logging.h"

bool ByteBuffer::Create(std::filesystem::path FilePath, std::unique_ptr<ByteBuffer>& OutBuffer) {
    std::ifstream File(FilePath, std::ios::in | std::ios::binary);

    if (!File.is_open()) {
        LOG_ERROR(L"Failed to open file");
        return false;
    }

    // Get the file size
    File.seekg(0, std::ios::end);

    std::streampos fileSizePos = File.tellg();
    if (fileSizePos < 0) {
        LOG_ERROR(L"Failed to get file size");
        return false;
    }

    size_t fileSize = static_cast<size_t>(fileSizePos);

    // Allocate buffer as std::byte[]
    auto buffer = std::make_unique<std::byte[]>(fileSize);

    // Read file contents
    File.seekg(0, std::ios::beg);
    File.read(reinterpret_cast<char*>(buffer.get()), fileSize);

    // Check if read was successful
    if (File.fail() && !File.eof()) {
        LOG_ERROR(L"Failed to read file contents");
        return false;
    }

    // Get actual bytes read (in case of eof before expected size)
    size_t bytesRead = static_cast<size_t>(File.gcount());
    if (bytesRead != fileSize) {
        LOG_ERROR(L"File read incomplete: expected %zu bytes, read %zu bytes", fileSize, bytesRead);
        return false;
    }

    OutBuffer = std::make_unique<ByteBuffer>(fileSize, std::move(buffer));
    return true;
}