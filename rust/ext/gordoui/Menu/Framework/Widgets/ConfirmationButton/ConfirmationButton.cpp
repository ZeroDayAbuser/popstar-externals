#include <Cheat.hpp>

void CConfirmationButton::Render() {
    SetContainedAlpha(Render::alpha);

    Vector2 text_size = Render::CalcTextSize(UI::WrapText(GetName(), GetSize().x - (Style::padding * 2.0f)));
    Vector2 size = Vector2{GetParent<CContainer>()->GetSize().x - Style::padding * 2.0f, text_size.y + 8.0f};
    SetSize(size);

    Render::AddShadowRect(
        GetPosition(), GetSize(),
        Style::shadow.Lerp(Style::accentColor, GetClickedAlpha()).ScaleAlpha(0.5f + (0.5f * (GetHoverAlpha() + GetClickedAlpha()))), 15.0f,
        Style::rounding);
    Render::AddRectFilled(GetPosition(), GetSize(), Style::widgetBackground, Style::rounding);
    Render::AddRect(GetPosition(), GetSize(), Style::outline, Style::rounding);
    Render::AddRectFilled(GetPosition(), GetSize(), Style::accentColor.ScaleAlpha(GetClickedAlpha()), Style::rounding);

    Render::AddText(UI::WrapText(GetName(), GetSize().x - (Style::padding * 2.0f)), GetPosition() + GetSize() / 2.0f, Style::text,
                     TEXT_FLAG_NONE, {0.5f, 0.5f});

    bool hovered = Input::IsMouseOverRect(GetPosition(), GetSize()) && !UI::BeingBlocked(GetParent()) && !GetHidden();
    if (hovered && Input::IsMouseClicked())
    {
        SetClickedAlpha(1.0f);
        SetOpened(true);
        UI::SetBlockingObject(this, GetParent());
    }

    if (GetOpened() || GetActiveAlpha() > 0.0f)
    {
        SetTooltipTime(0.0f);
        CWindow *parent_window = UI::GetDesc<CWindow>(GetParent());

        Vector2 conf_size = Render::CalcTextSize(GetConfirmationMessage()) + Vector2{Style::padding * 2.0f, size.y + (Style::padding * 3.0f)};
        Vector2 conf_pos = parent_window->GetPosition() + (parent_window->GetSize() * 0.5f) - (conf_size * 0.5f);

        // yes im forcing this bitch to the top
        Render::SetDrawList(ImGui::GetForegroundDrawList());
        int layer = Render::GetCurrentLayer();
        Render::SetLayer(10);
        float alpha_save = Render::alpha;
        Render::alpha *= GetActiveAlpha();

        // dim menu
        Render::AddRectFilled(parent_window->GetPosition(), parent_window->GetSize(), Color::Black().ScaleAlpha(0.35f),
                                Style::rounding);

        Render::AddShadowRect(
            conf_pos, conf_size,
            Style::shadow,
            15.0f, Style::rounding);
        Render::AddRectFilled(conf_pos, conf_size, Style::container, Style::rounding);
        Render::AddRect(conf_pos, conf_size, Style::outline, Style::rounding);

        Render::AddText(GetConfirmationMessage(), conf_pos + Vector2{Style::padding, Style::padding}, Style::text, TEXT_FLAG_NONE);

        // accept
        {
            Vector2 pos = conf_pos + Vector2{Style::padding, conf_size.y - (size.y + Style::padding)};
            Vector2 s = Vector2{(conf_size.x - (Style::padding * 3.0f)) / 2.0f, size.y};

            bool btn_hovered = Input::IsMouseOverRect(pos, s) && GetOpened();
            if ((btn_hovered && Input::IsMouseClicked()) || (Input::IsKeyPressed(VK_RETURN) && GetOpened()))
            {
                SetAcceptActiveAlpha(1.0f);
                TriggerAcceptCallback();
                SetOpened(false);
                UI::SetBlockingObject(nullptr, GetParent());
            }

            SetAcceptHoverAlpha(
                std::clamp(GetAcceptHoverAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (btn_hovered ? 1.0f : -1.0f)), 0.0f, 1.0f));
            SetAcceptActiveAlpha(std::clamp(GetAcceptActiveAlpha() + (5.0f * ImGui::GetIO().DeltaTime * -1.0f), 0.0f, 1.0f));

            Render::AddShadowRect(
                pos, s,
                Style::shadow.Lerp(Style::accentColor, GetAcceptActiveAlpha()).ScaleAlpha(0.5f + (0.5f * (GetAcceptHoverAlpha() + GetAcceptActiveAlpha()))),
                15.0f, Style::rounding);
            Render::AddRectFilled(pos, s, Style::widgetBackground, Style::rounding);
            Render::AddRect(pos, s, Style::outline, Style::rounding);
            Render::AddRectFilled(pos, s, Style::accentColor.ScaleAlpha(GetAcceptActiveAlpha()), Style::rounding);

            Render::AddText(X("Confirm"), pos + s / 2.0f, Style::text, TEXT_FLAG_NONE, Vector2{0.5f, 0.5f});

        }

        // decline
        {
            Vector2 s = Vector2{(conf_size.x - (Style::padding * 3.0f)) / 2.0f, size.y};
            Vector2 pos = conf_pos + Vector2{conf_size.x - (Style::padding + s.x), conf_size.y - (size.y + Style::padding)};
            bool btn_hovered = Input::IsMouseOverRect(pos, s) && GetOpened();
            if ((btn_hovered && Input::IsMouseClicked()) || (Input::IsKeyPressed(VK_ESCAPE) && GetOpened()))
            {
                SetDeclineActiveAlpha(1.0f);
                TriggerDeclineCallback();
                SetOpened(false);
                UI::SetBlockingObject(nullptr, GetParent());
            }

            SetDeclineHoverAlpha(
                std::clamp(GetDeclineHoverAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (btn_hovered ? 1.0f : -1.0f)), 0.0f, 1.0f));
            SetDeclineActiveAlpha(std::clamp(GetDeclineActiveAlpha() + (5.0f * ImGui::GetIO().DeltaTime * -1.0f), 0.0f, 1.0f));

            Render::AddShadowRect(pos, s,
                                    Style::shadow.Lerp(Style::accentColor, GetDeclineActiveAlpha())
                                        .ScaleAlpha(0.5f + (0.5f * (GetDeclineHoverAlpha() + GetDeclineActiveAlpha()))),
                                    15.0f, Style::rounding);

            Render::AddRectFilled(pos, s, Style::widgetBackground, Style::rounding);
            Render::AddRect(pos, s, Style::outline, Style::rounding);
            Render::AddRectFilled(pos, s, Style::accentColor.ScaleAlpha(GetDeclineActiveAlpha()), Style::rounding);

            Render::AddText(X("Cancel"), pos + s / 2.0f, Style::text, TEXT_FLAG_NONE, Vector2{0.5f, 0.5f});
        }

        Render::alpha = alpha_save;
        Render::SetLayer(layer);
        Render::RevertDrawList();
        
    }

    if (!GetOpened() && GetActiveAlpha() <= 0.0f)
    {
        SetAcceptHoverAlpha(0.0f);
        SetAcceptActiveAlpha(0.0f);

        SetDeclineHoverAlpha(0.0f);
        SetDeclineActiveAlpha(0.0f);
    }

    SetHoverAlpha(std::clamp(GetHoverAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (hovered ? 1.0f : -1.0f)), 0.0f, 1.0f));
    SetClickedAlpha(std::clamp(GetClickedAlpha() + (5.0f * ImGui::GetIO().DeltaTime * -1.0f), 0.0f, 1.0f));
    SetActiveAlpha(std::clamp(GetActiveAlpha() + (5.0f * ImGui::GetIO().DeltaTime * (GetOpened() ? 1.0f : -1.0f)), 0.0f, 1.0f));
}
