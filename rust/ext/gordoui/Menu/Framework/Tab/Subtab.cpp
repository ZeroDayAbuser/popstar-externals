#include <Cheat.hpp>

void CSubtab::Render()
{
    if (!this->ShouldShow() && this->GetSubtabAlpha() <= 0.0f)
        return;

    Vector2 nextPosition = GetPosition() + Vector2{0.0f, Style::headerHeight};
    Vector2 nextSize = GetSize();

    float alpha = Render::alpha;
    Render::alpha *= this->GetSubtabAlpha();

    float childWidth = (GetSize().x - Style::padding) / 2.0f;
    float columnYOffset[2] = {0.0f, 0.0f};

    int i = 0;
    for (auto &c : GetChildren())
    {
        if (auto st = dynamic_cast<CSubtab *>(c); st != nullptr)
        {
            st->SetPosition(GetPosition());
            st->SetSize(GetSize());
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

    Render::alpha = alpha;
}
