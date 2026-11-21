#include <os/process.h>

#include <portable/process.h>

/**
 * EN: Resolve /proc/self/exe with space reserved for the NUL terminator,
 *     avoiding a non-terminated buffer at PATH_MAX bytes.
 * PT-BR: Resolve /proc/self/exe reservando espaco para o terminador NUL,
 *     evitando strings sem terminacao quando o limite PATH_MAX e atingido.
 */
std::filesystem::path os::process::GetExecutablePath()
{
    char exePath[PATH_MAX] = {};
    const ssize_t length = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (length <= 0)
        return {};

    exePath[length] = '\0';
    return std::filesystem::path(exePath);
}

std::filesystem::path os::process::GetExecutableRoot()
{
    return GetExecutablePath().remove_filename();
}

std::filesystem::path os::process::GetWorkingDirectory()
{
    char cwd[PATH_MAX] = {};
    char *res = getcwd(cwd, sizeof(cwd));
    if (res != nullptr)
    {
        return std::filesystem::path(std::u8string_view((const char8_t*)(cwd)));
    }
    else
    {
        return std::filesystem::path();
    }
}

bool os::process::SetWorkingDirectory(const std::filesystem::path& path)
{
    return chdir(path.c_str()) == 0;
}

/**
 * EN: Launch an external Linux program through the shared POSIX implementation.
 *     Unlike the old fork/exec path, this detects exec and chdir failures.
 * PT-BR: Inicia um programa Linux pelo backend POSIX compartilhado.
 *     Diferentemente do fork/exec anterior, detecta falhas de exec e chdir.
 */
bool os::process::StartProcess(const std::filesystem::path& path,
    const std::vector<std::string>& args, std::filesystem::path work)
{
    return portable::process::StartDetachedLinux(path, args, work);
}

void os::process::CheckConsole()
{
    // Always visible on Linux.
    g_consoleVisible = true;
}

void os::process::ShowConsole()
{
    // Unnecessary on Linux.
}
