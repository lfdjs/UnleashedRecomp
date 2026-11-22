#pragma once

enum class ELanguage : uint32_t
{
    English = 1,
    Japanese,
    German,
    French,
    Spanish,
    Italian,
    // EN: Brazilian Portuguese is a new UI language; preserve legacy enum IDs.
    // PT-BR: Novo idioma da interface, sem alterar os IDs dos anteriores.
    PortugueseBrazil
};

/**
 * EN: The recompiled Xbox 360 game understands only language IDs 1 through
 *     6. Portuguese (Brazil) is a HOST UI language, so offer the original
 *     English resources to the guest until explicitly translated assets
 *     can be supplied through compatible game-data mods.
 * PT-BR: O jogo Xbox 360 conhece apenas os idiomas 1 a 6. O português
 *     brasileiro é um idioma da INTERFACE do port; o jogo recebe inglês
 *     enquanto os recursos traduzidos são carregados via mods compatíveis.
 */
constexpr uint32_t XboxGuestLanguage(ELanguage selected) noexcept
{
    switch (selected)
    {
    case ELanguage::English:
    case ELanguage::Japanese:
    case ELanguage::German:
    case ELanguage::French:
    case ELanguage::Spanish:
    case ELanguage::Italian:
        return static_cast<uint32_t>(selected);
    case ELanguage::PortugueseBrazil:
    default:
        return static_cast<uint32_t>(ELanguage::English);
    }
}

inline std::string g_localeMissing = "<missing string>";

extern std::unordered_map<std::string_view, std::unordered_map<ELanguage, std::string>> g_locale;

std::string& Localise(const std::string_view& key);
