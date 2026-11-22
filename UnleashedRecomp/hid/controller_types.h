#pragma once
namespace hid
{
    // EN: Shared controller models for desktop SDL2 and native Switch.
    // PT-BR: Modelos de controles compartilhados entre SDL2 e Switch nativo.
    enum class EInputDevice
    {
        Keyboard,
        Mouse,
        Xbox,
        PlayStation
    };

    enum class EInputDeviceExplicit
    {
        Unknown,
        Xbox360,
        XboxOne,
        DualShock3,
        DualShock4,
        SwitchPro,
        Virtual,
        DualSense,
        Luna,
        Stadia,
        NvShield,
        SwitchJCLeft,
        SwitchJCRight,
        SwitchJCPair,
        // EN: Native Switch handheld (attached Joy-Cons); append to retain IDs.
        // PT-BR: Joy-Cons acoplados no modo portátil; mantém os IDs anteriores.
        SwitchHandheld
    };
}
