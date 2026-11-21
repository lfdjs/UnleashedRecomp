#include "paths.h"
#include <os/process.h>
#include <portable/filesystem.h>

std::filesystem::path g_executableRoot = os::process::GetExecutableRoot();
std::filesystem::path g_userPath = BuildUserPath();

bool CheckPortable()
{
    return std::filesystem::exists(g_executableRoot / "portable.txt");
}

std::filesystem::path BuildUserPath()
{
    if (CheckPortable())
        return g_executableRoot;

    // EN: Resolve paths centrally while retaining portable.txt support.
    // PT-BR: Centraliza os caminhos sem alterar o modo portable.txt.
    return portable::filesystem::UserDataRoot(USER_DIRECTORY);
}

const std::filesystem::path& GetUserPath()
{
    return g_userPath;
}
