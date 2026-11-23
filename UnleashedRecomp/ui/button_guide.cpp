#include "button_guide.h"
#include "imgui_utils.h"
#include <gpu/imgui/imgui_snapshot.h>
#include <gpu/video.h>
#include <hid/hid.h>
#include <ui/controller_icon_style.h>
#include <patches/aspect_ratio_patches.h>
#include <user/config.h>
#include <app.h>
#include <decompressor.h>

#include <res/images/common/controller.dds.h>
#include <res/images/common/kbm.dds.h>

constexpr float DEFAULT_SIDE_MARGINS = 379;

ImFont* g_fntNewRodin;

std::unique_ptr<GuestTexture> g_upControllerIcons;
std::unique_ptr<GuestTexture> g_upKBMIcons;

float g_sideMargins = DEFAULT_SIDE_MARGINS;

static std::vector<Button> g_buttons;

std::unordered_map<EButtonIcon, float> g_iconWidths =
{
    { EButtonIcon::A, 40 },
    { EButtonIcon::B, 40 },
    { EButtonIcon::X, 40 },
    { EButtonIcon::Y, 40 },
    { EButtonIcon::LB, 70 },
    { EButtonIcon::RB, 70 },
    { EButtonIcon::LBRB, 70 },
    { EButtonIcon::LT, 42 },
    { EButtonIcon::RT, 42 },
    { EButtonIcon::LTRT, 42 },
    { EButtonIcon::Start, 40 },
    { EButtonIcon::Back, 40 },
    { EButtonIcon::LMB, 40 },
    { EButtonIcon::Enter, 40 },
    { EButtonIcon::Escape, 40 },
};

std::unordered_map<EButtonIcon, float> g_iconHeights =
{
    { EButtonIcon::A, 40 },
    { EButtonIcon::B, 40 },
    { EButtonIcon::X, 40 },
    { EButtonIcon::Y, 40 },
    { EButtonIcon::LB, 40 },
    { EButtonIcon::RB, 40 },
    { EButtonIcon::LBRB, 40 },
    { EButtonIcon::LT, 42 },
    { EButtonIcon::RT, 42 },
    { EButtonIcon::LTRT, 42 },
    { EButtonIcon::Start, 40 },
    { EButtonIcon::Back, 40 },
    { EButtonIcon::LMB, 40 },
    { EButtonIcon::Enter, 40 },
    { EButtonIcon::Escape, 40 },
};

std::tuple<std::tuple<ImVec2, ImVec2>, GuestTexture*> GetButtonIcon(EButtonIcon icon)
{
    std::tuple<ImVec2, ImVec2> btn;
    GuestTexture* texture;

    auto isPlayStation = Config::ControllerIcons == EControllerIcons::Auto
        ? hid::g_inputDeviceController == hid::EInputDevice::PlayStation
        : Config::ControllerIcons == EControllerIcons::PlayStation;

    auto yOffsetCmn = isPlayStation ? 42 : 0;
    auto yOffsetStartBack = isPlayStation ? 46 : 0;

    switch (icon)
    {
        case EButtonIcon::A:
            btn = PIXELS_TO_UV_COORDS(512, 128, 0, yOffsetCmn, 40, 40);
            texture = g_upControllerIcons.get();
            break;

        case EButtonIcon::B:
            btn = PIXELS_TO_UV_COORDS(512, 128, 40, yOffsetCmn, 40, 40);
            texture = g_upControllerIcons.get();
            break;

        case EButtonIcon::X:
            btn = PIXELS_TO_UV_COORDS(512, 128, 80, yOffsetCmn, 40, 40);
            texture = g_upControllerIcons.get();
            break;

        case EButtonIcon::Y:
            btn = PIXELS_TO_UV_COORDS(512, 128, 120, yOffsetCmn, 40, 40);
            texture = g_upControllerIcons.get();
            break;

        case EButtonIcon::LB:
            btn = PIXELS_TO_UV_COORDS(512, 128, 166, yOffsetCmn, 70, 40);
            texture = g_upControllerIcons.get();
            break;

        case EButtonIcon::RB:
            btn = PIXELS_TO_UV_COORDS(512, 128, 246, yOffsetCmn, 70, 40);
            texture = g_upControllerIcons.get();
            break;

        case EButtonIcon::LT:
            btn = PIXELS_TO_UV_COORDS(512, 128, 320, yOffsetCmn, 40, 42);
            texture = g_upControllerIcons.get();
            break;

        case EButtonIcon::RT:
            btn = PIXELS_TO_UV_COORDS(512, 128, 360, yOffsetCmn, 40, 42);
            texture = g_upControllerIcons.get();
            break;

        case EButtonIcon::Start:
            btn = PIXELS_TO_UV_COORDS(512, 128, 455, yOffsetStartBack, 46, 46);
            texture = g_upControllerIcons.get();
            break;

        case EButtonIcon::Back:
            btn = PIXELS_TO_UV_COORDS(512, 128, 408, yOffsetStartBack, 46, 46);
            texture = g_upControllerIcons.get();
            break;

        case EButtonIcon::LMB:
            btn = PIXELS_TO_UV_COORDS(384, 128, 0, 0, 128, 128);
            texture = g_upKBMIcons.get();
            break;

        case EButtonIcon::Enter:
            btn = PIXELS_TO_UV_COORDS(384, 128, 128, 0, 128, 128);
            texture = g_upKBMIcons.get();
            break;

        case EButtonIcon::Escape:
            btn = PIXELS_TO_UV_COORDS(384, 128, 256, 0, 128, 128);
            texture = g_upKBMIcons.get();
            break;
    }

    return std::make_tuple(btn, texture);
}

