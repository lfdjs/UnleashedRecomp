#pragma once

#include "controller_icon_style.h"

namespace ui::loading_controller
{
    /**
     * EN: The original Sonic Unleashed loading UI has only TWO built-in
     *     scene families: "360_*" and "ps3_*". This model selection is
     *     independent of external user-installed controller-art mods.
     * PT-BR: A tela de carregamento do jogo original tem somente DUAS
     *     familias: "360_*" e "ps3_*". A selecao e independente dos
     *     mods de desenhos instalados pelo usuario.
     */
    enum class Profile : uint8_t
    {
        XboxClassic,
        XboxOneSeries,
        PlayStationClassic,
        DualSense
    };

    constexpr Profile AutomaticProfile(
        hid::EInputDeviceExplicit model, hid::EInputDevice family) noexcept
    {
        switch (ui::controller_icons::AutomaticStyle(model))
        {
        case ui::controller_icons::Style::DualSense:
            return Profile::DualSense;
        case ui::controller_icons::Style::XboxSeries:
            return Profile::XboxOneSeries;
        case ui::controller_icons::Style::PlayStationClassic:
            return Profile::PlayStationClassic;
        default:
            // EN: Unknown HID/SDL models use the last known input family.
            // PT-BR: Modelo desconhecido usa a familia do ultimo controle.
            return family == hid::EInputDevice::PlayStation
                ? Profile::PlayStationClassic : Profile::XboxClassic;
        }
    }

    constexpr bool UsesPlayStationScene(Profile profile) noexcept
    {
        return profile == Profile::DualSense
            || profile == Profile::PlayStationClassic;
    }

    constexpr const char* ScenePrefix(Profile profile) noexcept
    {
        // EN: DO NOT write "ps5" or "xbs" into the game strings: those
        //     scenes do not exist in the original ui_loading.yncp.
        //     User-installed asset mods may replace the existing slots.
        // PT-BR: NUNCA escreve "ps5" ou "xbs" nas strings originais:
        //     essas cenas nao existem no ui_loading.yncp original.
        //     Mods podem substituir as artes das cenas ps3/360.
        return UsesPlayStationScene(profile) ? "ps3" : "360";
    }

    constexpr const char* Name(Profile profile) noexcept
    {
        switch (profile)
        {
        case Profile::DualSense: return "DualSense (ps3 scene slot)";
        case Profile::XboxOneSeries: return "Xbox One/Series (360 scene slot)";
        case Profile::PlayStationClassic: return "PlayStation classic (ps3 scene slot)";
        default: return "Xbox classic (360 scene slot)";
        }
    }
}
