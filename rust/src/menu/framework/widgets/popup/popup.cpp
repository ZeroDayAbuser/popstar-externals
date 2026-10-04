#include <Cheat.hpp>

void CPopup::Render()
{
    SetOpenAlpha(std::clamp(GetOpenAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (GetOpened() ? 1.0f : -1.0f)), 0.0f, 1.0f));
    if (!GetOpened() && GetOpenAlpha() <= 0.0f)
        return;

    auto base = UI::GetDesc<CBaseWindow>(this);
    if (GetOpened() && base)
    {
        if (base->GetBlockingObject() != this)
            base->SetBlockingObject(this);
    }

    if (base)
    {
        Vector2 popSize = GetSize();
        if (popSize.x < 1.0f) popSize.x = GetOverrideSize() ? GetSize().x : 250.0f;
        if (popSize.y < 1.0f) popSize.y = 40.0f;

        const Vector2 basePos  = base->GetPosition();
        const Vector2 baseSize = base->GetSize();
        const float pad = Style::padding;

        float minX = basePos.x + pad;
        float maxX = basePos.x + baseSize.x - popSize.x - pad;
        float minY = basePos.y + pad;
        float maxY = basePos.y + baseSize.y - popSize.y - pad;
        if (maxX < minX) maxX = minX;
        if (maxY < minY) maxY = minY;

        Vector2 pos = GetPosition();
        pos.x = ImClamp(pos.x, minX, maxX);
        pos.y = ImClamp(pos.y, minY, maxY);
        SetPosition(pos);
    }

    Render::SetDrawList(ImGui::GetForegroundDrawList());
    Render::SetLayer(Render::GetCurrentLayer() + 5);

    Render::AddShadowRect(GetPosition(), GetSize(), Style::shadow, 15.0f, Style::rounding);
    Render::AddRectFilled(GetPosition(), GetSize(), Style::container, Style::rounding);
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
    if (!hovered && Input::IsMouseClicked() && (!popup_asc || !popup_asc->GetParent()))
    {
        UI::SetBlockingObject(nullptr, GetParent());
        SetOpened(false);
    }

    Render::SetLayer(Render::GetCurrentLayer() - 5);
    Render::RevertDrawList();
}
