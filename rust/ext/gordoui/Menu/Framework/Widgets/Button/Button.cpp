#include <Cheat.hpp>

void CButton::Render() {
    SetContainedAlpha(Render::alpha);

    Vector2 text_size = Render::CalcTextSize(UI::WrapText(GetName(), GetSize().x - (Style::padding * 2.0f)));
    Vector2 size = Vector2{GetParent<CContainer>()->GetSize().x - Style::padding * 2.0f, text_size.y + 8.0f};
    SetSize(size);

    Render::AddShadowRect(GetPosition(), GetSize(), Style::shadow.Lerp(Style::accentColor, GetClickedAlpha()).ScaleAlpha(0.5f + (0.5f * (GetHoverAlpha() + GetClickedAlpha()))),
                            15.0f, Style::rounding);
    Render::AddRectFilled(GetPosition(), GetSize(), Style::widgetBackground,
                            Style::rounding);
    Render::AddRect(GetPosition(), GetSize(), Style::outline, Style::rounding);
    Render::AddRectFilled(GetPosition(), GetSize(), Style::accentColor.ScaleAlpha(GetClickedAlpha()), Style::rounding);

    Render::AddText(UI::WrapText(GetName(), GetSize().x - (Style::padding * 2.0f)), GetPosition() + GetSize() / 2.0f, Style::text, TEXT_FLAG_NONE, {0.5f, 0.5f});

    bool hovered = Input::IsMouseOverRect(GetPosition(), GetSize()) && !UI::BeingBlocked(GetParent()) && !GetHidden();
    if (hovered && Input::IsMouseClicked())
    {
        TriggerCallback();
        SetTooltipTime(0.0f);
        SetClickedAlpha(1.0f);
    }

    SetHoverAlpha(std::clamp(GetHoverAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (hovered ? 1.0f : -1.0f)), 0.0f, 1.0f));
    SetClickedAlpha(std::clamp(GetClickedAlpha() + (5.0f * ImGui::GetIO().DeltaTime * -1.0f), 0.0f, 1.0f));
}
