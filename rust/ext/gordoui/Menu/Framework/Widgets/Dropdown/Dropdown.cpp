#include <Cheat.hpp>

void CDropdown::Render()
{
    SetContainedAlpha(Render::alpha);

    float rect_height = 22.0f;
    Vector2 text_size = Render::CalcTextSize(GetName());
    Vector2 size = Vector2{GetSize().x, text_size.y + (Style::padding * 0.5f) + rect_height};
    SetSize(size);
    size.y = rect_height;

    Render::AddText(GetName(), GetPosition(), Style::dimmedText.Lerp(Style::text, GetActiveAlpha()), TEXT_FLAG_NONE,
                     {0.0f, 0.0f});

    SetPosition({GetPosition().x, GetPosition().y + text_size.y + (Style::padding * 0.5f)});
    Render::AddShadowRect(GetPosition(), size, Style::shadow.ScaleAlpha(0.5f + (0.5f * GetHoverAlpha())), 15.0f, 3.0f);
    Render::AddRectFilled(GetPosition(), size, Style::widgetBackground, 3.0f);
    Render::AddRect(GetPosition(), size, Style::outline, 3.0f);

    Render::AddText(GetItems().at(*GetValue()), GetPosition() + Vector2{Style::padding, (size.y / 2.0f) - 1.0f}, Style::text, TEXT_FLAG_NONE,
                     {0.0f, 0.5f});

    bool hovered = Input::IsMouseOverRect(GetPosition(), size) && !UI::BeingBlocked(GetParent()) && !GetHidden();
    if (hovered && Input::IsMouseClicked())
    {
        UI::SetBlockingObject(this, GetParent());
        SetOpened(!GetOpened());
        SetTooltipTime(0.0f);
    }

    if (GetOpened() || GetActiveAlpha() > 0.0f)
    {
        SetTooltipTime(0.0f);
        Render::SetDrawList(ImGui::GetForegroundDrawList());
        Render::SetLayer(Render::GetCurrentLayer() + 5);

        Vector2 base_position = GetPosition();
        Vector2 list_size = Vector2{size.x, rect_height + std::roundf((rect_height * static_cast<float>(GetItems().size())) * GetActiveAlpha())};

        bool list_hovered = Input::IsMouseOverRect(base_position + Vector2{0.0f, rect_height}, list_size - Vector2{0.0f, rect_height});
        if (!list_hovered && Input::IsMouseClicked())
        {
            UI::SetBlockingObject(nullptr, GetParent());
            SetOpened(false);
        }

        Render::AddShadowRect(GetPosition(), list_size, Style::shadow.ScaleAlpha(0.5f + (0.5f * GetActiveAlpha())), 15.0f, 3.0f);
        Render::AddRectFilled(GetPosition(), list_size, Style::widgetBackground, 3.0f);
        Render::AddRect(GetPosition(), list_size, Style::outline, 3.0f);

        Render::AddText(GetItems().at(*GetValue()), GetPosition() + Vector2{Style::padding, (size.y / 2.0f) - 1.0f}, Style::text, TEXT_FLAG_NONE,
                         {0.0f, 0.5f});

        for (std::size_t i = 0; i < GetItems().size(); ++i)
        {
            Vector2 item_position = base_position + Vector2{0.0f, rect_height + (static_cast<float>(i) * (rect_height * GetActiveAlpha()))};
            Vector2 item_size = Vector2{list_size.x, rect_height * GetActiveAlpha()};
            bool item_hovered = Input::IsMouseOverRect(item_position, item_size) && GetOpened();

            SetAnimItemHoverAt(i, std::clamp(GetAnimItemAtHover(i) + (10.0f * ImGui::GetIO().DeltaTime * (item_hovered ? 1.0f : -1.0f)), 0.0f, 1.0f));
            SetAnimItemActiveAt(
                i, std::clamp(GetAnimItemAtActive(i) + (10.0f * ImGui::GetIO().DeltaTime * ((*GetValue() == i) ? 1.0f : -1.0f)), 0.0f, 1.0f));

            if (item_hovered && Input::IsMouseClicked())
            {
                UI::SetBlockingObject(nullptr, GetParent());
                *GetValue() = static_cast<int>(i);
                SetOpened(false);
            }

            Render::AddText(GetItems().at(i),
                             item_position + Vector2{Style::padding + (Style::padding * GetAnimItemAtHover(i)), rect_height / 2.0f - 1.0f},
                             Style::dimmedText.Lerp(Style::text, GetAnimItemAtHover(i)).Lerp(Style::accentColor, GetAnimItemAtActive(i)).ScaleAlpha(GetActiveAlpha()), TEXT_FLAG_NONE, {0.0f, 0.5f});
        }

        Render::SetLayer(Render::GetCurrentLayer() - 5);
        Render::RevertDrawList();
    }

    SetHoverAlpha(std::clamp(GetHoverAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (hovered ? 1.0f : -1.0f)), 0.0f, 1.0f));
    SetActiveAlpha(std::clamp(GetActiveAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (GetOpened() ? 1.0f : -1.0f)), 0.0f, 1.0f));
}
