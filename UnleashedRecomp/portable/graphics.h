#pragma once
#include "platform.h"

namespace portable::graphics {
enum class Backend { Vulkan, Direct3D12 };
/** EN: Choose the build's preferred renderer; Plume still probes physical devices.
 *  PT-BR: Seleciona o renderizador preferido; o Plume verifica a GPU de verdade. */
constexpr Backend PreferredBackend() noexcept {
#if defined(_WIN32) && defined(UNLEASHED_RECOMP_D3D12)
    return Backend::Direct3D12;
#else
    return Backend::Vulkan;
#endif
}
/** EN: Tell whether a renderer was compiled into the application.
 *  PT-BR: Informa se a API grafica esta presente no executavel. */
constexpr bool IsCompiled(Backend backend) noexcept {
    // EN: Reject unknown enum values even on Windows builds with D3D12 enabled.
    // PT-BR: Rejeita valores desconhecidos mesmo quando D3D12 esta habilitado.
    switch (backend) {
    case Backend::Vulkan: return BuildCapabilities().vulkanBackend;
    case Backend::Direct3D12: return BuildCapabilities().d3d12Backend;
    default: return false;
    }
}
/** EN: Provide readable, stable renderer labels.
 *  PT-BR: Fornece nomes estaveis para registros do renderizador. */
constexpr const char* Name(Backend backend) noexcept {
    return backend == Backend::Vulkan ? "Vulkan" : "Direct3D 12";
}
} // namespace portable::graphics
