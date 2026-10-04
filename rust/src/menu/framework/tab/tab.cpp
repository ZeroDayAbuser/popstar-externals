#include <Cheat.hpp>

void CTab::Render()
{
    if (!this->ShouldShow() && this->GetTabAlpha() <= 0.0f)
    {
        this->SetAnim(1.0f);
        return;
    }

    this->SetAnim(std::lerp(this->GetAnim(), 0.0f, 20.0f * ImGui::GetIO().DeltaTime));
    Vector2 nextPosition = GetPosition() + Vector2{Style::padding, Style::headerHeight + Style::padding};

    float alpha = Render::alpha;
    Render::alpha *= this->GetTabAlpha();

    float childWidth = (GetSize().x - Style::padding) / 2.0f;
    float columnYOffset[2] = {0.0f, 0.0f};

    std::vector<CSubtab *> subtabs;

    int i = 0;
    for (auto &c : GetChildren())
    {
        if (auto st = dynamic_cast<CSubtab *>(c); st != nullptr)
        {
            if (GetSubtabHash() == 0)
                SetSubtabHash(st->GetSubtabHash());

            subtabs.push_back(st);

            st->SetPosition(GetPosition() + Vector2{Style::padding, Style::headerHeight + (Style::padding * 0.5f)});
            st->SetSize(GetSize() - Vector2{0.0f, Style::headerHeight * 1.19f});
            continue;
        }

        int col = i % 2;

        Vector2 childPos = nextPosition;
        childPos.x += col * (childWidth + Style::padding);
        childPos.y += columnYOffset[col];

        c->SetPosition(childPos);
        c->SetSize(Vector2{GetChildren().size() == 1 ? GetSize().x : childWidth, c->GetSize().y});

        c->Render();

        columnYOffset[col] += c->GetSize().y + Style::padding;
        i++;
    }

    float subtabOffset = -Style::padding * 0.5f;
    for (auto& st : subtabs)
    {
        subtabOffset += Render::CalcTextSize(st->GetName()).x + (Style::padding * 1.5f);
    }

    for (auto& st : subtabs)
    {
        Vector2 stPos = nextPosition + Vector2{GetSize().x - subtabOffset, -Style::padding * 0.5f};
        Vector2 stSize = Render::CalcTextSize(st->GetName()) + Vector2{Style::padding, Style::padding};

        bool hovered = Input::IsMouseOverRect(stPos, stSize) && !UI::BeingBlocked(GetParent());
        if (hovered && Input::IsMouseClicked())
        {
            this->SetSubtabHash(st->GetSubtabHash());
        }

        st->SetSubtabAlpha(std::clamp(st->GetSubtabAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (st->ShouldShow() ? 1.0f : -1.0f)), 0.0f, 1.0f));

        Render::AddText(st->GetName(), stPos + stSize / 2.0f, Style::dimmedText.Lerp(Style::text, st->GetSubtabAlpha()), TEXT_FLAG_NONE,
                         Vector2{0.5f, 0.5f});

        if (st->GetSubtabAlpha() > 0.0f)
        {
            if (!st->GetHint().empty())
                Render::AddText(st->GetHint(), GetPosition() + Vector2{Style::padding, (Style::headerHeight * 1.5f) + (Style::padding * 0.25f)},
                                 Style::dimmedText.ScaleAlpha(st->GetSubtabAlpha()), TEXT_FLAG_NONE, Vector2{0.0f, 0.5f});
            
            Vector2 barSize = Vector2{stSize.x * st->GetSubtabAlpha(), 3.0f};
            Vector2 barPos = stPos + Vector2{(stSize.x / 2.0f) * (1.0f - st->GetSubtabAlpha()), stSize.y - barSize.y};

            Render::AddShadowRect(barPos, barSize, Style::accentColor.ScaleAlpha(st->GetSubtabAlpha()), 15.0f, Style::rounding);
            Render::AddRectFilled(barPos, barSize, Style::accentColor.ScaleAlpha(st->GetSubtabAlpha()), Style::rounding);

            st->Render();
        }

        subtabOffset -= stSize.x + (Style::padding / 2.0f);
    }

    for (auto c : GetChildren())
        UI::RenderAllTooltips(c);

    Render::alpha = alpha;
}
