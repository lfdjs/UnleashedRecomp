#pragma once

#include <filesystem>

namespace portable::diagnostics
{
    /**
     * EN: Run a voluntary, non-destructive host preflight before the game
     *     loads assets or creates its renderer. Check guest memory reservation,
     *     user-data root, SDL video/audio/input initialization and Vulkan loader.
     *     Result is a bitmask: 1=memory, 2=video, 4=Vulkan, 8=audio,
     *     16=missing user-data root. No error bit means checks passed.
     * PT-BR: Executa diagnostico opcional, sem alterar arquivos, antes de
     *     carregar os dados do jogo ou iniciar a renderizacao. Verifica memoria,
     *     caminho de dados, SDL (video/audio/controles) e carregador Vulkan.
     *     Resultado em bits: 1=memoria, 2=video, 4=Vulkan, 8=audio,
     *     16=diretorio de usuario ausente. Zero indica verificacoes aprovadas.
     */
    int RunHostPreflight(bool guestMemoryReserved,
        const std::filesystem::path& userDataRoot);
}
