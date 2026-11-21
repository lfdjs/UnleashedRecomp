#pragma once
#include <cstdint>

namespace portable {
enum class HostOS : uint8_t { Windows, Linux, MacOS, Switch, Android, Unknown };
enum class Cpu : uint8_t { X64, ARM64, Unknown };
struct Capabilities {
    bool desktopWindow, nativeFilePicker, processLaunch, vulkanBackend, d3d12Backend, consoleStorage;
};
/** EN: Detect the target operating system at compile time, not the build host.
 *  PT-BR: Detecta o sistema operacional de destino, nao a maquina de compilacao. */
constexpr HostOS CurrentOS() noexcept {
#if defined(__SWITCH__) || defined(UNLEASHED_RECOMP_SWITCH)
    return HostOS::Switch;
#elif defined(__ANDROID__)
    return HostOS::Android;
#elif defined(_WIN32)
    return HostOS::Windows;
#elif defined(__APPLE__)
    return HostOS::MacOS;
#elif defined(__linux__)
    return HostOS::Linux;
#else
    return HostOS::Unknown;
#endif
}
/** EN: Detect the target CPU ISA; host and target may differ in cross builds.
 *  PT-BR: Identifica a arquitetura de destino, inclusive em compilacao cruzada. */
constexpr Cpu CurrentCPU() noexcept {
#if defined(__aarch64__) || defined(_M_ARM64)
    return Cpu::ARM64;
#elif defined(__x86_64__) || defined(_M_X64)
    return Cpu::X64;
#else
    return Cpu::Unknown;
#endif
}
/** EN: Return a diagnostic OS name without allocations or exceptions.
 *  PT-BR: Retorna o nome do SO para logs sem alocar memoria. */
constexpr const char* Name(HostOS os) noexcept {
    switch(os) {
    case HostOS::Windows: return "Windows";
    case HostOS::Linux: return "Linux";
    case HostOS::MacOS: return "macOS";
    case HostOS::Switch: return "Nintendo Switch";
    case HostOS::Android: return "Android";
    default: return "Unknown";
    }
}
/** EN: Return a diagnostic CPU name.
 *  PT-BR: Retorna o nome da arquitetura para diagnosticos. */
constexpr const char* Name(Cpu cpu) noexcept {
    switch(cpu) {
    case Cpu::X64: return "x86_64";
    case Cpu::ARM64: return "arm64";
    default: return "unknown";
    }
}
/** EN: Describe compiled APIs, not actual GPU capabilities.
 *  PT-BR: Descreve APIs compiladas, nao as capacidades reais da GPU. */
constexpr Capabilities BuildCapabilities() noexcept {
    constexpr bool desktop = CurrentOS() == HostOS::Windows ||
                             CurrentOS() == HostOS::Linux || CurrentOS() == HostOS::MacOS;
#if defined(_WIN32) && defined(UNLEASHED_RECOMP_D3D12)
    constexpr bool d3d12 = true;
#else
    constexpr bool d3d12 = false;
#endif
    return { desktop, desktop, desktop, true, d3d12, CurrentOS() == HostOS::Switch };
}
} // namespace portable
