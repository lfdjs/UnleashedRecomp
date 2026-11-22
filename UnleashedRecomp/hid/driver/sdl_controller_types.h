#pragma once
#include <SDL.h>
#include <hid/controller_types.h>

namespace hid::sdl
{
    /**
     * EN: Convert SDL device types explicitly; numeric enum casts are
     *     fragile across SDL versions and can misidentify PlayStation/Nintendo.
     * PT-BR: Converte os tipos SDL explicitamente, sem depender de valores
     *     numéricos coincidentes entre duas enumerações distintas.
     */
    constexpr EInputDeviceExplicit Identify(SDL_GameControllerType type) noexcept
    {
        switch (type)
        {
        case SDL_CONTROLLER_TYPE_XBOX360: return EInputDeviceExplicit::Xbox360;
        case SDL_CONTROLLER_TYPE_XBOXONE: return EInputDeviceExplicit::XboxOne;
        case SDL_CONTROLLER_TYPE_PS3: return EInputDeviceExplicit::DualShock3;
        case SDL_CONTROLLER_TYPE_PS4: return EInputDeviceExplicit::DualShock4;
        case SDL_CONTROLLER_TYPE_PS5: return EInputDeviceExplicit::DualSense;
        case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO: return EInputDeviceExplicit::SwitchPro;
        case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT: return EInputDeviceExplicit::SwitchJCLeft;
        case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT: return EInputDeviceExplicit::SwitchJCRight;
        case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR: return EInputDeviceExplicit::SwitchJCPair;
        case SDL_CONTROLLER_TYPE_VIRTUAL: return EInputDeviceExplicit::Virtual;
        case SDL_CONTROLLER_TYPE_AMAZON_LUNA: return EInputDeviceExplicit::Luna;
        case SDL_CONTROLLER_TYPE_GOOGLE_STADIA: return EInputDeviceExplicit::Stadia;
        case SDL_CONTROLLER_TYPE_NVIDIA_SHIELD: return EInputDeviceExplicit::NvShield;
        default: return EInputDeviceExplicit::Unknown;
        }
    }

    /**
     * EN: Stable human-readable device name for logs and preflight reports.
     * PT-BR: Nome legível do dispositivo para logs e diagnósticos.
     */
    constexpr const char* ModelName(SDL_GameControllerType type) noexcept
    {
        switch (Identify(type))
        {
        case EInputDeviceExplicit::Xbox360: return "Xbox 360";
        case EInputDeviceExplicit::XboxOne: return "Xbox One";
        case EInputDeviceExplicit::DualShock3: return "DualShock 3";
        case EInputDeviceExplicit::DualShock4: return "DualShock 4";
        case EInputDeviceExplicit::DualSense: return "DualSense";
        case EInputDeviceExplicit::SwitchPro: return "Nintendo Switch Pro";
        case EInputDeviceExplicit::SwitchJCLeft: return "Nintendo Switch Joy-Con (Left)";
        case EInputDeviceExplicit::SwitchJCRight: return "Nintendo Switch Joy-Con (Right)";
        case EInputDeviceExplicit::SwitchJCPair: return "Nintendo Switch Joy-Con (Pair)";
        case EInputDeviceExplicit::Luna: return "Amazon Luna";
        case EInputDeviceExplicit::Stadia: return "Google Stadia";
        case EInputDeviceExplicit::NvShield: return "NVIDIA Shield";
        case EInputDeviceExplicit::Virtual: return "Virtual Controller";
        default: return "Unknown SDL Controller";
        }
    }

    /**
     * EN: The current UI has PlayStation and Xbox prompt textures only.
     *     Nintendo input is mapped to Xbox prompts until a Nintendo atlas
     *     is provided; the actual device name remains accurate.
     * PT-BR: A interface tem ícones de PlayStation e Xbox; controles da
     *     Nintendo usam ícones Xbox temporariamente, mas são reconhecidos
     *     pelo nome correto no registro de execução.
     */
    constexpr EInputDevice PromptFamily(SDL_GameControllerType type) noexcept
    {
        switch (type)
        {
        case SDL_CONTROLLER_TYPE_PS3:
        case SDL_CONTROLLER_TYPE_PS4:
        case SDL_CONTROLLER_TYPE_PS5:
            return EInputDevice::PlayStation;
        default:
            return EInputDevice::Xbox;
        }
    }

    /**
     * EN: Paired Joy-Cons, handheld and Pro have two sticks; individual
     *     Joy-Cons appear separately but may not control every game action.
     * PT-BR: Pro e Joy-Con pareados possuem dois analógicos; cada Joy-Con
     *     isolado pode não fornecer todos os comandos necessários.
     */
    constexpr bool HasFullGamepad(SDL_GameControllerType type) noexcept
    {
        return type != SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT
            && type != SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT;
    }
}
