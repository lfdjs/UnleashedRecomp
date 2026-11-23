#pragma once

#include <cstdint>
// EN: Resolve from the adjacent UI directory in both the standalone
//     portable tests and the full game's include-path configuration.
// PT-BR: Caminho relativo funciona nos testes independentes e no jogo.
//     Evita depender de -I exclusivo do CMake principal.
#include "../hid/controller_types.h"

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
     * EN: Modern overlays replace ABXY on DualSense/Xbox One-Series. Only
     *     Xbox One-Series additionally replaces LB/RB shoulders; triggers,
     *     menu buttons and forced icon styles still use the original atlas.
     * PT-BR: DualSense e Xbox One-Series modernizam A/B/X/Y. Somente Xbox
     *     One-Series moderniza tambem LB/RB; gatilhos e menu usam DDS.
     */
    constexpr bool UsesModernFaceButtons(Style style) noexcept
    {
        return style == Style::XboxSeries || style == Style::DualSense;
    }

    /**
     * EN: The shoulder bumpers are redrawn only for Xbox One/Series.
     * PT-BR: Somente Xbox One/Series recebe LB e RB redesenhados.
     */
    constexpr bool UsesModernShoulderButtons(Style style) noexcept
    {
        return style == Style::XboxSeries;
    }
}
