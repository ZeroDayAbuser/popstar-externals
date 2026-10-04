#include <Cheat.hpp>

void CSeperator::Render()
{
    SetContainedAlpha(Render::alpha);

    Vector2 size = Vector2{GetParent<CContainer>()->GetSize().x - Style::padding * 2.0f, Style::padding};
    SetSize(size);

    Vector2 text_size = GetName().empty() ? Vector2{0.0f, 0.0f} : Render::CalcTextSize(GetName());

    //Render::AddRect(GetPosition(), GetSize(), Color::Red());

    float height = 2.0f;
    Vector2 position = GetPosition() + Vector2{text_size.x == 0.0f ? 0.0f : text_size.x + Style::padding, (size.y * 0.5f) - (height * 0.5f)};
    Vector2 bar_size = Vector2{size.x - (text_size.x + Style::padding), height};

    Render::GradientItems(position, bar_size, Style::shadow, Style::shadow.ScaleAlpha(0.0f),
                           [&] { Render::AddShadowRect(position, bar_size, Color::White(), 15.0f, 4.0f); });

    Render::GradientItems(position, bar_size, Style::outline, Style::outline.ScaleAlpha(0.0f),
                           [&] { Render::AddRectFilled(position, bar_size, Color::White()); });

    if (!GetName().empty())
        Render::AddText(GetName(), GetPosition() + Vector2{0.0f, size.y * 0.5f}, Style::text, TEXT_FLAG_NONE, Vector2{0.0f, 0.5f});
    // Render::AddShadowRect(position, Vector2{size.x / 2.0f, height}, Style::shadow, 15.0f, 4.0f);
    // Render::AddRectFilled(position, Vector2{size.x / 2.0f, height}, Style::outline, 4.0f);
}
