#include "Bytes.h"

#include <fstream>

#include "Logging/Logging.h"

bool Bytes::Load(std::filesystem::path FilePath, std::unique_ptr<Bytes>& OutFile) {
    std::ifstream file(FilePath, std::ios::in | std::ios::binary);

    if (!file.is_open()) {
        LOG_ERROR(L"Failed to open file");
        return false;
    }

    // Get the file size
    file.seekg(0, std::ios::end);

    std::streampos fileSizePos = file.tellg();
    if (fileSizePos < 0) {
        LOG_ERROR(L"Failed to get file size");
        return false;
    }

    size_t fileSize = static_cast<size_t>(fileSizePos);

    // Allocate buffer as std::byte[]
    auto buffer = std::make_unique<std::byte[]>(fileSize);

    // Read file contents
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(buffer.get()), fileSize);

    // Check if read was successful
    if (file.fail() && !file.eof()) {
        LOG_ERROR(L"Failed to read file contents");
        return false;
    }

    // Get actual bytes read (in case of eof before expected size)
    size_t bytesRead = static_cast<size_t>(file.gcount());
    if (bytesRead != fileSize) {
        LOG_ERROR(L"File read incomplete: expected %zu bytes, read %zu bytes", fileSize, bytesRead);
        return false;
    }

    OutFile = std::unique_ptr<Bytes>(new Bytes(fileSize, std::move(buffer)));
    return true;
}