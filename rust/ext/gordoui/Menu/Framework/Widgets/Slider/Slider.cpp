#include <Cheat.hpp>

void CSliderFloat::Render()
{
    SetContainedAlpha(Render::alpha);

    float rect_height = 6.0f;
    Vector2 text_size = Render::CalcTextSize(GetName());
    Vector2 size = Vector2{GetSize().x, text_size.y + (Style::padding * 0.5f) + rect_height};
    SetSize(size);
    size.y = rect_height;

    Render::AddText(GetName(), GetPosition(), Style::dimmedText.Lerp(Style::text, GetActiveAlpha()), TEXT_FLAG_NONE, {0.0f, 0.0f});

    const float hovered_relative = (ImGui::GetIO().MousePos.x - GetPosition().x) / size.x;
    float hovered_value = GetMin() + std::clamp(hovered_relative, 0.0f, 1.0f) * (GetMax() - GetMin());

    std::string value_fmt = std::vformat(GetFormat().empty() ? X("{:.1f}") : GetFormat().data(), std::make_format_args(*GetValue()));
    std::string hovered_value_fmt = std::vformat(GetFormat().empty() ? X("{:.1f}") : GetFormat().data(), std::make_format_args(hovered_value));
    Vector2 value_fmt_size = Render::CalcTextSize(value_fmt);
    Vector2 hovered_fmt_size = Render::CalcTextSize(hovered_value_fmt);

    Render::AddText(hovered_value_fmt, GetPosition() + Vector2{size.x - ((value_fmt_size.x + (Style::padding * 0.5f)) * GetHoverAlpha()), 0.0f},
                     Style::text.ScaleAlpha(GetHoverAlpha()), TEXT_FLAG_NONE, {1.0f, 0.0f});

    Render::AddText(value_fmt, GetPosition() + Vector2{size.x, 0.0f}, Style::dimmedText.Lerp(Style::text, GetActiveAlpha()), TEXT_FLAG_NONE,
                     {1.0f, 0.0f});

    SetPosition({GetPosition().x, GetPosition().y + text_size.y + (Style::padding * 0.5f)});
    Render::AddShadowRect(GetPosition(), size, Style::shadow.ScaleAlpha(0.5f + (0.5f * GetHoverAlpha())), 15.0f, 3.0f);
    Render::AddRectFilled(GetPosition(), size, Style::widgetBackground, 3.0f);
    Render::AddRect(GetPosition(), size, Style::outline, 3.0f);

    bool hovered = Input::IsMouseOverRect(GetPosition(), size) && !UI::BeingBlocked(GetParent()) && !GetHidden() && !GetActive();
    if (hovered && Input::IsMouseClicked())
    {
        UI::SetBlockingObject(this, GetParent());
        SetActive(true);
    }

    if (GetActive())
    {
        SetTooltipTime(0.0f);

        if (!Input::IsKeyDown(VK_LBUTTON))
        {
            SetActive(false);
            UI::SetBlockingObject(nullptr, GetParent());
        }

        const float relative = (ImGui::GetIO().MousePos.x - GetPosition().x) / size.x;
        *GetValue() = GetMin() + std::clamp(relative, 0.0f, 1.0f) * (GetMax() - GetMin());
    }

    float slider_fraction_static = std::clamp((*GetValue() - GetMin()) / (GetMax() - GetMin()), 0.0f, 1.0f);
    SetLerpedValue(std::lerp(GetLerpedValue(), slider_fraction_static, 25.0f * ImGui::GetIO().DeltaTime));
    float slider_fraction = GetLerpedValue();

    bool zero_centered = GetMin() < 0.0f && GetMax() > 0.0f;

    if (zero_centered)
    {
        float center_fraction = -GetMin() / (GetMax() - GetMin());
        float zero_fraction = slider_fraction - center_fraction;
        float center_px = GetPosition().x + (GetSize().x * center_fraction);

        float bar_width_px = std::abs(GetSize().x * zero_fraction);
        float pos_x = (zero_fraction >= 0.0f) ? center_px : (center_px - bar_width_px);

        Render::AddRectFilled({pos_x, GetPosition().y}, Vector2{bar_width_px, size.y}, Style::accentColor, Style::rounding);
    }
    else
    {
        Render::AddRectFilled(GetPosition(), Vector2{GetSize().x * slider_fraction, size.y}, Style::accentColor, Style::rounding);
    }

    SetHoverAlpha(std::clamp(GetHoverAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (hovered ? 1.0f : -1.0f)), 0.0f, 1.0f));
    SetActiveAlpha(std::clamp(GetActiveAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (GetActive() ? 1.0f : -1.0f)), 0.0f, 1.0f));
}