/**
 * EN: Approximate the raised, bold lettering on Xbox One/Series controller
 *     keycaps. The shipped New Rodin font remains in use, but tiny offset
 *     passes thicken its strokes so X is visibly a *letter*, unlike the
 *     thin white PlayStation cross. No font files or DDS assets are added.
 * PT-BR: Aproxima a tipografia em negrito dos botoes Xbox One/Series.
 *     Pequenos deslocamentos engrossam as letras da fonte existente,
 *     diferenciando o X azul da cruz branca do PlayStation.
 */
static void DrawBoldXboxLabel(ImDrawList* drawList, const char* label,
    const ImVec2& center, float fontSize, ImU32 color)
{
    if (!g_fntNewRodin || !label)
        return;

    const ImVec2 textSize = g_fntNewRodin->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label);
    const ImVec2 origin{
        center.x - textSize.x * 0.5f,
        center.y - textSize.y * 0.5f
    };

    // EN: Font-weight approximation scales with the button artwork, while
    //     the dark shadow gives the colored glyph some visual depth.
    // PT-BR: O negrito cresce com o botao e a sombra realca a letra colorida.
    const float weight = std::max(0.85f, fontSize * 0.052f);
    drawList->AddText(g_fntNewRodin, fontSize,
        { origin.x + weight * 0.65f, origin.y + weight * 0.95f },
        IM_COL32(4, 5, 7, 200), label);

    drawList->AddText(g_fntNewRodin, fontSize,
        { origin.x - weight, origin.y }, color, label);
    drawList->AddText(g_fntNewRodin, fontSize,
        { origin.x + weight, origin.y }, color, label);
    drawList->AddText(g_fntNewRodin, fontSize,
        { origin.x, origin.y - weight }, color, label);
    drawList->AddText(g_fntNewRodin, fontSize,
        { origin.x, origin.y + weight }, color, label);
    drawList->AddText(g_fntNewRodin, fontSize, origin, color, label);
}

/**
 * EN: Use a rounded, low-profile bumper shape with white bold LB/RB
 *     lettering, closer to Xbox One/Series shoulder buttons than the
 *     classic Xbox 360 DDS strip. The 70x40 guide slot remains unchanged.
 * PT-BR: Desenha uma tecla alongada e arredondada com LB/RB brancos e
 *     fortes, mantendo o tamanho 70x40 do guia anterior.
 */
