#pragma once

#include <filesystem>

class Paths {
   public:
    static bool GetMaterialsDirAbsPath(std::filesystem::path& OutPath);
};
