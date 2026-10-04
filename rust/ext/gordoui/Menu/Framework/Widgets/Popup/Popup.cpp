#include <Cheat.hpp>

void CPopup::Render()
{
    SetOpenAlpha(std::clamp(GetOpenAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (GetOpened() ? 1.0f : -1.0f)), 0.0f, 1.0f));
    if (!GetOpened() && GetOpenAlpha() <= 0.0f)
        return;

    // fixes blocking object with dropdowns / anything inside of popups that require a object change
    auto base = UI::GetDesc<CBaseWindow>(this);
    if (GetOpened() && base)
    {
        if (base->GetBlockingObject() != this)
            base->SetBlockingObject(this);
    }

    Render::SetDrawList(ImGui::GetForegroundDrawList());
    Render::SetLayer(Render::GetCurrentLayer() + 5);

    Render::AddShadowRect(GetPosition(), GetSize(), Style::shadow, 15.0f, Style::rounding);
    Render::AddRectFilled(GetPosition(), GetSize(), Style::container/*.override_alpha(Render::alpha / 0.92f)*/, Style::rounding);
    Render::AddRect(GetPosition(), GetSize(), Style::outline, Style::rounding);

    float offset = Style::padding;
    for (auto c : GetChildren())
    {
        c->SetPosition(GetPosition() + Vector2(Style::padding, offset));
        c->SetSize(GetSize() - Vector2(Style::padding * 2.0f, Style::padding * 2.0f));

        c->Render();

        offset += c->GetSize().y + Style::padding;
    }

    SetSize(Vector2{GetOverrideSize() ? GetSize().x : 250.0f, offset});

    auto popup_asc = UI::GetDesc<CPopup>(this);
    bool hovered = Input::IsMouseOverRect(GetPosition(), GetSize());
    if (!hovered && Input::IsMouseClicked() && !popup_asc->GetParent())
    {
        UI::SetBlockingObject(nullptr, GetParent());
        SetOpened(false);
    }

    Render::SetLayer(Render::GetCurrentLayer() - 5);
    Render::RevertDrawList();
}
