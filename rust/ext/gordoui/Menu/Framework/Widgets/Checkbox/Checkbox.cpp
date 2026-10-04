#include <Cheat.hpp>

void CCheckbox::Render()
{
    SetContainedAlpha(Render::alpha);

    float base_height = 17.0f;
    Vector2 text_size = Render::CalcTextSize(GetName());
    Vector2 size = Vector2{base_height + Style::padding + text_size.x, base_height};
    SetSize(size);
    size.x = size.y;

    Render::AddShadowRect(
        GetPosition(), size,
        Style::shadow.Lerp(Style::accentColor, GetActiveAlpha()).ScaleAlpha(0.5f + (0.5f * (GetHoverAlpha() + GetActiveAlpha()))), 15.0f,
        3.0f);
    Render::AddRectFilled(GetPosition(), size, Style::widgetBackground, 3.0f);
    Render::AddRect(GetPosition(), size, Style::outline, 3.0f);
    Render::AddRectFilled(GetPosition(), size, Style::accentColor.ScaleAlpha(GetActiveAlpha()), 3.0f);
    Render::RenderCheckmark(GetPosition() + (size / 4.0f) + ((size.y / 4.0f) * (1.0f - GetActiveAlpha())),
                             Style::checkmark.ScaleAlpha(GetActiveAlpha()), (size.y / 2.0f) * GetActiveAlpha(), GetActiveAlpha());

    Render::AddText(GetName(), GetPosition() + Vector2{size.x + Style::padding, GetSize().y / 2.0f},
                     Style::dimmedText.Lerp(Style::text, GetActiveAlpha()), TEXT_FLAG_NONE, {0.0f, 0.5f});

    bool hovered = Input::IsMouseOverRect(GetPosition(), GetSize()) && !UI::BeingBlocked(GetParent()) && !GetHidden();
    if (hovered && Input::IsMouseClicked())
    {
        *GetValue() = !*GetValue();
        SetTooltipTime(0.0f);
    }

    SetHoverAlpha(std::clamp(GetHoverAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (hovered ? 1.0f : -1.0f)), 0.0f, 1.0f));
    SetActiveAlpha(std::clamp(GetActiveAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (*GetValue() ? 1.0f : -1.0f)), 0.0f, 1.0f));

    Vector2 base_size = Vector2{base_height, base_height};
    Vector2 base_position = GetPosition() + Vector2{GetParent()->GetSize().x - (base_size.x + (Style::padding * 2.0f)), 0.0f};

    for (auto c : GetChildren())
    {
        CColorPicker *cp = dynamic_cast<CColorPicker *>(c);
        if (cp)
        {
            Render::AddShadowRect(base_position, base_size,
                                    Style::shadow.Lerp(cp->GetValue()->OverrideAlpha(1.0f * Render::alpha), cp->GetOpenAlpha())
                                        .ScaleAlpha(0.5f + (0.5f * cp->GetOpenAlpha())),
                                    15.0f,
                                    Style::rounding);
            Render::AddRectFilled(base_position, base_size, cp->GetValue()->OverrideAlpha(1.0f * Render::alpha), Style::rounding);
            bool cp_hovered = Input::IsMouseOverRect(base_position, base_size) && !UI::BeingBlocked(GetParent()) && !GetHidden();
            if (cp_hovered && Input::IsMouseClicked())
            {
                cp->SetPosition(ImGui::GetIO().MousePos);
                cp->SetOpened(true);
                UI::SetBlockingObject(cp, GetParent());
            }

            if (cp_hovered)
            {
                if (ImGui::GetIO().KeyCtrl && (Input::IsKeyDown('C') || ImGui::IsKeyPressed(ImGuiKey_C)))
                    Style::copiedColor = *cp->GetValue();

                if (ImGui::GetIO().KeyCtrl && (Input::IsKeyDown('V') || ImGui::IsKeyPressed(ImGuiKey_V)))
                    *cp->GetValue() = Style::copiedColor;
            }

            // this is cancer
            float alpha_save = Render::alpha;
            Render::alpha *= cp->GetOpenAlpha();
            cp->Render();
            Render::alpha = alpha_save;

            base_position.x -= base_size.x + Style::padding;
        }

        CPopup *pu = dynamic_cast<CPopup *>(c);
        if (pu)
        {
            Render::AddShadowCircle(base_position + base_size / 2.0f, 7.0f, Style::shadow.ScaleAlpha(0.5f + (0.5f * pu->GetOpenAlpha())), 30.0f);
            Render::AddText(ICON_FA_GEAR, base_position + base_size / 2.0f, Style::dimmedText.Lerp(Style::text, pu->GetOpenAlpha()),
                             TEXT_FLAG_NONE,
                             {0.5f, 0.5f},
                             Render::Fonts::fontAwesome14px);

            bool cp_hovered = Input::IsMouseOverRect(base_position, base_size) && !UI::BeingBlocked(GetParent()) && !GetHidden();
            if (cp_hovered && Input::IsMouseClicked())
            {
                pu->SetPosition(ImGui::GetIO().MousePos);
                pu->SetOpened(true);
                UI::SetBlockingObject(pu, GetParent());
            }

            // this is cancer
            float alpha_save = Render::alpha;
            Render::alpha *= pu->GetOpenAlpha();
            pu->Render();
            Render::alpha = alpha_save;

            base_position.x -= base_size.x + Style::padding;
        }
    }
}
