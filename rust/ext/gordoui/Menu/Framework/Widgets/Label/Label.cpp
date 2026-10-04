#include <Cheat.hpp>

void CLabel::Render()
{
    SetContainedAlpha(Render::alpha);

    std::string label_text = UI::WrapText(GetName(), GetSize().x - (Style::padding * 2.0f));
    Vector2 text_size = Render::CalcTextSize(label_text);
    SetSize(Vector2{GetSize().x, text_size.y});

    Render::AddText(label_text, GetPosition(), GetUseCustomColor() ? GetCustomColor() : Style::text, TEXT_FLAG_NONE);

    Vector2 base_size = Vector2{text_size.y, text_size.y};
    Vector2 base_position = GetPosition() + Vector2{GetSize().x - base_size.x, 0.0f};

    for (auto c : GetChildren())
    {
        CColorPicker *cp = dynamic_cast<CColorPicker *>(c);
        if (cp)
        {
            Render::AddShadowRect(base_position, base_size,
                                    Style::shadow.Lerp(cp->GetValue()->OverrideAlpha(1.0f * Render::alpha), cp->GetOpenAlpha())
                                        .ScaleAlpha(0.5f + (0.5f * cp->GetOpenAlpha())),
                                    15.0f, Style::rounding);
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
                             TEXT_FLAG_NONE, {0.5f, 0.5f}, Render::Fonts::fontAwesome14px);

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
