#include "Paths.h"

#include <Windows.h>

#include "Logging/Logging.h"

bool Paths::GetMaterialsDirAbsPath(std::filesystem::path& OutPath) {
    static std::filesystem::path compiledShaderDir;
    if (compiledShaderDir.empty()) {
        // Get abs path to the exec
        wchar_t execAbsPath[MAX_PATH];

        DWORD result = GetModuleFileNameW(nullptr, execAbsPath, MAX_PATH);
        if (result == 0) {
            LOG_ERROR(L"Failed to get path to the executing module as %lu\n", GetLastError());
            return false;
        }
        if (result == MAX_PATH) {
            LOG_ERROR(L"Failed to get path to the executing module as MAX_PATH reached.\n");
            return false;
        }

        // Drop the file name
        std::filesystem::path moduleAbsPath(execAbsPath);
        moduleAbsPath.remove_filename();

        // Append Materials dirname
        compiledShaderDir = moduleAbsPath / "Materials";
    }

    OutPath = compiledShaderDir;
    return true;
}
