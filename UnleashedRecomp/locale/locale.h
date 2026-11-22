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

inline std::string g_localeMissing = "<missing string>";

extern std::unordered_map<std::string_view, std::unordered_map<ELanguage, std::string>> g_locale;

std::string& Localise(const std::string_view& key);
