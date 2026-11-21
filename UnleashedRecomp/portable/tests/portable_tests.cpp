#include "../platform.h"
#include "../graphics.h"
#include "../audio.h"
#include "../input.h"
#include "../threading.h"
#include "../memory.h"
#include "../filesystem.h"
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <system_error>

static int failures = 0;
/** EN: Check a condition even in Release builds with NDEBUG defined.
 *  PT-BR: Confere a condicao tambem em builds Release com NDEBUG. */
static void Check(bool value, std::string_view message) {
    if (!value) { ++failures; std::cerr << "FAILED: " << message << '\n'; }
}
/** EN: Execute unit tests for metadata, memory, paths, audio and controller math.
 *  PT-BR: Testa identificacao, memoria, caminhos, audio e conversao de controles. */
int main() {
    Check(portable::CurrentOS() != portable::HostOS::Unknown, "supported desktop OS");
    Check(portable::CurrentCPU() != portable::Cpu::Unknown, "supported desktop architecture");
    Check(portable::BuildCapabilities().desktopWindow, "desktop capabilities");
    Check(portable::graphics::IsCompiled(portable::graphics::Backend::Vulkan), "Vulkan compiled");
    Check(portable::audio::QueuedBlocks(4096, 2, 256, 4) == 2, "audio queue");
    Check(portable::audio::QueuedBlocks(1, 0, 0, 0) == 0, "audio zero divisor");
    Check(!portable::audio::NeedsRefill(0, 0, 256, 4, 10), "invalid audio");
    Check(portable::audio::NeedsRefill(0, 2, 256, 4, 10), "audio refill");
    Check(portable::input::ClampAxis(-50000) == -32768, "negative axis");
    Check(portable::input::ClampAxis(50000) == 32767, "positive axis");
    Check(portable::input::DigitalTrigger(true) == 255, "digital trigger");
    Check(portable::memory::AllocateDesktop(0).base == nullptr, "zero allocation rejected");
    const auto hostPage = portable::memory::HostPageSize();
    Check(hostPage >= 4096, "native page size available");
    auto region = portable::memory::AllocateDesktop(65536, 0);
    Check(region.base != nullptr, "guest virtual memory allocation");
    if (region.base) {
        // EN: macOS ARM64 typically has 16 KiB pages; do not assume 4 KiB.
        // PT-BR: O macOS ARM64 normalmente usa paginas de 16 KiB.
        Check(portable::memory::ProtectNull(region, hostPage), "native null guard");
        if (hostPage > 4096)
            Check(!portable::memory::ProtectNull(region, 4096), "non-native guest guard rejected");
        Check(!portable::memory::ProtectNull(region, 65537), "oversized guard rejected");
        auto* bytes = static_cast<uint8_t*>(region.base);
        bytes[hostPage] = 0x5a;
        Check(bytes[hostPage] == 0x5a, "writable next host page");
        portable::memory::ReleaseDesktop(region);
    }
    Check(!portable::filesystem::UserDataRoot("UnleashedRecomp").empty(), "user path");
    std::error_code ec;
    auto temp = std::filesystem::temp_directory_path(ec);
    if (!ec) {
        auto path = temp / "unleashed_portable_unit_test";
        Check(portable::filesystem::EnsureDirectory(path, ec), "create directory");
        Check(portable::filesystem::EnsureDirectory(path, ec), "existing directory");
        std::filesystem::remove(path, ec);
    }
    portable::threading::Yield();
    portable::threading::SleepFor(std::chrono::nanoseconds::zero());
    std::cout << (failures ? "Portable tests failed\n" : "Portable tests passed\n");
    return failures ? 1 : 0;
}
