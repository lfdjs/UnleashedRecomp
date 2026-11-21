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
} // namespace portable::input
