#pragma once

#include <cstdint>
#include <hid/controller_types.h>

namespace ui::controller_icons
{
    /**
     * EN: Select an icon visual from the real controller model, not only
     *     Xbox versus PlayStation. Numeric values are unrelated to SDL.
     * PT-BR: Escolhe o estilo visual pelo modelo real do controle, sem
     *     reduzir todos os dispositivos a Xbox ou PlayStation.
     */
    enum class Style : uint8_t
    {
        XboxClassic,
        PlayStationClassic,
        XboxSeries,
        DualSense
    };

    /**
     * EN: SDL2 reports Xbox Series controllers as XBOXONE. Use the modern
     *     face-button style for that entire group, retaining classic 360.
     * PT-BR: O SDL2 normalmente identifica Xbox Series como XBOXONE.
     *     Usa icones modernos para o grupo, mantendo Xbox 360 classico.
     */
    constexpr Style AutomaticStyle(hid::EInputDeviceExplicit model) noexcept
    {
        switch (model)
        {
        case hid::EInputDeviceExplicit::DualSense:
            return Style::DualSense;
        case hid::EInputDeviceExplicit::DualShock3:
        case hid::EInputDeviceExplicit::DualShock4:
            return Style::PlayStationClassic;
        case hid::EInputDeviceExplicit::XboxOne:
            return Style::XboxSeries;
        default:
            return Style::XboxClassic;
        }
    }

    /**
     * EN: The procedural overlay only replaces ABXY face buttons.
     *     All shoulder, trigger, menu and keyboard artwork remains in the
     *     original atlas until a complete licensed replacement is available.
     * PT-BR: O desenho vetorial substitui apenas A/B/X/Y. Ombros, gatilhos,
     *     menu e teclado continuam usando o atlas original.
     */
    constexpr bool UsesModernFaceButtons(Style style) noexcept
    {
        return style == Style::XboxSeries || style == Style::DualSense;
    }
}
