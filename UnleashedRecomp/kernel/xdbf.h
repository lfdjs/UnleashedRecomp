#pragma once

#include <gpu/video.h>
#include <xdbf_wrapper.h>
#include <locale/locale.h>

extern XDBFWrapper g_xdbfWrapper;
extern std::unordered_map<uint16_t, GuestTexture*> g_xdbfTextureCache;

namespace xdbf
{
    /**
     * EN: Guest Xbox 360 XDBF achievement resources only provide the original
     *     six languages. Brazilian Portuguese localizes the port UI but has
     *     no XDBF translation, so use English guest assets for that locale.
     *     Unknown/corrupted config language values also fall back safely.
     * PT-BR: As conquistas XDBF do Xbox 360 possuem apenas os seis idiomas
     *     originais. Português (Brasil) traduz a interface do port, não os
     *     recursos XDBF; por isso usamos inglês nesses dados do jogo.
     *     Idiomas inválidos na configuração também retornam ao inglês.
     */
    inline EXDBFLanguage GetAssetLanguage(ELanguage language) noexcept
    {
        // EN: Share the same guest-language mapping with XGetLanguage and
        //     ExGetXConfigSetting, avoiding inconsistent Xbox resource IDs.
        // PT-BR: Compartilha o mapeamento com XGetLanguage e
        //     ExGetXConfigSetting para evitar IDs inconsistentes.
        return static_cast<EXDBFLanguage>(XboxGuestLanguage(language));
    }

    inline std::string& FixInvalidSequences(std::string& str)
    {
        static std::array<std::string_view, 1> invalidSequences =
        {
            "\xE2\x80\x99"
        };

        static std::array<std::string_view, 1> replaceSequences =
        {
            "'"
        };

        for (int i = 0; i < invalidSequences.size(); i++)
        {
            size_t pos = 0;

            auto& invalidSeq = invalidSequences[i];
            auto& replaceSeq = replaceSequences[i];

            while ((pos = str.find(invalidSequences[i], pos)) != std::string::npos)
            {
                str = str.replace(pos, invalidSeq.length(), replaceSeq);

                pos += replaceSeq.length();
            }
        }

        return str;
    }
}
