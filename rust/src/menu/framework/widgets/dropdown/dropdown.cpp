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

    const int itemCount = static_cast<int>(GetItems().size());
    if (itemCount <= 0) return;
    int* valuePtr = GetValue();
    if (!valuePtr) return;
    if (*valuePtr < 0 || *valuePtr >= itemCount) *valuePtr = 0;
    const int selIdx = *valuePtr;

    if (static_cast<int>(GetAnimItemsHover().size())  != itemCount) SetAnimItemsHover (std::vector<float>(itemCount, 0.0f));
    if (static_cast<int>(GetAnimItemsActive().size()) != itemCount) SetAnimItemsActive(std::vector<float>(itemCount, 0.0f));

    Render::AddText(GetItems()[selIdx], GetPosition() + Vector2{Style::padding, (size.y / 2.0f) - 1.0f}, Style::text, TEXT_FLAG_NONE,
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

        constexpr int kMaxVisible = 12;
        const float content_h = rect_height * static_cast<float>(itemCount);

        Vector2 anchor = GetPosition();
        float space_below = ImGui::GetIO().DisplaySize.y - (anchor.y + rect_height) - 8.0f;
        float space_above = anchor.y - 8.0f;
        const bool open_up = space_below < rect_height * 6.0f && space_above > space_below;
        const float available = open_up ? space_above : space_below;
        const int visible_count = std::max(1, std::min(itemCount, std::min(kMaxVisible, static_cast<int>(available / rect_height))));
        const float view_h = rect_height * static_cast<float>(visible_count);
        const float max_scroll = std::max(0.0f, content_h - view_h);

        Vector2 items_origin = open_up
            ? Vector2{ anchor.x, anchor.y - view_h }
            : Vector2{ anchor.x, anchor.y + rect_height };
        Vector2 items_view = Vector2{ size.x, view_h };
        Vector2 panel_origin = open_up ? items_origin : anchor;
        Vector2 panel_size = Vector2{ size.x, view_h + (open_up ? 0.0f : rect_height) };

        bool list_hovered = Input::IsMouseOverRect(items_origin, items_view) || Input::IsMouseOverRect(anchor, size);
        if (GetOpened() && !list_hovered && Input::IsMouseClicked())
        {
            UI::SetBlockingObject(nullptr, GetParent());
            SetOpened(false);
        }

        if (GetOpened() && max_scroll > 0.0f && ImGui::GetIO().MouseWheel != 0.0f)
        {
            scroll = std::clamp(scroll - ImGui::GetIO().MouseWheel * rect_height, 0.0f, max_scroll);
            ImGui::GetIO().MouseWheel = 0.0f;
        }
        if (!GetOpened())
            scroll = 0.0f;
        scroll = std::clamp(scroll, 0.0f, max_scroll);

        auto* overlay = ImGui::GetForegroundDrawList();
        overlay->PushClipRectFullScreen();

        const float open = GetActiveAlpha();
        const ImVec2 list_min{ panel_origin.x, panel_origin.y };
        const ImVec2 list_max{ panel_origin.x + panel_size.x, panel_origin.y + panel_size.y * open };
        overlay->AddRectFilled(list_min, list_max, Style::widgetBackground.ScaleAlpha(open), 3.0f);
        overlay->AddRect(list_min, list_max, Style::outline.ScaleAlpha(open), 3.0f);

        overlay->PushClipRect(
            ImVec2{ items_origin.x, items_origin.y },
            ImVec2{ items_origin.x + items_view.x, items_origin.y + items_view.y },
            true);

        for (std::size_t i = 0; i < GetItems().size(); ++i)
        {
            Vector2 item_position = items_origin + Vector2{0.0f, static_cast<float>(i) * rect_height - scroll};
            Vector2 item_size = Vector2{items_view.x, rect_height};
            bool item_hovered = Input::IsMouseOverRect(item_position, item_size)
                && item_position.y + item_size.y > items_origin.y
                && item_position.y < items_origin.y + items_view.y
                && GetOpened();

            SetAnimItemHoverAt(i, std::clamp(GetAnimItemAtHover(i) + (10.0f * ImGui::GetIO().DeltaTime * (item_hovered ? 1.0f : -1.0f)), 0.0f, 1.0f));
            SetAnimItemActiveAt(
                i, std::clamp(GetAnimItemAtActive(i) + (10.0f * ImGui::GetIO().DeltaTime * ((*GetValue() == static_cast<int>(i)) ? 1.0f : -1.0f)), 0.0f, 1.0f));

            if (item_hovered && Input::IsMouseClicked())
            {
                UI::SetBlockingObject(nullptr, GetParent());
                *GetValue() = static_cast<int>(i);
                SetOpened(false);
            }

            const Color col = Style::dimmedText.Lerp(Style::text, GetAnimItemAtHover(i)).Lerp(Style::accentColor, GetAnimItemAtActive(i)).ScaleAlpha(GetActiveAlpha());
            overlay->AddText(ImVec2{item_position.x + Style::padding, item_position.y + rect_height * 0.5f - 8.0f}, col, GetItems()[i].c_str());
        }
        overlay->PopClipRect();

        if (max_scroll > 0.0f && GetActiveAlpha() > 0.01f)
        {
            const float track_x = items_origin.x + items_view.x - 5.0f;
            const float track_y = items_origin.y + 2.0f;
            const float track_h = items_view.y - 4.0f;
            const float visible_ratio = std::clamp(view_h / content_h, 0.12f, 1.0f);
            const float thumb_h = track_h * visible_ratio;
            const float thumb_y = (scroll / max_scroll) * (track_h - thumb_h);
            overlay->AddRectFilled(ImVec2{track_x, track_y}, ImVec2{track_x + 3.0f, track_y + track_h}, Style::background.ScaleAlpha(GetActiveAlpha()), 2.0f);
            overlay->AddRectFilled(ImVec2{track_x, track_y + thumb_y}, ImVec2{track_x + 3.0f, track_y + thumb_y + thumb_h}, Style::accentColor.ScaleAlpha(GetActiveAlpha()), 2.0f);
        }

        overlay->PopClipRect();
    }

    SetHoverAlpha(std::clamp(GetHoverAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (hovered ? 1.0f : -1.0f)), 0.0f, 1.0f));
    SetActiveAlpha(std::clamp(GetActiveAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (GetOpened() ? 1.0f : -1.0f)), 0.0f, 1.0f));
}
