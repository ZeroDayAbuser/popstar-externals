#include <Cheat.hpp>

void CColorPicker::Render()
{
    SetContainedAlpha(Render::alpha);

    SetOpenAlpha(std::clamp(GetOpenAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (GetOpened() ? 1.0f : -1.0f)), 0.0f, 1.0f));
    if (!GetOpened() && GetOpenAlpha() <= 0.0f)
        return;

    float base_size = 250.0f;
    float base_bar_size = 20.0f;
    Vector2 size = Vector2{base_size, base_size + base_bar_size + (Style::padding * 2.0f)};
    if (GetAlphaBar())
        size.y += base_bar_size + Style::padding;

    SetSize(size);

    Vector2 sv_position = GetPosition();
    Vector2 hue_position = GetPosition() + Vector2{Style::padding, base_size + Style::padding};
    Vector2 alpha_position = GetPosition() + Vector2{Style::padding, base_size + base_bar_size + (Style::padding * 2.0f)};
    Vector2 bar_size = Vector2{GetSize().x - (Style::padding * 2.0f), base_bar_size};
    Vector2 sv_size = Vector2{base_size, base_size};

    float h, s, v;
    ImGui::ColorConvertRGBtoHSV(GetValue()->r / 255.0f, GetValue()->g / 255.0f, GetValue()->b / 255.0f, h, s, v);

    bool hovered = Input::IsMouseOverRect(GetPosition(), GetSize());
    if (!hovered && Input::IsMouseClicked())
    {
        SetOpened(false);
        UI::SetBlockingObject(nullptr, GetParent());
    }

    if (GetCtxHash() == 0)
        SetHSV(GetValue()->ToHSV());

    bool main_rect_hovered = Input::IsMouseOverRect(GetPosition(), Vector2{base_size, base_size});
    if (main_rect_hovered && Input::IsMouseClicked())
        SetCtxHash(cp_hs);

    if (GetCtxHash() == cp_hs)
    {
        if (!Input::IsKeyDown(VK_LBUTTON))
            SetCtxHash(0);

        float saturation = std::clamp((ImGui::GetIO().MousePos.x - GetPosition().x) / base_size, 0.0f, 1.0f);
        float value = 1.0f - std::clamp((ImGui::GetIO().MousePos.y - GetPosition().y) / base_size, 0.0f, 1.0f);

        float r, g, b;
        ImGui::ColorConvertHSVtoRGB(GetHSV().h, saturation, value, r, g, b);
        *GetValue() = Color(static_cast<int>(r * 255), static_cast<int>(g * 255), static_cast<int>(b * 255), GetValue()->a);
    }

    bool hue_rect_hovered = Input::IsMouseOverRect(hue_position, bar_size);
    if (hue_rect_hovered && Input::IsMouseClicked())
        SetCtxHash(cp_hue);

    if (GetCtxHash() == cp_hue)
    {
        if (!Input::IsKeyDown(VK_LBUTTON))
            SetCtxHash(0);

        float hue = std::clamp((ImGui::GetIO().MousePos.x - hue_position.x) / bar_size.x, 0.0f, 0.9899f);
        float r, g, b;
        ImGui::ColorConvertHSVtoRGB(hue, GetHSV().s, GetHSV().v, r, g, b);
        *GetValue() = Color(static_cast<int>(r * 255), static_cast<int>(g * 255), static_cast<int>(b * 255), GetValue()->a);
    }

    bool alpha_bar_hovered = Input::IsMouseOverRect(alpha_position, bar_size) && GetAlphaBar();
    if (alpha_bar_hovered && Input::IsMouseClicked())
        SetCtxHash(cp_alpha);

    if (GetCtxHash() == cp_alpha)
    {
        if (!Input::IsKeyDown(VK_LBUTTON))
            SetCtxHash(0);

        float alpha = std::clamp((ImGui::GetIO().MousePos.x - alpha_position.x) / bar_size.x, 0.0f, 1.0f);
        GetValue()->a = static_cast<int>(std::roundf(alpha * 255.0f));
    }

    Render::SetDrawList(ImGui::GetForegroundDrawList());
    Render::SetLayer(Render::GetCurrentLayer() + 1);

    Render::AddShadowRect(GetPosition(), GetSize(), Style::shadow, 15.0f, Style::rounding);
    Render::AddRectFilled(GetPosition(), GetSize(), Style::widgetBackground, Style::rounding);
    Render::AddRect(GetPosition(), GetSize(), Style::outline, Style::rounding);

    {
        float r, g, b;
        ImGui::ColorConvertHSVtoRGB(h, 1.0f, 1.0f, r, g, b);
        Color display_color = Color(static_cast<int>(r * 255.0f), static_cast<int>(g * 255.0f), static_cast<int>(b * 255.0f));
        Render::AddRectGradient(sv_position, sv_size, horizontal, Color::White(), display_color, Style::rounding, ImDrawFlags_RoundCornersTop);
        Render::AddRectGradient(sv_position, sv_size, vertical, Color::Black().ScaleAlpha(0.0f), Color::Black(), Style::rounding,
                                  ImDrawFlags_RoundCornersTop);
        Vector2 ballpoint_position = GetPosition() + Vector2{base_size * s, base_size * (1.0f - v)};
        Render::AddShadowCircle(ballpoint_position, 4.0f, Style::shadow);
        Render::AddCircleFilled(ballpoint_position, 4.0f, Style::background);
        Render::AddCircleFilled(ballpoint_position, 3.0f, Color::White());
    }

    {
        float h, s, v;
        ImGui::ColorConvertRGBtoHSV(GetValue()->r / 255.0f, GetValue()->g / 255.0f, GetValue()->b / 255.0f, h, s, v);

        const Color col_hues[6 + 1] = {Color::Red(), Color::Yellow(), Color::Green(), Color::Cyan(), Color::Blue(), Color::Pink(), Color::Red()};
        float segment_width = bar_size.x / 6.0f;

        Render::AddShadowRect(hue_position, bar_size, Style::shadow, 15.0f, Style::rounding);

        for (int i = 0; i < 6; i++)
        {
            int rounding_flags = 0;
            if (i == 0)
            {
                rounding_flags = ImDrawFlags_RoundCornersLeft;
            }
            else if (i == 5)
            {
                rounding_flags = ImDrawFlags_RoundCornersRight;
            }

            Render::GradientItems(
                Vector2{hue_position.x + (i * segment_width), hue_position.y}, Vector2{segment_width, bar_size.y}, col_hues[i], col_hues[i + 1],
                [&, i, rounding_flags]()
                {
                    Render::AddRectFilled(
                        Vector2{hue_position.x + (i * segment_width), hue_position.y}, Vector2{segment_width, bar_size.y}, Color(255, 255, 255),
                        rounding_flags == 0 ? 0.0f : Style::rounding,
                        rounding_flags); // i == 0 ? globals.widget_rounding : i == 5 ? globals.widget_rounding : 0.0f, rounding_flags
                },
                0.0f);
        }

        Vector2 grab_size = Vector2{4.0f, bar_size.y - 4.0f};
        Vector2 grab_position = hue_position + Vector2{2.0f + (bar_size.x * h), 2.0f};

        grab_position.x -= grab_size.x * 0.5f;
        grab_position.x = std::clamp(grab_position.x, hue_position.x + 2.0f, hue_position.x + bar_size.x - 2.0f - grab_size.x - (grab_size.x * 0.5f));

        Render::AddShadowRect(grab_position, grab_size, Color::Black(), 15.0f, grab_size.y);
        Render::AddRectFilled(grab_position, grab_size, Color::Black(), grab_size.y);
        Render::AddRectFilled(grab_position + Vector2{1.0f, 1.0f}, grab_size - Vector2{2.0f, 2.0f}, Color::White(), grab_size.y);
    }

    if (GetAlphaBar())
    {
        Render::AddShadowRect(alpha_position, bar_size, Style::shadow, 15.0f, Style::rounding);
        Render::GradientItems(alpha_position, bar_size, Color::Black(), GetValue()->OverrideAlpha(Render::alpha),
                               [&] { Render::AddRectFilled(alpha_position, bar_size, Color::White(), Style::rounding); });

        Vector2 grab_size = Vector2{4.0f, bar_size.y - 4.0f};
        Vector2 grab_position = alpha_position + Vector2{2.0f + (bar_size.x * (GetValue()->a / 255.0f)), 2.0f};

        grab_position.x -= grab_size.x * 0.5f;
        grab_position.x =
            std::clamp(grab_position.x, alpha_position.x + 2.0f, alpha_position.x + bar_size.x - 2.0f - grab_size.x - (grab_size.x * 0.5f));

        Render::AddShadowRect(grab_position, grab_size, Color::Black(), 15.0f, grab_size.y);
        Render::AddRectFilled(grab_position, grab_size, Color::Black(), grab_size.y);
        Render::AddRectFilled(grab_position + Vector2{1.0f, 1.0f}, grab_size - Vector2{2.0f, 2.0f}, Color::White(), grab_size.y);
    }

    Render::SetLayer(Render::GetCurrentLayer() - 1);
    Render::RevertDrawList();
}
