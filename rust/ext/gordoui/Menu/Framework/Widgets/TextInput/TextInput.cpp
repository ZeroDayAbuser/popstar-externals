#include <Cheat.hpp>

void CTextInput::Render()
{
    SetContainedAlpha(Render::alpha);

    std::string name = UI::WrapText(GetName(), GetSize().x - (Style::padding * 2.0f));
    Vector2 text_size = Render::CalcTextSize(name);
    float rect_height = text_size.y + 6.0f;

    std::string raw_value = *GetValue();
    if (GetHashed())
        raw_value = std::string(raw_value.length(), '*');

    float wrap_width = GetSize().x - (Style::padding * 2.0f);
    std::string display_text = UI::WrapText(raw_value, wrap_width);

    Vector2 value_size = Render::CalcTextSize(display_text);
    if (value_size.y > 16.0f)
        rect_height = value_size.y + 10.0f;

    Vector2 size = Vector2{GetSize().x, text_size.y + (Style::padding * 0.5f) + rect_height};

    if (GetUseNameAsPreview())
        size.y = rect_height;

    SetSize(size);
    size.y = rect_height;

    if (!GetUseNameAsPreview())
    {
        Render::AddText(name, GetPosition(), Style::dimmedText.Lerp(Style::text, GetHoverAlpha()), TEXT_FLAG_NONE, {0.0f, 0.0f});
        SetPosition({GetPosition().x, GetPosition().y + text_size.y + (Style::padding * 0.5f)});
    }

    bool hovered = Input::IsMouseOverRect(GetPosition(), size) && !UI::BeingBlocked(GetParent()) && !GetHidden();
    Vector2 content_start_pos = GetPosition() + Vector2{Style::padding, size.y / 2.0f};
    float line_height = Render::CalcTextSize("A").y;

    if (hovered && Input::IsMouseClicked())
    {
        SetActive(true);

        float click_local_x = ImGui::GetIO().MousePos.x - content_start_pos.x;
        float click_local_y = ImGui::GetIO().MousePos.y - (content_start_pos.y - (value_size.y / 2.0f));

        std::size_t best_index = 0;
        float min_dist = FLT_MAX;

        std::size_t line_start = 0;
        float current_y = 0.0f;

        for (std::size_t i = 0; i <= raw_value.length(); ++i)
        {
            if (i > 0)
            {
                std::string line_check = raw_value.substr(line_start, i - line_start);
                if (Render::CalcTextSize(line_check).x > wrap_width)
                {
                    line_start = i - 1;
                    current_y += line_height;
                }
            }

            std::string current_line_str = raw_value.substr(line_start, i - line_start);
            Vector2 sz = Render::CalcTextSize(current_line_str);

            float dist_x = std::abs(click_local_x - sz.x);
            float dist_y = std::abs(click_local_y - (current_y + line_height * 0.5f));
            float dist = dist_y * 100.0f + dist_x;

            if (dist < min_dist)
            {
                min_dist = dist;
                best_index = i;
            }
        }

        SetCursorPos(best_index);
        if (!Input::IsKeyDown(VK_SHIFT))
            SetSelectionAnchor(best_index);
    }

    if (GetActive())
    {
        if (!hovered && Input::IsMouseClicked())
        {
            SetActive(false);
            UI::SetBlockingObject(nullptr, GetParent());
        }

        bool ctrl = ImGui::GetIO().KeyCtrl;
        bool shift = ImGui::GetIO().KeyShift;
        bool changed = false;

        if (Input::IsKeyPressed(VK_ESCAPE) || Input::IsKeyPressed(VK_RETURN) || ImGui::IsKeyPressed(ImGuiKey_Escape) ||
            ImGui::IsKeyPressed(ImGuiKey_Enter))
        {
            SetActive(false);
            UI::SetBlockingObject(nullptr, GetParent());
        }

        auto move_left = [&]()
        {
            if (ctrl)
            {
                if (cursorPos > 0)
                {
                    while (cursorPos > 0 && std::isspace((*GetValue())[cursorPos - 1]))
                        cursorPos--;
                    while (cursorPos > 0 && !std::isspace((*GetValue())[cursorPos - 1]))
                        cursorPos--;
                }
            }
            else if (cursorPos > 0)
                cursorPos--;
        };

        auto move_right = [&]()
        {
            if (ctrl)
            {
                if (cursorPos < GetValue()->length())
                {
                    while (cursorPos < GetValue()->length() && !std::isspace((*GetValue())[cursorPos]))
                        cursorPos++;
                    while (cursorPos < GetValue()->length() && std::isspace((*GetValue())[cursorPos]))
                        cursorPos++;
                }
            }
            else if (cursorPos < GetValue()->length())
                cursorPos++;
        };

        if (Input::IsKeyPressed(VK_LEFT))
        {
            move_left();
            if (!shift)
                selectionAnchor = cursorPos;
        }
        if (Input::IsKeyPressed(VK_RIGHT))
        {
            move_right();
            if (!shift)
                selectionAnchor = cursorPos;
        }
        if (Input::IsKeyPressed(VK_HOME))
        {
            cursorPos = 0;
            if (!shift)
                selectionAnchor = cursorPos;
        }
        if (Input::IsKeyPressed(VK_END))
        {
            cursorPos = GetValue()->length();
            if (!shift)
                selectionAnchor = cursorPos;
        }

        if (ctrl && Input::IsKeyPressed('A'))
        {
            selectionAnchor = 0;
            cursorPos = GetValue()->length();
        }

        if (Input::IsKeyPressed(VK_BACK) || ImGui::IsKeyPressed(ImGuiKey_Backspace))
        {
            if (selectionAnchor != cursorPos)
            {
                std::size_t start = std::min(selectionAnchor, cursorPos);
                std::size_t end = std::max(selectionAnchor, cursorPos);
                GetValue()->erase(start, end - start);
                cursorPos = start;
                selectionAnchor = start;
                changed = true;
            }
            else if (cursorPos > 0)
            {
                GetValue()->erase(cursorPos - 1, 1);
                cursorPos--;
                selectionAnchor = cursorPos;
                changed = true;
            }
        }

        if (Input::IsKeyPressed(VK_DELETE))
        {
            if (selectionAnchor != cursorPos)
            {
                std::size_t start = std::min(selectionAnchor, cursorPos);
                std::size_t end = std::max(selectionAnchor, cursorPos);
                GetValue()->erase(start, end - start);
                cursorPos = start;
                selectionAnchor = start;
                changed = true;
            }
            else if (cursorPos < GetValue()->length())
            {
                GetValue()->erase(cursorPos, 1);
                changed = true;
            }
        }

        bool is_copy_shortcut = (ctrl && (Input::IsKeyDown('C') || ImGui::IsKeyPressed(ImGuiKey_C)));
        if (is_copy_shortcut && selectionAnchor != cursorPos)
        {
            std::size_t start = std::min(selectionAnchor, cursorPos);
            std::size_t end = std::max(selectionAnchor, cursorPos);
            std::string text_to_copy = GetValue()->substr(start, end - start);
            if (OpenClipboard(NULL))
            {
                EmptyClipboard();
                HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, text_to_copy.size() + 1);
                if (hg)
                {
                    memcpy(GlobalLock(hg), text_to_copy.c_str(), text_to_copy.size() + 1);
                    GlobalUnlock(hg);
                    SetClipboardData(CF_TEXT, hg);
                }
                CloseClipboard();
            }
        }

        bool is_paste_shortcut = (ctrl && (Input::IsKeyPressed('V') || ImGui::IsKeyPressed(ImGuiKey_V)));
        if (is_paste_shortcut)
        {
            if (IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard(NULL))
            {
                HANDLE hData = GetClipboardData(CF_TEXT);
                if (hData)
                {
                    char *pszText = static_cast<char *>(GlobalLock(hData));
                    if (pszText)
                    {
                        std::string clipboard(pszText);
                        GlobalUnlock(hData);
                        if (selectionAnchor != cursorPos)
                        {
                            std::size_t start = std::min(selectionAnchor, cursorPos);
                            std::size_t end = std::max(selectionAnchor, cursorPos);
                            GetValue()->erase(start, end - start);
                            cursorPos = start;
                        }
                        GetValue()->insert(cursorPos, clipboard);
                        cursorPos += clipboard.length();
                        selectionAnchor = cursorPos;
                        changed = true;
                    }
                }
                CloseClipboard();
            }
        }

        if (!ctrl)
        {
            for (int n = 0; n < ImGui::GetIO().InputQueueCharacters.Size; n++)
            {
                unsigned int c = ImGui::GetIO().InputQueueCharacters[n];
                if (c >= 32 && c < 0x10000 && c != 3 && c != 22)
                {
                    if (selectionAnchor != cursorPos)
                    {
                        std::size_t start = std::min(selectionAnchor, cursorPos);
                        std::size_t end = std::max(selectionAnchor, cursorPos);
                        GetValue()->erase(start, end - start);
                        cursorPos = start;
                    }
                    if (c < 128)
                    {
                        GetValue()->insert(cursorPos, 1, (char)c);
                        cursorPos++;
                        selectionAnchor = cursorPos;
                        changed = true;
                    }
                }
            }
        }

        if (changed)
        {
            cursorPos = std::min(cursorPos, GetValue()->length());
            selectionAnchor = std::min(selectionAnchor, GetValue()->length());
        }
    }

    SetHoverAlpha(std::clamp(GetHoverAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (hovered || GetActive() ? 1.0f : -1.0f)), 0.0f, 1.0f));
    SetActiveAlpha(std::clamp(GetActiveAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (GetActive() ? 1.0f : -1.0f)), 0.0f, 1.0f));
    SetNotEmptyAlpha(
        std::clamp(GetNotEmptyAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (GetValue()->empty() ? 1.0f : -1.0f)),
                   0.0f, 1.0f));

    Render::AddShadowRect(GetPosition(), size, Style::shadow.ScaleAlpha(0.5f + (0.5f * GetHoverAlpha())), 15.0f, 3.0f);
    Render::AddRectFilled(GetPosition(), size, Style::widgetBackground, 3.0f);
    Render::AddRect(GetPosition(), size, Style::outline, 3.0f);

    if (GetUseNameAsPreview())
        Render::AddText(name, GetPosition() + Vector2{Style::padding, size.y / 2.0f}, Style::dimmedText.ScaleAlpha(GetNotEmptyAlpha()), TEXT_FLAG_NONE,
                         {0.0f, 0.5f});

    Render::PushClipRect(GetPosition() + Vector2{3.0f, 0.0f}, size - Vector2{6.0f, 0.0f}, true);

    Vector2 render_start_pos = content_start_pos - Vector2{0.0f, value_size.y / 2.0f};

    if (GetActive() && selectionAnchor != cursorPos)
    {
        std::size_t start = std::min(selectionAnchor, cursorPos);
        std::size_t end = std::max(selectionAnchor, cursorPos);

        std::size_t line_start = 0;
        float current_y = 0.0f;

        for (std::size_t i = 0; i < raw_value.length(); ++i)
        {
            std::string line_check = raw_value.substr(line_start, (i + 1) - line_start);
            if (Render::CalcTextSize(line_check).x > wrap_width)
            {
                line_start = i;
                current_y += line_height;
            }

            if (i >= start && i < end)
            {
                float char_w = Render::CalcTextSize(raw_value.substr(i, 1)).x;
                float prev_x = Render::CalcTextSize(raw_value.substr(line_start, i - line_start)).x;
                Render::AddRectFilled(render_start_pos + Vector2{prev_x, current_y}, Vector2{char_w, line_height}, Style::text.ScaleAlpha(0.2f),
                                        0.0f);
            }
        }
    }

    Render::AddText(display_text, content_start_pos, Style::text.ScaleAlpha(1.0f - GetNotEmptyAlpha()), TEXT_FLAG_NONE, {0.0f, 0.5f});

    if (GetActive() && (fmod(ImGui::GetTime(), 1.0f) < 0.5f))
    {
        std::size_t line_start = 0;
        float current_y = 0.0f;
        float cursor_x = 0.0f;

        for (std::size_t i = 0; i <= cursorPos; ++i)
        {
            if (i > 0)
            {
                std::string line_check = raw_value.substr(line_start, i - line_start);
                if (Render::CalcTextSize(line_check).x > wrap_width)
                {
                    line_start = i - 1;
                    current_y += line_height;
                }
            }

            if (i == cursorPos)
            {
                cursor_x = Render::CalcTextSize(raw_value.substr(line_start, i - line_start)).x;
            }
        }

        Render::AddRectFilled(render_start_pos + Vector2{cursor_x, current_y}, Vector2{1.0f, line_height}, Style::text);
    }

    Render::PopClipRect();
}
