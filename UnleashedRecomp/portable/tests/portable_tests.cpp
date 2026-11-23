#include "../platform.h"
#include "../graphics.h"
#include "../audio.h"
#include "../input.h"
#include "../threading.h"
#include "../memory.h"
#include "../guest_address.h"
#include "../process.h"
#include <limits>
#include "../filesystem.h"
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <string>
#include <unordered_map>
// EN: Test the real host-to-Xbox language mapping without game assets.
// PT-BR: Testa a conversão de idioma sem precisar dos dados do jogo.
#include "../../locale/locale.h"
// EN/PT-BR: Pure controller-icon selection without image or GPU assets.
#include "../../ui/controller_icon_style.h"
#include "../../ui/loading_controller_profile.h"
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
    // EN: Legacy languages keep Xbox IDs 1-6. pt-BR and malformed settings
    //     must never be exposed as unsupported IDs to the guest engine.
    // PT-BR: Os seis idiomas originais mantêm IDs 1-6. pt-BR e valores
    //     inválidos não podem ser enviados diretamente ao motor Xbox.
    Check(XboxGuestLanguage(ELanguage::English) == 1, "Xbox language English");
    Check(XboxGuestLanguage(ELanguage::Japanese) == 2, "Xbox language Japanese");
    Check(XboxGuestLanguage(ELanguage::German) == 3, "Xbox language German");
    Check(XboxGuestLanguage(ELanguage::French) == 4, "Xbox language French");
    Check(XboxGuestLanguage(ELanguage::Spanish) == 5, "Xbox language Spanish");
    Check(XboxGuestLanguage(ELanguage::Italian) == 6, "Xbox language Italian");
    Check(XboxGuestLanguage(ELanguage::PortugueseBrazil) == 1, "Xbox fallback for pt-BR");
    Check(XboxGuestLanguage(static_cast<ELanguage>(0)) == 1, "Xbox fallback for invalid language");
    // EN: Modern icons follow detected models without changing the
    //     original Xbox/PlayStation icon configuration or game assets.
    // PT-BR: Icones modernos seguem o modelo detectado, sem alterar
    //     recursos originais nem a selecao manual de icones.
    using ui::controller_icons::Style;
    using ui::controller_icons::AutomaticStyle;
    using ui::controller_icons::UsesModernFaceButtons;
    Check(AutomaticStyle(hid::EInputDeviceExplicit::DualSense) == Style::DualSense,
        "DualSense uses white PlayStation symbols");
    Check(AutomaticStyle(hid::EInputDeviceExplicit::DualShock4) == Style::PlayStationClassic,
        "DualShock 4 keeps classic PlayStation icons");
    Check(AutomaticStyle(hid::EInputDeviceExplicit::XboxOne) == Style::XboxSeries,
        "Xbox Series and One family uses modern ABXY");
    Check(AutomaticStyle(hid::EInputDeviceExplicit::Xbox360) == Style::XboxClassic,
        "Xbox 360 keeps classic atlas");
    Check(AutomaticStyle(hid::EInputDeviceExplicit::SwitchPro) == Style::XboxClassic,
        "Nintendo Pro keeps Xbox fallback prompts");
    Check(AutomaticStyle(hid::EInputDeviceExplicit::SwitchHandheld) == Style::XboxClassic,
        "Switch handheld keeps Xbox fallback prompts");
    Check(UsesModernFaceButtons(Style::DualSense), "DualSense face overlay active");
    Check(UsesModernFaceButtons(Style::XboxSeries), "Xbox Series face overlay active");
    Check(!UsesModernFaceButtons(Style::PlayStationClassic),
        "Legacy PlayStation DDS remains unchanged");
    // EN: Xbox One/Series gets modern LB/RB while DualSense retains
    //     its own original L1/R1 atlas. Manual forced icons are separate.
    // PT-BR: Xbox One/Series moderniza LB/RB; DualSense preserva L1/R1.
    using ui::controller_icons::UsesModernShoulderButtons;
    Check(UsesModernShoulderButtons(Style::XboxSeries),
        "Xbox One/Series modern LB and RB");
    Check(!UsesModernShoulderButtons(Style::DualSense),
        "DualSense L1/R1 atlas preserved");
    Check(!UsesModernShoulderButtons(Style::XboxClassic),
        "Xbox 360 bumpers preserved");
    Check(!UsesModernShoulderButtons(Style::PlayStationClassic),
        "DualShock 4 shoulders preserved");

    // EN: The guest owns exactly two scene families. Test mapping model
    //     -> profile -> valid prefix without any copyrighted YNCP assets.
    // PT-BR: As cenas do guest aceitam apenas 360/ps3; verifica o
    //     mapeamento sem precisar dos arquivos licenciados do jogo.
    using ui::loading_controller::Profile;
    using ui::loading_controller::AutomaticProfile;
    using ui::loading_controller::ScenePrefix;
    using ui::loading_controller::UsesPlayStationScene;
    const auto dualSenseLoading = AutomaticProfile(
        hid::EInputDeviceExplicit::DualSense, hid::EInputDevice::PlayStation);
    const auto seriesLoading = AutomaticProfile(
        hid::EInputDeviceExplicit::XboxOne, hid::EInputDevice::Xbox);
    Check(dualSenseLoading == Profile::DualSense, "DualSense loading model");
    Check(seriesLoading == Profile::XboxOneSeries, "Xbox Series loading model");
    Check(std::string_view(ScenePrefix(dualSenseLoading)) == "ps3",
        "DualSense uses existing PS3 loading scene slot");
    Check(std::string_view(ScenePrefix(seriesLoading)) == "360",
        "Xbox One/Series uses existing 360 loading scene slot");
    Check(UsesPlayStationScene(dualSenseLoading), "DualSense loading PlayStation branch");
    Check(!UsesPlayStationScene(seriesLoading), "Xbox Series loading Xbox branch");
    Check(AutomaticProfile(hid::EInputDeviceExplicit::DualShock4,
        hid::EInputDevice::Xbox) == Profile::PlayStationClassic,
        "DS4 model is preferred over stale fallback family");
    Check(AutomaticProfile(hid::EInputDeviceExplicit::Xbox360,
        hid::EInputDevice::PlayStation) == Profile::PlayStationClassic,
        "Xbox 360 fallback family follows existing icon selection policy");
    Check(AutomaticProfile(hid::EInputDeviceExplicit::Unknown,
        hid::EInputDevice::PlayStation) == Profile::PlayStationClassic,
        "unknown model retains PlayStation family fallback");
    Check(AutomaticProfile(hid::EInputDeviceExplicit::Unknown,
        hid::EInputDevice::Xbox) == Profile::XboxClassic,
        "unknown model retains Xbox fallback");



    Check(portable::graphics::IsCompiled(portable::graphics::Backend::Vulkan), "Vulkan compiled");
    Check(!portable::graphics::IsCompiled(static_cast<portable::graphics::Backend>(255)),
        "unknown graphics backend rejected");
    // EN: Validate guest-address boundaries without allocating 4 GiB.
    // PT-BR: Valida limites de enderecos do jogo sem alocar 4 GiB.
    constexpr std::uintptr_t guestBase = 0x100000000ULL;
    constexpr std::size_t guestBytes = 0x100000000ULL;
    Check(portable::memory::ContainsHostRange(guestBase, guestBytes, guestBase), "guest base");
    Check(!portable::memory::ContainsHostRange(guestBase, guestBytes, 0), "null host pointer");
    Check(!portable::memory::ContainsHostRange(guestBase, guestBytes, guestBase - 1), "address before region");
    Check(portable::memory::ContainsHostRange(guestBase, guestBytes, guestBase + guestBytes - 1), "last guest byte");
    Check(!portable::memory::ContainsHostRange(guestBase, guestBytes, guestBase + guestBytes), "guest one-past-end");
    Check(!portable::memory::ContainsHostRange(guestBase, guestBytes, guestBase + guestBytes - 1, 2), "length beyond guest end");
    Check(!portable::memory::ContainsHostRange(std::numeric_limits<std::uintptr_t>::max() - 4,
        guestBytes, std::numeric_limits<std::uintptr_t>::max() - 4), "host address overflow rejected");
    std::uint32_t offset = 99;
    Check(portable::memory::TryGuestOffset(guestBase, guestBytes, guestBase + 0x1234, offset) &&
        offset == 0x1234, "guest address translation");
    Check(!portable::memory::TryGuestOffset(guestBase, guestBytes, guestBase + guestBytes, offset),
        "invalid guest address translation");
    Check(portable::audio::BytesPerBlock(2, 256, 4) == 2048, "audio bytes per block");
    Check(portable::audio::BytesPerBlock(UINT32_MAX, UINT32_MAX, UINT32_MAX) == 0,
        "audio overflow rejected");
    Check(!portable::audio::NeedsRefill(0, UINT32_MAX, UINT32_MAX, UINT32_MAX, 10),
        "audio refill overflow rejected");
    Check(portable::audio::QueuedBlocks(4096, 2, 256, 4) == 2, "audio queue");
    Check(portable::audio::QueuedBlocks(1, 0, 0, 0) == 0, "audio zero divisor");
    Check(!portable::audio::NeedsRefill(0, 0, 256, 4, 10), "invalid audio");
    Check(portable::audio::NeedsRefill(0, 2, 256, 4, 10), "audio refill");
    Check(portable::input::ClampAxis(-50000) == -32768, "negative axis");
    Check(portable::input::ClampAxis(50000) == 32767, "positive axis");
    Check(portable::input::DigitalTrigger(true) == 255, "digital trigger");
    // EN: Verify Nintendo controller layouts and the full two-stick
    //     requirement without a Switch SDK or physical controllers.
    // PT-BR: Verifica Pro, Joy-Con pareados/separados e modo portátil
    //     sem SDK do Switch e sem controles físicos.
    using portable::input::NintendoLayout;
    using portable::input::DetectNintendoLayout;
    using portable::input::HasFullGamepad;
    Check(DetectNintendoLayout(false, portable::input::SwitchFullKeyBit)
        == NintendoLayout::ProController, "Switch Pro identified");
    Check(DetectNintendoLayout(false, portable::input::SwitchJoyDualBit)
        == NintendoLayout::JoyConPair, "paired Joy-Cons identified");
    Check(DetectNintendoLayout(false, portable::input::SwitchJoyLeftBit)
        == NintendoLayout::JoyConLeft, "left Joy-Con identified");
    Check(DetectNintendoLayout(false, portable::input::SwitchJoyRightBit)
        == NintendoLayout::JoyConRight, "right Joy-Con identified");
    Check(DetectNintendoLayout(true, portable::input::SwitchFullKeyBit)
        == NintendoLayout::Handheld, "handheld overrides docked style");
    Check(DetectNintendoLayout(false, 0) == NintendoLayout::Unknown,
        "disconnected Switch controller rejected");
    Check(HasFullGamepad(NintendoLayout::Handheld), "handheld full gamepad");
    Check(HasFullGamepad(NintendoLayout::JoyConPair), "Joy-Con pair full gamepad");
    Check(HasFullGamepad(NintendoLayout::ProController), "Switch Pro full gamepad");
    Check(!HasFullGamepad(NintendoLayout::JoyConLeft), "single left Joy-Con limited");
    Check(!HasFullGamepad(NintendoLayout::JoyConRight), "single right Joy-Con limited");

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
#if defined(__linux__) && !defined(__ANDROID__)
    // EN: Test both successful exec and errors on the detached Linux launcher.
    // PT-BR: Testa exec bem-sucedido e falhas no lancador Linux separado.
    Check(portable::process::StartDetachedLinux("/bin/true", {}), "spawn existing process");
    Check(!portable::process::StartDetachedLinux(
        "/unlikely_unleashed_recomp_executable_missing_314159", {}), "reject nonexistent process");
    Check(!portable::process::StartDetachedLinux("/bin/true", {},
        "/unlikely_unleashed_working_dir_missing_314159"), "reject invalid working directory");
    Check(!portable::process::StartDetachedLinux({}, {}), "reject empty executable");
#endif
#if defined(__linux__) && defined(__x86_64__)
    // EN: Reserve the full 4 GiB Xbox 360 address range without touching it;
    //     POSIX only consumes virtual address space for this smoke check.
    // PT-BR: Reserva os 4 GiB virtuais do Xbox 360 sem ocupar toda a RAM
    //     fisica, validando o requisito principal do runtime em Linux x64.
    constexpr std::size_t fullGuestBytes = 0x100000000ULL;
    const auto fullGuest = portable::memory::AllocateDesktop(fullGuestBytes);
    Check(fullGuest.base != nullptr && fullGuest.size == fullGuestBytes,
        "full 4 GiB guest virtual reservation");
    if (fullGuest.base)
    {
        Check(portable::memory::ProtectNull(fullGuest, hostPage), "4 GiB null guard");
        portable::memory::ReleaseDesktop(fullGuest);
    }
#endif
    portable::threading::Yield();
    portable::threading::SleepFor(std::chrono::nanoseconds::zero());
    std::cout << (failures ? "Portable tests failed\n" : "Portable tests passed\n");
    return failures ? 1 : 0;
}