static void DrawXboxSeriesShoulder(ImDrawList* drawList, EButtonIcon icon,
    const ImVec2& iconMin, const ImVec2& iconMax)
{
    const float width = iconMax.x - iconMin.x;
    const float height = iconMax.y - iconMin.y;
    const float paddingX = width * 0.035f;
    const float paddingY = height * 0.12f;
    const ImVec2 min{ iconMin.x + paddingX, iconMin.y + paddingY };
    const ImVec2 max{ iconMax.x - paddingX, iconMax.y - paddingY };
    const float round = std::min(height * 0.30f, width * 0.16f);
    const ImVec2 center{ (min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f };

    // EN: Raised charcoal cap with a subtle rim and reflection.
    // PT-BR: Tecla de grafite com borda suave e reflexo discreto.
    drawList->AddRectFilled({ min.x, min.y + height * 0.055f },
        { max.x, max.y + height * 0.055f },
        IM_COL32(5, 6, 8, 240), round);
    drawList->AddRectFilled(min, max, IM_COL32(29, 32, 39, 255), round);
    drawList->AddRect(min, max, IM_COL32(144, 149, 160, 225),
        round, 0, std::max(1.0f, height * 0.035f));
    drawList->AddLine(
        { min.x + round, min.y + height * 0.055f },
        { max.x - round, min.y + height * 0.055f },
        IM_COL32(176, 184, 197, 125), std::max(1.0f, height * 0.03f));

    // EN: Shoulder labels are neutral white, not ABXY face-button colors.
    // PT-BR: LB/RB usam branco, diferente das letras coloridas A/B/X/Y.
    DrawBoldXboxLabel(drawList, icon == EButtonIcon::LB ? "LB" : "RB",
        center, height * 0.57f, IM_COL32(245, 246, 250, 255));
}

/**
 * EN: Automatically render modern controller prompts for the port-owned UI.
 *     DualSense keeps its white geometric face symbols. Xbox One/Series use
 *     dark physical-style caps with large bold colored A/B/X/Y lettering
 *     and new LB/RB bumpers. The classic DDS atlas is not modified.
 * PT-BR: O DualSense mantem simbolos brancos; Xbox One/Series recebem
 *     letras A/B/X/Y maiores e em negrito, alem de LB/RB modernos.
 *     Os arquivos originais do atlas DDS permanecem intactos.
 */
static bool DrawModernControllerButton(ImDrawList* drawList, EButtonIcon icon,
    const ImVec2& iconMin, const ImVec2& iconMax)
{
    if (Config::ControllerIcons != EControllerIcons::Auto)
        return false;

    const auto visual = ui::controller_icons::AutomaticStyle(hid::g_inputDeviceExplicit);

    const bool shoulder = icon == EButtonIcon::LB || icon == EButtonIcon::RB;
    if (shoulder && ui::controller_icons::UsesModernShoulderButtons(visual))
    {
        if (!g_fntNewRodin)
            return false;
        DrawXboxSeriesShoulder(drawList, icon, iconMin, iconMax);
        return true;
    }

    const bool face = icon == EButtonIcon::A || icon == EButtonIcon::B ||
        icon == EButtonIcon::X || icon == EButtonIcon::Y;
    if (!face || !ui::controller_icons::UsesModernFaceButtons(visual))
        return false;

    // EN: If the font is unavailable, retain the original Xbox atlas
    //     instead of displaying unlabeled black circles.
    // PT-BR: Sem fonte, retorna ao DDS original em vez de botoes sem letras.
    if (visual == ui::controller_icons::Style::XboxSeries && !g_fntNewRodin)
        return false;

    const float width = iconMax.x - iconMin.x;
    const float height = iconMax.y - iconMin.y;
    const float radius = std::min(width, height) * 0.46f;
    if (radius <= 0.0f)
        return false;

    const ImVec2 center{ (iconMin.x + iconMax.x) * 0.5f,
        (iconMin.y + iconMax.y) * 0.5f };
    const ImU32 background = IM_COL32(25, 27, 33, 255);
    const ImU32 outline = IM_COL32(148, 153, 167, 200);
    drawList->AddCircleFilled(
        { center.x, center.y + radius * 0.11f }, radius,
        IM_COL32(5, 6, 8, 235), 40);
    drawList->AddCircleFilled(center, radius, background, 40);
    drawList->AddCircle(center, radius - 0.75f, outline, 40,
        std::max(1.0f, radius * 0.065f));

    if (visual == ui::controller_icons::Style::DualSense)
    {
        // EN: SDL's A/B/X/Y correspond to Cross/Circle/Square/Triangle.
        // PT-BR: A/B/X/Y do SDL equivalem a Cruz/Circulo/Quadrado/Triangulo.
        const ImU32 white = IM_COL32(246, 246, 250, 255);
        const float size = radius * 0.49f;
        const float thickness = std::max(2.0f, radius * 0.145f);
        switch (icon)
        {
        case EButtonIcon::A:
            drawList->AddLine({ center.x - size, center.y - size },
                { center.x + size, center.y + size }, white, thickness);
            drawList->AddLine({ center.x + size, center.y - size },
                { center.x - size, center.y + size }, white, thickness);
            break;
        case EButtonIcon::B:
            drawList->AddCircle(center, size, white, 36, thickness);
            break;
        case EButtonIcon::X:
            drawList->AddRect({ center.x - size, center.y - size },
                { center.x + size, center.y + size }, white, 0.0f, 0, thickness);
            break;
        case EButtonIcon::Y:
            drawList->AddTriangle({ center.x, center.y - size * 1.17f },
                { center.x - size * 1.12f, center.y + size * 0.86f },
                { center.x + size * 1.12f, center.y + size * 0.86f },
                white, thickness);
            break;
        default:
            break;
        }
    }
    else
    {
        // EN: Large *typographic* Xbox letters; the blue bold 'X' is no
        //     longer a pair of thin white lines like a PlayStation cross.
        // PT-BR: O X azul agora e uma letra grande e em negrito, nao duas
        //     linhas brancas finas como a cruz do PlayStation.
        const char* label = nullptr;
        ImU32 color = IM_COL32_WHITE;
        switch (icon)
        {
        case EButtonIcon::A:
            label = "A"; color = IM_COL32(91, 214, 98, 255); break;
        case EButtonIcon::B:
            label = "B"; color = IM_COL32(246, 99, 105, 255); break;
        case EButtonIcon::X:
            label = "X"; color = IM_COL32(99, 166, 253, 255); break;
        case EButtonIcon::Y:
            label = "Y"; color = IM_COL32(255, 210, 78, 255); break;
        default:
            break;
        }
        if (label)
            DrawBoldXboxLabel(drawList, label, center, radius * 1.48f, color);
    }
    return true;
}

/**
 * EN: Keep the existing image atlas for every other button or for
 *     explicitly forced Xbox/PlayStation prompts.
 * PT-BR: Preserva os icones existentes para os demais botoes e para
 *     selecoes manuais de Xbox ou PlayStation.
 */
static void DrawControllerIcon(ImDrawList* drawList, EButtonIcon icon,
    const ImVec2& iconMin, const ImVec2& iconMax)
{
    if (DrawModernControllerButton(drawList, icon, iconMin, iconMax))
        return;

    const auto buttonIcon = GetButtonIcon(icon);
    drawList->AddImage(std::get<1>(buttonIcon), iconMin, iconMax,
        GET_UV_COORDS(std::get<0>(buttonIcon)));
}

static void DrawGuide(float* offset, ImVec2 regionMin, ImVec2 regionMax, EButtonIcon icon,
    EButtonAlignment alignment, ImVec2 iconMin, ImVec2 iconMax, EFontQuality fontQuality,
    float textWidth, float maxTextWidth, float textScale, float fontSize, const char* text)
{
    auto drawList = ImGui::GetBackgroundDrawList();
    auto iconWidth = Scale(g_iconWidths[icon]);
    auto textMarginY = regionMin.y + Scale(9.0f);

    ImVec2 textPos;

    if (icon == EButtonIcon::LBRB || icon == EButtonIcon::LTRT)
    {
        auto iconMarginX = Scale(16);

        iconMin.x = alignment == EButtonAlignment::Left
            ? regionMin.x + *offset - maxTextWidth + (iconWidth / 2)
            : regionMax.x - *offset - maxTextWidth - (iconWidth / 2) - iconMarginX;

        iconMax.x = iconMin.x + iconWidth;

        // Left button.
        DrawControllerIcon(drawList, icon == EButtonIcon::LBRB ? EButtonIcon::LB : EButtonIcon::LT,
            iconMin, iconMax);

        auto dualIconMin = alignment == EButtonAlignment::Left
            ? ImVec2(iconMax.x + maxTextWidth + iconMarginX, iconMin.y)
            : ImVec2(regionMax.x - *offset + maxTextWidth - iconWidth, iconMin.y);

        auto dualIconMax = ImVec2(dualIconMin.x + iconWidth, iconMax.y);

        // Right button.
        DrawControllerIcon(drawList, icon == EButtonIcon::LBRB ? EButtonIcon::RB : EButtonIcon::RT,
            dualIconMin, dualIconMax);

        textPos = { (iconMax.x + ((dualIconMin.x - iconMax.x) - maxTextWidth + std::max(0.0f, maxTextWidth - textWidth)) / 2) + Scale(2), textMarginY};

        *offset += iconWidth;
    }
    else
    {
        DrawControllerIcon(drawList, icon, iconMin, iconMax);

        auto textMarginX = alignment == EButtonAlignment::Left
            ? regionMin.x + *offset
            : regionMax.x - *offset;

        textPos = { textMarginX, textMarginY };
    }

    SetScale({ textScale, 1.0f });
    SetOrigin(textPos);

    if (fontQuality == EFontQuality::Low)
        SetShaderModifier(IMGUI_SHADER_MODIFIER_LOW_QUALITY_TEXT);

    DrawTextWithOutline(g_fntNewRodin, fontSize, textPos, IM_COL32_WHITE, text, 4, IM_COL32_BLACK);

    if (fontQuality == EFontQuality::Low)
    {
        // Add extra luminance to low quality text.
        drawList->AddText(g_fntNewRodin, fontSize, textPos, IM_COL32(255, 255, 255, 127), text);
        SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);
    }

    SetScale({ 1.0f, 1.0f });
    SetOrigin({ 0.0f, 0.0f });
}

