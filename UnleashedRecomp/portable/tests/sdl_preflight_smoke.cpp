#include "../diagnostics.h"

#include <filesystem>

/**
 * EN: Exercise the real SDL preflight in a headless CI worker. The dummy
 *     video backend may not load Vulkan, so assert stable memory/path bits
 *     instead of falsely requiring real graphics hardware in GitHub Actions.
 * PT-BR: Executa o diagnostico SDL real em um runner sem monitor. O backend
 *     dummy pode nao carregar Vulkan; valida os bits de memoria e caminhos
 *     sem exigir GPU fisica no GitHub Actions.
 */
int main()
{
    const int valid = portable::diagnostics::RunHostPreflight(true,
        std::filesystem::path{"."});
    const int invalid = portable::diagnostics::RunHostPreflight(false, {});

    // EN: Bit 1 is guest memory; bit 16 signals a missing user-data directory.
    // PT-BR: Bit 1 indica memoria guest; bit 16 indica caminho de dados ausente.
    constexpr int MEMORY_ERROR = 1;
    constexpr int USER_PATH_ERROR = 16;

    if ((valid & (MEMORY_ERROR | USER_PATH_ERROR)) != 0)
        return 1;
    if ((invalid & MEMORY_ERROR) == 0 || (invalid & USER_PATH_ERROR) == 0)
        return 2;

    return 0;
}
