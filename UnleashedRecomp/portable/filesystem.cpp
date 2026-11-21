#include "filesystem.h"
#include <cstdlib>
#include <string>
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <ShlObj.h>
#elif defined(__linux__) && !defined(__ANDROID__)
#include <pwd.h>
#include <unistd.h>
#endif

namespace portable::filesystem {
std::filesystem::path UserDataRoot(std::string_view application) {
    const std::string name(application);
#if defined(__SWITCH__) || defined(UNLEASHED_RECOMP_SWITCH)
    // EN: Keep the existing SD card directory layout.
    // PT-BR: Preserva a estrutura atual de diretorios no SD.
    return std::filesystem::path("sdmc:/switch") / name;
#elif defined(_WIN32)
    PWSTR folder = nullptr;
    auto result = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &folder);
    std::filesystem::path path;
    if (SUCCEEDED(result) && folder) path = std::filesystem::path(folder) / name;
    if (folder) CoTaskMemFree(folder);
    return path;
#else
#if defined(__ANDROID__)
    // EN: An Android host must supply its app sandbox path explicitly.
    // PT-BR: No Android o aplicativo deve fornecer o caminho sandbox explicitamente.
    return {};
#else
    const char* home = std::getenv("HOME");
#if defined(__linux__)
    if (!home || !*home) {
        const passwd* entry = getpwuid(getuid());
        home = entry ? entry->pw_dir : nullptr;
    }
#endif
    if (!home || !*home) return {};
    const std::filesystem::path root(home);
#if defined(__APPLE__)
    const auto config = root / "Library" / "Application Support";
#else
    const auto config = root / ".config";
#endif
    return std::filesystem::exists(config) ? config / name : root / ("." + name);
#endif
#endif
}
bool EnsureDirectory(const std::filesystem::path& path, std::error_code& error) noexcept {
    error.clear();
    if (path.empty()) return false;
    if (std::filesystem::create_directories(path, error)) return true;
    return !error && std::filesystem::is_directory(path, error);
}
} // namespace portable::filesystem