void CSliderInt::Render()
{
    SetContainedAlpha(Render::alpha);

    float rect_height = 6.0f;
    Vector2 text_size = Render::CalcTextSize(GetName());
    Vector2 size = Vector2{GetSize().x, text_size.y + (Style::padding * 0.5f) + rect_height};
    SetSize(size);
    size.y = rect_height;

    Render::AddText(GetName(), GetPosition(), Style::dimmedText.Lerp(Style::text, GetActiveAlpha()), TEXT_FLAG_NONE, {0.0f, 0.0f});

    const float hovered_relative = (ImGui::GetIO().MousePos.x - GetPosition().x) / size.x;
    int hovered_value = GetMin() + std::clamp(hovered_relative, 0.0f, 1.0f) * (GetMax() - GetMin());

    std::string value_fmt = std::vformat(GetFormat().empty() ? X("{}") : GetFormat().data(), std::make_format_args(*GetValue()));
    std::string hovered_value_fmt = std::vformat(GetFormat().empty() ? X("{}") : GetFormat().data(), std::make_format_args(hovered_value));
    Vector2 value_fmt_size = Render::CalcTextSize(value_fmt);
    Vector2 hovered_fmt_size = Render::CalcTextSize(hovered_value_fmt);

    Render::AddText(hovered_value_fmt, GetPosition() + Vector2{size.x - ((value_fmt_size.x + (Style::padding * 0.5f)) * GetHoverAlpha()), 0.0f},
                     Style::text.ScaleAlpha(GetHoverAlpha()), TEXT_FLAG_NONE, {1.0f, 0.0f});

    Render::AddText(value_fmt, GetPosition() + Vector2{size.x, 0.0f}, Style::dimmedText.Lerp(Style::text, GetActiveAlpha()), TEXT_FLAG_NONE,
                     {1.0f, 0.0f});

    SetPosition({GetPosition().x, GetPosition().y + text_size.y + (Style::padding * 0.5f)});
    Render::AddShadowRect(GetPosition(), size, Style::shadow.ScaleAlpha(0.5f + (0.5f * GetHoverAlpha())), 15.0f, 3.0f);
    Render::AddRectFilled(GetPosition(), size, Style::widgetBackground, 3.0f);
    Render::AddRect(GetPosition(), size, Style::outline, 3.0f);

    bool hovered = Input::IsMouseOverRect(GetPosition(), size) && !UI::BeingBlocked(GetParent()) && !GetHidden() && !GetActive();
    if (hovered && Input::IsMouseClicked())
    {
        UI::SetBlockingObject(this, GetParent());
        SetActive(true);
    }

    if (GetActive())
    {
        SetTooltipTime(0.0f);

        if (!Input::IsKeyDown(VK_LBUTTON))
        {
            SetActive(false);
            UI::SetBlockingObject(nullptr, GetParent());
        }

        const float relative = (ImGui::GetIO().MousePos.x - GetPosition().x) / size.x;
        *GetValue() = GetMin() + std::clamp(relative, 0.0f, 1.0f) * (GetMax() - GetMin());
    }

    float slider_fraction_static = std::clamp((static_cast<float>(*GetValue()) - static_cast<float>(GetMin())) / static_cast<float>(GetMax() - GetMin()), 0.0f, 1.0f);
    SetLerpedValue(std::lerp(GetLerpedValue(), slider_fraction_static, 25.0f * ImGui::GetIO().DeltaTime));
    float slider_fraction = GetLerpedValue();

    bool zero_centered = GetMin() < 0.0f && GetMax() > 0.0f;

    if (zero_centered)
    {
        float center_fraction = -GetMin() / (GetMax() - GetMin());
        float zero_fraction = slider_fraction - center_fraction;
        float center_px = GetPosition().x + (GetSize().x * center_fraction);

        float bar_width_px = std::abs(GetSize().x * zero_fraction);
        float pos_x = (zero_fraction >= 0.0f) ? center_px : (center_px - bar_width_px);

        Render::AddRectFilled({pos_x, GetPosition().y}, Vector2{bar_width_px, size.y}, Style::accentColor, Style::rounding);
    }
    else
    {
        Render::AddRectFilled(GetPosition(), Vector2{GetSize().x * slider_fraction, size.y}, Style::accentColor, Style::rounding);
    }

    SetHoverAlpha(std::clamp(GetHoverAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (hovered ? 1.0f : -1.0f)), 0.0f, 1.0f));
    SetActiveAlpha(std::clamp(GetActiveAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (GetActive() ? 1.0f : -1.0f)), 0.0f, 1.0f));
}
