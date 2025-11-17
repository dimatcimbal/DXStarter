#include "Paths.h"

#include <Windows.h>

#include "Logging/Logging.h"

bool Paths::GetMaterialsDir(std::filesystem::path& OutPath) {
    static std::filesystem::path cachedMaterialsDir;
    if (cachedMaterialsDir.empty()) {
        wchar_t execPathStr[MAX_PATH];

        DWORD result = GetModuleFileNameW(nullptr, execPathStr, MAX_PATH);
        if (result == 0) {
            LOG_ERROR(L"Failed to get path to the executing module as %lu\n", GetLastError());
            return false;
        }
        if (result == MAX_PATH) {
            LOG_ERROR(L"Failed to get path to the executing module as MAX_PATH reached.\n");
            return false;
        }

        std::filesystem::path execAbsPath(execPathStr);
        std::filesystem::path parentAbsPath(execAbsPath
                                                .parent_path()  // Current example dir
                                                .parent_path()  // Parent of the current example dir
        );

        // Materials home dir is at the same level as the current example dir
        cachedMaterialsDir = parentAbsPath / "Materials";
    }

    OutPath = cachedMaterialsDir;
    return true;
}
