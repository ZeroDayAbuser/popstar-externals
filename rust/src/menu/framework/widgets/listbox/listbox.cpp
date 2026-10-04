#include <Cheat.hpp>

void CListbox::Render()
{
    SetContainedAlpha(Render::alpha);

    std::string name = UI::WrapText(GetName(), GetSize().x - (Style::padding * 2.0f));
    float rect_height = 22.0f;
    Vector2 text_size = Render::CalcTextSize(name);
    Vector2 size = Vector2{GetSize().x, text_size.y + (Style::padding * 0.5f) + (rect_height * GetMaxItems())};

    if (name.empty())
        size.y = (rect_height * GetMaxItems());

    SetSize(size);
    size.y = (rect_height * GetMaxItems());

    if (!name.empty())
    {
        Render::AddText(name, GetPosition(), Style::dimmedText.Lerp(Style::text, GetHoverAlpha()), TEXT_FLAG_NONE, {0.0f, 0.0f});
        SetPosition({GetPosition().x, GetPosition().y + text_size.y + (Style::padding * 0.5f)});
    }

    Render::AddShadowRect(GetPosition(), size, Style::shadow.ScaleAlpha(0.5f + (0.5f * GetHoverAlpha())), 15.0f, 3.0f);
    Render::AddRectFilled(GetPosition(), size, Style::widgetBackground, 3.0f);
    Render::AddRect(GetPosition(), size, Style::outline, 3.0f);

    bool hovered = Input::IsMouseOverRect(GetPosition(), size) && !UI::BeingBlocked(GetParent()) && !GetHidden();

    Render::PushClipRect(GetPosition() + Vector2{0.0f, 1.0f}, size - Vector2{0.0f, 2.0f}, true);

    std::vector<std::string> filtered_items;
    for (auto t : GetItems())
    {
        if (!UI::ToLower(t).contains(UI::ToLower(GetFilter())))
            continue;

        filtered_items.push_back(t);
    }

    float offset = 0.0f;
    for (std::size_t i = 0; i < GetItems().size(); ++i)
    {
        Vector2 item_position = GetPosition() + Vector2{0.0f, offset - GetScrollInterp()};
        Vector2 item_size = Vector2{GetSize().x, rect_height};
        bool item_hovered = Input::IsMouseOverRect(item_position, item_size) && hovered;

        if (!UI::ToLower(GetItems()[i]).contains(UI::ToLower(GetFilter())))
            continue;

        SetAnimItemHoverAt(
            i, std::clamp(GetAnimItemAtHover(i) + (10.0f * ImGui::GetIO().DeltaTime * (item_hovered ? 1.0f : -1.0f)), 0.0f, 1.0f));
        SetAnimItemActiveAt(
            i, std::clamp(GetAnimItemAtActive(i) + (10.0f * ImGui::GetIO().DeltaTime * ((*GetValue() == i) ? 1.0f : -1.0f)), 0.0f, 1.0f));

        if (item_hovered && Input::IsMouseClicked())
        {
            UI::SetBlockingObject(nullptr, GetParent());
            *GetValue() = static_cast<int>(i);
        }

        Render::AddText(GetItems()[i],
                         item_position + Vector2{Style::padding + (Style::padding * GetAnimItemAtHover(i)), rect_height / 2.0f - 1.0f},
                         Style::dimmedText.Lerp(Style::text, GetAnimItemAtHover(i)).Lerp(Style::accentColor, GetAnimItemAtActive(i)),
                         TEXT_FLAG_NONE, {0.0f, 0.5f});

        offset += item_size.y;
    }
    Render::PopClipRect();

    bool should_scroll = filtered_items.size() > GetMaxItems();
    if (should_scroll)
    {
        Vector2 track_pos = GetPosition() + Vector2{GetSize().x - 5.0f, 1.0f};
        Vector2 track_size = Vector2{4.0f, size.y - 2.0f};

        float offset = rect_height * filtered_items.size();
        float max_height = rect_height * GetMaxItems();
        float top_alpha = std::clamp(GetScrollInterp() / 15.0f, 0.0f, 1.0f);
        float bottom_alpha = std::clamp(((offset - max_height) - GetScrollInterp()) / 15.0f, 0.0f, 1.0f);

        Render::SetLayer(Render::GetCurrentLayer() + 1);
        Render::GradientItems(
            GetPosition() + Vector2{Style::rounding, 1.0f}, Vector2{GetSize().x - (track_size.x + Style::rounding), 15.0f},
            Style::widgetBackground.ScaleAlpha(0.0f), Style::widgetBackground.ScaleAlpha(top_alpha), [&]
            { Render::AddRectFilled(GetPosition() + Vector2{0.0f, 1.0f}, Vector2{GetSize().x - track_size.x, 15.0f}, Color::White()); }, 0.25f);

        Render::GradientItems(
            GetPosition() + Vector2{Style::rounding, size.y - 16.0f}, Vector2{GetSize().x - track_size.x - (Style::rounding * 2.0f), 15.0f},
            Style::widgetBackground.ScaleAlpha(bottom_alpha), Style::widgetBackground.ScaleAlpha(0.0f),
            [&]
            {
                Render::AddRectFilled(GetPosition() + Vector2{Style::rounding, size.y - 16.0f},
                                        Vector2{GetSize().x - track_size.x - (Style::rounding * 2.0f), 15.0f}, Color::White());
            },
            0.25f);
        Render::SetLayer(Render::GetCurrentLayer() - 1);

        Render::AddRectFilled(track_pos, track_size, Style::background, Style::rounding, ImDrawFlags_RoundCornersRight);

        float total_content_height = track_size.y + (offset - max_height);

        float visible_ratio = std::clamp(track_size.y / total_content_height, 0.1f, 1.0f);
        float thumb_height = (track_size.y * visible_ratio);

        float adjustable_range = (offset - max_height);
        float scroll_percentage = GetScrollInterp() / adjustable_range;

        float scrollable_track_area = track_size.y - thumb_height;
        float thumb_y_offset = scroll_percentage * scrollable_track_area;

        Render::AddRectFilled(track_pos + Vector2{0.0f, thumb_y_offset}, Vector2{track_size.x, thumb_height}, Style::accentColor, Style::rounding);

        if (ImGui::GetIO().MouseWheel != 0.0f &&
            Input::IsMouseOverRect(GetPosition(), GetSize()) &&
            !UI::BeingBlocked(GetParent()))
        {
            float scroll_speed = rect_height;
            float new_offset = GetScroll() - (ImGui::GetIO().MouseWheel * scroll_speed);
            SetScroll(std::clamp(new_offset, 0.0f, std::abs(adjustable_range)));
        }
    }

    float target = should_scroll ? GetScroll() : 0.0f;
    float current = GetScrollInterp();

    float next_val = std::lerp(current, target, 25.0f * ImGui::GetIO().DeltaTime);
    if (std::abs(target - next_val) < 0.01f)
        next_val = target;

    SetScrollInterp(next_val);

    SetHoverAlpha(std::clamp(GetHoverAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (hovered ? 1.0f : -1.0f)), 0.0f, 1.0f));
}