void ButtonGuide::Init()
{
    auto& io = ImGui::GetIO();

    g_fntNewRodin = ImFontAtlasSnapshot::GetFont("FOT-NewRodinPro-M.otf");
    g_upControllerIcons = LOAD_ZSTD_TEXTURE(g_controller);
    g_upKBMIcons = LOAD_ZSTD_TEXTURE(g_kbm);
}

void ButtonGuide::Draw()
{
    if (!s_isVisible)
        return;

    auto drawList = ImGui::GetBackgroundDrawList();
    auto& res = ImGui::GetIO().DisplaySize;

    ImVec2 regionMin = { g_aspectRatioOffsetX + Scale(g_sideMargins), g_aspectRatioOffsetY * 2.0f + Scale(720.0f - 102.0f) };
    ImVec2 regionMax = { g_aspectRatioOffsetX + Scale(1280.0f - g_sideMargins), g_aspectRatioOffsetY * 2.0f + Scale(720.0f) };

    auto textMarginX = Scale(21.25f);
    auto iconMarginX = Scale(4);
    auto fontSize = Scale(21.8f);

    auto offsetLeft = 0.0f;
    auto offsetRight = 0.0f;

    // Draw left aligned icons.
    for (int i = 0; i < g_buttons.size(); i++)
    {
        auto& btn = g_buttons[i];

        if (btn.Alignment != EButtonAlignment::Left)
            continue;

        if (btn.Visibility && !*btn.Visibility)
            continue;

        auto str = Localise(btn.Name).c_str();
        auto iconWidth = Scale(g_iconWidths[btn.Icon]);
        auto iconHeight = Scale(g_iconHeights[btn.Icon]);
        auto textWidth = g_fntNewRodin->CalcTextSizeA(fontSize, FLT_MAX, 0, str).x;
        auto maxWidth = btn.MaxWidth == FLT_MAX ? textWidth : Scale(btn.MaxWidth);
        auto textScale = std::min(1.0f, maxWidth / textWidth);

        if (i > 0)
            offsetLeft += maxWidth + iconWidth + textMarginX;

        ImVec2 iconMin = { regionMin.x + offsetLeft - iconWidth - iconMarginX, regionMin.y };
        ImVec2 iconMax = { regionMin.x + offsetLeft - iconMarginX, regionMin.y + iconHeight };

        DrawGuide(&offsetLeft, regionMin, regionMax, btn.Icon, btn.Alignment, iconMin, iconMax, btn.FontQuality, textWidth, maxWidth, textScale, fontSize, str);
    }

    // Draw right aligned icons.
    for (int i = g_buttons.size() - 1; i >= 0; i--)
    {
        auto& btn = g_buttons[i];

        if (btn.Alignment != EButtonAlignment::Right)
            continue;

        if (btn.Visibility && !*btn.Visibility)
            continue;

        auto str = Localise(btn.Name).c_str();
        auto iconWidth = Scale(g_iconWidths[btn.Icon]);
        auto iconHeight = Scale(g_iconHeights[btn.Icon]);
        auto textWidth = g_fntNewRodin->CalcTextSizeA(fontSize, FLT_MAX, 0, str).x;
        auto maxWidth = btn.MaxWidth == FLT_MAX ? textWidth : Scale(btn.MaxWidth);
        auto textScale = std::min(1.0f, maxWidth / textWidth);

        if (i < g_buttons.size() - 1)
            offsetRight += maxWidth + iconWidth + textMarginX;

        ImVec2 iconMin = { regionMax.x - offsetRight - iconWidth - iconMarginX, regionMin.y };
        ImVec2 iconMax = { regionMax.x - offsetRight - iconMarginX, regionMin.y + iconHeight };

        DrawGuide(&offsetRight, regionMin, regionMax, btn.Icon, btn.Alignment, iconMin, iconMax, btn.FontQuality, textWidth, maxWidth, textScale, fontSize, str);
    }
}

void ButtonGuide::Open(Button button)
{
    s_isVisible = true;
    g_sideMargins = DEFAULT_SIDE_MARGINS;

    g_buttons = {};
    g_buttons.push_back(button);
}

void ButtonGuide::Open(const std::span<Button> buttons)
{
    s_isVisible = true;
    g_sideMargins = DEFAULT_SIDE_MARGINS;
    g_buttons = std::vector(buttons.begin(), buttons.end());
}

void ButtonGuide::SetSideMargins(float width = DEFAULT_SIDE_MARGINS)
{
    g_sideMargins = width;
}

void ButtonGuide::Close()
{
    s_isVisible = false;
}
