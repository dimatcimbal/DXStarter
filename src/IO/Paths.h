#pragma once

#include <filesystem>

class Paths {
   public:
    static bool GetCompiledShaderDirAbsPath(std::filesystem::path& OutPath);
};
