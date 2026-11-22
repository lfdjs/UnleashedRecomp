#pragma once
#include <cstdint>

namespace portable::input {
/** EN: Clamp any native analog axis to the Xbox 360 signed 16-bit range.
 *  PT-BR: Limita um eixo analogico nativo ao intervalo de 16 bits do Xbox 360. */
constexpr int16_t ClampAxis(int32_t value) noexcept {
    return static_cast<int16_t>(value < -32768 ? -32768 : (value > 32767 ? 32767 : value));
}
/** EN: Convert a digital trigger to the full 8-bit Xbox trigger scale.
 *  PT-BR: Converte um gatilho digital para a escala de 8 bits do Xbox. */
constexpr uint8_t DigitalTrigger(bool pressed) noexcept {
    return pressed ? 255 : 0;
}
/**
 * EN: libnx Npad style bits are independent of desktop SDL; expose only
 *     numeric values so platform-independent tests can verify handheld,
 *     Pro Controller, and paired/separate Joy-Con layouts.
 * PT-BR: Os bits do Npad no libnx independem do SDL; a classificação
 *     pode ser testada sem hardware e sem o SDK do Nintendo Switch.
 */
enum class NintendoLayout : uint8_t {
    Unknown, ProController, JoyConPair, JoyConLeft, JoyConRight, Handheld
};

constexpr uint32_t SwitchFullKeyBit = 1u << 0;
constexpr uint32_t SwitchHandheldBit = 1u << 1;
constexpr uint32_t SwitchJoyDualBit = 1u << 2;
constexpr uint32_t SwitchJoyLeftBit = 1u << 3;
constexpr uint32_t SwitchJoyRightBit = 1u << 4;

constexpr NintendoLayout DetectNintendoLayout(bool handheld, uint32_t styles) noexcept
{
    if (handheld || (styles & SwitchHandheldBit) != 0)
        return NintendoLayout::Handheld;
    if ((styles & SwitchJoyDualBit) != 0)
        return NintendoLayout::JoyConPair;
    if ((styles & SwitchFullKeyBit) != 0)
        return NintendoLayout::ProController;
    if ((styles & SwitchJoyLeftBit) != 0)
        return NintendoLayout::JoyConLeft;
    if ((styles & SwitchJoyRightBit) != 0)
        return NintendoLayout::JoyConRight;
    return NintendoLayout::Unknown;
}

/**
 * EN: Single horizontal Joy-Cons lack the full two-stick Xbox 360 layout.
 * PT-BR: Um Joy-Con isolado não possui dois analógicos como no Xbox 360.
 */
constexpr bool HasFullGamepad(NintendoLayout type) noexcept
{
    return type == NintendoLayout::ProController
        || type == NintendoLayout::JoyConPair
        || type == NintendoLayout::Handheld;
}

} // namespace portable::input
