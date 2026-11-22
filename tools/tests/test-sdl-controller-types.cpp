#include <SDL.h>
#include <hid/driver/sdl_controller_types.h>
#include <cstdio>
#include <string_view>

/**
 * EN: Exercise real SDL2 enum-to-model mappings on CI without USB, Bluetooth,
 *     SDL_Init or licensed game data. Hotplug behavior needs device testing.
 * PT-BR: Testa os tipos de controles SDL2 sem USB/Bluetooth ou dados do jogo.
 *     O comportamento real de reconexão ainda exige hardware.
 */
int main()
{
    struct Case
    {
        SDL_GameControllerType input;
        hid::EInputDeviceExplicit expected;
        hid::EInputDevice prompts;
        std::string_view name;
        bool fullPad;
    };
    constexpr Case cases[] = {
        {SDL_CONTROLLER_TYPE_XBOX360, hid::EInputDeviceExplicit::Xbox360,
            hid::EInputDevice::Xbox, "Xbox 360", true},
        {SDL_CONTROLLER_TYPE_XBOXONE, hid::EInputDeviceExplicit::XboxOne,
            hid::EInputDevice::Xbox, "Xbox One", true},
        {SDL_CONTROLLER_TYPE_PS4, hid::EInputDeviceExplicit::DualShock4,
            hid::EInputDevice::PlayStation, "DualShock 4", true},
        {SDL_CONTROLLER_TYPE_PS5, hid::EInputDeviceExplicit::DualSense,
            hid::EInputDevice::PlayStation, "DualSense", true},
        {SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO, hid::EInputDeviceExplicit::SwitchPro,
            hid::EInputDevice::Xbox, "Nintendo Switch Pro", true},
        {SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR, hid::EInputDeviceExplicit::SwitchJCPair,
            hid::EInputDevice::Xbox, "Nintendo Switch Joy-Con (Pair)", true},
        {SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT, hid::EInputDeviceExplicit::SwitchJCLeft,
            hid::EInputDevice::Xbox, "Nintendo Switch Joy-Con (Left)", false},
        {SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT, hid::EInputDeviceExplicit::SwitchJCRight,
            hid::EInputDevice::Xbox, "Nintendo Switch Joy-Con (Right)", false},
        {SDL_CONTROLLER_TYPE_UNKNOWN, hid::EInputDeviceExplicit::Unknown,
            hid::EInputDevice::Xbox, "Unknown SDL Controller", true},
    };
    for (const auto& item : cases)
    {
        if (hid::sdl::Identify(item.input) != item.expected ||
            hid::sdl::PromptFamily(item.input) != item.prompts ||
            hid::sdl::ModelName(item.input) != item.name ||
            hid::sdl::HasFullGamepad(item.input) != item.fullPad)
        {
            std::fprintf(stderr, "CONTROLLER_CLASSIFICATION|FAIL|type=%d\n",
                static_cast<int>(item.input));
            return 1;
        }
    }
    std::printf("CONTROLLER_CLASSIFICATION|PASS|models=%zu\n",
        sizeof(cases) / sizeof(cases[0]));
    return 0;
}
