//
// Created by dtcimbal on 23/06/2025.
//
#pragma once

#include <Windows.h>  // required for OutputDebugString

#include <cstdio>  // required for swprintf_s

#ifdef _DEBUG
// Buffer size for logging macros
#define LOGGING_BUFFER_SIZE 512

#define LOG_PRINT(file, line, msg, ...)                                                 \
    do {                                                                                \
        wchar_t buffer[LOGGING_BUFFER_SIZE];                                            \
        wchar_t fileLine[256];                                                          \
        swprintf_s(fileLine, _countof(fileLine), L"%hs:%d", file, line);                \
        swprintf_s(buffer, _countof(buffer), L"%-70s - " msg, fileLine, ##__VA_ARGS__); \
        OutputDebugString(buffer);                                                      \
    } while (0)
#else
#define LOG_PRINT(file, line, msg, ...) ((void)0)
#endif

// Helper macro for success messages
#define LOG_SUCCESS(msg, ...) LOG_PRINT(__FILE__, __LINE__, L"[✅] " msg, ##__VA_ARGS__)

// Helper macro for error messages
#define LOG_ERROR(msg, ...) LOG_PRINT(__FILE__, __LINE__, L"[❌] " msg, ##__VA_ARGS__)

// Helper macro for warning messages
#define LOG_WARN(msg, ...) LOG_PRINT(__FILE__, __LINE__, L"[⚠️] " msg, ##__VA_ARGS__)

// Helper macro for info messages
#define LOG_INFO(msg, ...) LOG_PRINT(__FILE__, __LINE__, L"[ℹ️] " msg, ##__VA_ARGS__)