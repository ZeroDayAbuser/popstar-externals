#include <Cheat.hpp>

void CSubtab::Render()
{
    if (!this->ShouldShow() && this->GetSubtabAlpha() <= 0.0f)
        return;

    const Vector2 contentOrigin = GetPosition() + Vector2{0.0f, Style::headerHeight};
    const float   availableH    = GetSize().y - Style::headerHeight;

    float alpha = Render::alpha;
    Render::alpha *= this->GetSubtabAlpha();

    float childWidth = (GetSize().x - Style::padding) / 2.0f;
    float columnYOffset[2] = {0.0f, 0.0f};

    const float scroll = std::roundf(this->GetScrollOffsetLerp());

    Render::PushClipRect(contentOrigin, Vector2{GetSize().x, availableH}, true);

    int i = 0;
    for (auto &c : GetChildren())
    {
        if (auto st = dynamic_cast<CSubtab *>(c); st != nullptr)
        {
            st->SetPosition(GetPosition());
            st->SetSize(GetSize());
            continue;
        }

        c->UpdateState();
        if (c->GetHidden() && c->GetAlpha() <= 0.0f)
            continue;

        int col = i % 2;

        Vector2 childPos = contentOrigin;
        childPos.x += col * (childWidth + Style::padding);
        childPos.y += columnYOffset[col] - scroll;

        c->SetPosition(childPos);
        c->SetSize(Vector2{GetChildren().size() == 1 ? GetSize().x : childWidth, c->GetSize().y});

        float alpha_save = Render::alpha;
        Render::alpha *= c->GetAlpha();
        c->Render();
        Render::alpha = alpha_save;

        columnYOffset[col] += c->GetSize().y + Style::padding;
        i++;
    }

    Render::PopClipRect();

    const float totalH    = std::max(columnYOffset[0], columnYOffset[1]);
    const float maxScroll = std::max(0.0f, totalH - availableH);

    if (maxScroll <= 0.0f)
    {
        this->SetScrollOffset(0.0f);
    }
    else
    {
        const bool overBody = Input::IsMouseOverRect(contentOrigin, Vector2{GetSize().x, availableH});
        if (overBody && ImGui::GetIO().MouseWheel != 0.0f && !UI::BeingBlocked(GetParent()))
        {
            const float scrollSpeed = 40.0f;
            const float newOffset = this->GetScrollOffset() - (ImGui::GetIO().MouseWheel * scrollSpeed);
            this->SetScrollOffset(std::clamp(newOffset, 0.0f, maxScroll));
        }
        else
        {
            this->SetScrollOffset(std::clamp(this->GetScrollOffset(), 0.0f, maxScroll));
        }
    }

    this->SetScrollOffsetLerp(std::lerp(
        this->GetScrollOffsetLerp(), this->GetScrollOffset(),
        25.0f * ImGui::GetIO().DeltaTime));

    if (maxScroll > 0.0f)
    {
        const Vector2 trackPos  = GetPosition() + Vector2{GetSize().x - 5.0f, Style::headerHeight};
        const Vector2 trackSize = Vector2{4.0f, availableH - 1.0f};

        Render::AddRectFilled(trackPos, trackSize, Style::background, Style::rounding);

        const float visibleRatio  = std::clamp(availableH / totalH, 0.1f, 1.0f);
        const float thumbHeight   = trackSize.y * visibleRatio;
        const float scrollableArea = trackSize.y - thumbHeight;
        const float scrollPct     = this->GetScrollOffset() / maxScroll;
        const float thumbY        = scrollPct * scrollableArea;

        Render::AddRectFilled(
            trackPos + Vector2{0.0f, thumbY},
            Vector2{trackSize.x, thumbHeight},
            Style::accentColor, Style::rounding);
    }

    Render::alpha = alpha;
}
