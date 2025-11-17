#pragma once

#include <filesystem>

class Paths {
   public:
    static bool GetMaterialsDir(std::filesystem::path& OutPath);
};
