#include <Cheat.hpp>

void CContainer::Render()
{
    // shadow
    Render::AddShadowRect(GetPosition(), GetSize(), Style::shadow, 15.0f, Style::rounding);
    // background
    Render::AddRectFilled(GetPosition(), GetSize(), Style::container, Style::rounding);
    // header dropshadow
    Render::SetLayer(Render::GetCurrentLayer() + 2);
    Render::AddDropshadow(GetPosition() + Vector2{0.0f, Style::containerHeaderHeight}, GetSize().x, Style::shadow, 15.0f);
    Render::SetLayer(Render::GetCurrentLayer() - 2);

    // header bg
    Render::AddRectFilled(GetPosition(), Vector2{GetSize().x, Style::containerHeaderHeight}, Style::containerHeader, Style::rounding,
                            ImDrawFlags_RoundCornersTop);
    // header text
    Render::AddText(GetName(), GetPosition() + Vector2{Style::padding, Style::containerHeaderHeight / 2.0f}, Style::headerText, TEXT_FLAG_NONE,
                     {0.0f, 0.5f});

    // bottom header outline
    Render::AddRectFilled(GetPosition() + Vector2{0.0f, Style::containerHeaderHeight - 1.0f}, Vector2{GetSize().x, 1.0f}, Style::outline);

    // outline
    Render::SetLayer(Render::GetCurrentLayer() + 3);
    Render::AddRect(GetPosition(), GetSize(), Style::outline, Style::rounding);
    Render::SetLayer(Render::GetCurrentLayer() - 3);

    Render::PushClipRect(GetPosition() + Vector2{0.0f, Style::containerHeaderHeight},
                           Vector2{GetSize().x, GetSize().y - Style::containerHeaderHeight});
    float offset = Style::containerHeaderHeight + Style::padding;
    for (auto c : GetChildren())
    { 
        c->UpdateState();
        if (c->GetHidden() && c->GetAlpha() <= 0.0f)
            continue;

        // set the base position of the widget and its working size of the container
        c->SetPosition(GetPosition() + Vector2(Style::padding, offset - this->GetScrollOffset() - (c->GetSize().y * (1.0f - c->GetAlpha()))));
        c->SetSize(GetSize() - Vector2(Style::padding * 2.0f, Style::padding * 2.0f));

        // render the widget (will set the size in render)
        float alpha_save = Render::alpha;
        Render::alpha *= c->GetAlpha();
        c->Render();
        Render::alpha = alpha_save;

        // push the offset from the widget to the container
        offset += std::roundf((c->GetSize().y + Style::padding) * c->GetAlpha());
    }
    Render::PopClipRect();

    // clamp the container height to min of the header and max of the max working height
    auto parent = GetParent();
    float parent_bottom = parent->GetPosition().y + parent->GetSize().y;
    float current_y = this->GetPosition().y;
    float max_height = parent_bottom - current_y + (Style::padding * 0.5f);

    max_height = this->GetForcedContentHeight() ? max_height * this->GetForcedHeightFraction() : max_height;

    float clamped_height = std::clamp(offset, Style::containerHeaderHeight + Style::padding, std::max(max_height, Style::containerHeaderHeight + Style::padding));
    if (this->GetForcedContentHeight())
        clamped_height = std::min(clamped_height, max_height);

    bool needs_scrolling = offset >= max_height;
    this->SetScrollState(needs_scrolling || this->GetForcedContentHeight());

    if (this->GetScrollState())
    {
        Vector2 track_pos = GetPosition() + Vector2{GetSize().x - 5.0f, Style::containerHeaderHeight};
        Vector2 track_size = Vector2{4.0f, GetSize().y - Style::containerHeaderHeight - 1.0f};

        float top_alpha = std::clamp(this->GetScrollOffset() / 15.0f, 0.0f, 1.0f);
        float bottom_alpha = std::clamp(((offset - max_height) - this->GetScrollOffset()) / 15.0f, 0.0f, 1.0f);

        Render::SetLayer(Render::GetCurrentLayer() + 1);
        Render::GradientItems(
            GetPosition() + Vector2{0.0f, Style::containerHeaderHeight},
            Vector2{GetSize().x - track_size.x, 15.0f},
            Style::container.ScaleAlpha(0.0f), Style::container.ScaleAlpha(top_alpha),
            [&] { Render::AddRectFilled(GetPosition() + Vector2{0.0f, Style::containerHeaderHeight}, Vector2{GetSize().x - track_size.x, 15.0f}, Color::White());
            }, 0.25f);

        Render::GradientItems(
            GetPosition() + Vector2{Style::rounding, GetSize().y - 15.0f},
            Vector2{GetSize().x - track_size.x - (Style::rounding * 2.0f), 15.0f},
            Style::container.ScaleAlpha(bottom_alpha), Style::container.ScaleAlpha(0.0f),
            [&] { Render::AddRectFilled(GetPosition() + Vector2{Style::rounding, GetSize().y - 15.0f}, Vector2{GetSize().x - track_size.x - (Style::rounding * 2.0f), 15.0f}, Color::White());
            }, 0.25f);
        Render::SetLayer(Render::GetCurrentLayer() - 1);

        Render::AddRectFilled(track_pos, track_size, Style::background, Style::rounding, ImDrawFlags_RoundCornersBottomRight);

        float total_content_height = track_size.y + (offset - max_height);

        float visible_ratio = std::clamp(track_size.y / total_content_height, 0.1f, 1.0f);
        float thumb_height = (track_size.y * visible_ratio);

        float adjustable_range = (offset - max_height);
        float scroll_percentage = this->GetScrollOffset() / adjustable_range;

        float scrollable_track_area = track_size.y - thumb_height;
        float thumb_y_offset = scroll_percentage * scrollable_track_area;

        Render::AddRectFilled(track_pos + Vector2{0.0f, thumb_y_offset}, Vector2{track_size.x, thumb_height}, Style::accentColor, Style::rounding);

        if (ImGui::GetIO().MouseWheel != 0.0f &&
            Input::IsMouseOverRect(GetPosition(), GetSize()) &&
            !UI::BeingBlocked(GetParent()))
        {
            float scroll_speed = std::abs(adjustable_range) / 5.0f;
            float new_offset = this->GetScrollOffset() - (ImGui::GetIO().MouseWheel * scroll_speed);
            this->SetScrollOffset(std::clamp(new_offset, 0.0f, std::abs(adjustable_range)));
        }
    }

    this->SetScrollOffsetLerp(std::roundf(
        std::lerp(this->GetScrollOffsetLerp(), this->GetScrollState() ? this->GetScrollOffset() : 0.0f, 25.0f * ImGui::GetIO().DeltaTime)));

    SetSize(Vector2{GetSize().x, clamped_height});
}
