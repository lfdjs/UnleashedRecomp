#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace portable::process
{
    /**
     * EN: Launch a detached Linux process and report exec/chdir errors to its
     *     parent. Empty working directories preserve the parent's directory.
     *     Other operating systems retain their existing OS-specific launchers.
     * PT-BR: Inicia um processo Linux separado e comunica falhas de exec/chdir
     *     ao chamador. Diretorio vazio preserva o diretorio atual.
     *     Outros sistemas continuam com os lancadores ja existentes.
     */
    bool StartDetachedLinux(const std::filesystem::path& executable,
        const std::vector<std::string>& arguments,
        const std::filesystem::path& workingDirectory = {});
}
