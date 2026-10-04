#include <Cheat.hpp>

void CObject::RenderToolTip()
{
    if (GetTooltip().empty())
        return;

    bool hovered = Input::IsMouseOverRect(GetPosition(), GetSize()) && !UI::BeingBlocked(GetParent()) && GetContainedAlpha() > 0.0f;
    if (!hovered && GetTooltipAlpha() <= 0.0f)
    {
        SetTooltipTime(0.0f);
        SetTooltipAlpha(std::clamp(GetTooltipAlpha() - (10.0f * ImGui::GetIO().DeltaTime), 0.0f, 1.0f));
        return;
    }

    if (hovered)
        SetTooltipTime(GetTooltipTime() + 1.0f * ImGui::GetIO().DeltaTime);
    else
        SetTooltipTime(0.0f);

    const float max_width = 300.0f;
    std::string processed_text = UI::WrapText(GetTooltip(), max_width);

    Vector2 text_size = Render::CalcTextSize(processed_text);
    Vector2 size = text_size + Vector2{Style::padding * 2.0f, Style::padding * 2.0f};

    bool should_show_tooltip = GetAlpha() > 0.0f && GetTooltipTime() >= 1.0f;
    if (GetTooltipTime() < 1.0f && GetTooltipAlpha() <= 0.0f)
        SetTooltipPosition(UI::ClampToScreen(ImGui::GetIO().MousePos - Vector2{size.x / 2.0f, size.y + Style::padding}, size));

    SetTooltipAlpha(std::clamp(GetTooltipAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (should_show_tooltip ? 1.0f : -1.0f)), 0.0f, 1.0f));

    if (GetTooltipAlpha() <= 0.0f)
        return;

    Render::SetDrawList(ImGui::GetForegroundDrawList());
    Render::SetLayer(Render::GetCurrentLayer() + 5);

    Render::AddShadowRect(GetTooltipPosition(), size, Style::shadow.ScaleAlpha(GetTooltipAlpha()), 15.0f, Style::rounding);
    Render::AddRectFilled(GetTooltipPosition(), size, Style::widgetBackground.ScaleAlpha(GetTooltipAlpha()), Style::rounding);

    Render::AddText(processed_text, GetTooltipPosition() + Vector2{Style::padding, Style::padding}, Style::text.ScaleAlpha(GetTooltipAlpha()),
                     TEXT_FLAG_NONE, {0.0f, 0.0f});

    Render::SetLayer(Render::GetCurrentLayer() - 5);
    Render::RevertDrawList();
}

void CObject::UpdateState()
{
    SetHidden(!GetVisibleCondition());
    SetAlpha(std::clamp(GetAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (GetHidden() ? -1.0f : 1.0f)), 0.0f, 1.0f));
}
