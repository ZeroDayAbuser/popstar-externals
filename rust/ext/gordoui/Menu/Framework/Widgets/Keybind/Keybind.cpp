#include <Cheat.hpp>

static int ImGuiKeyToVirtualKey(ImGuiKey k)
{
    switch (k)
    {
    case ImGuiKey_MouseLeft:
        return VK_LBUTTON;
    case ImGuiKey_MouseRight:
        return VK_RBUTTON;
    case ImGuiKey_MouseMiddle:
        return VK_MBUTTON;
    case ImGuiKey_MouseX1:
        return VK_XBUTTON1;
    case ImGuiKey_MouseX2:
        return VK_XBUTTON2;
    case ImGuiKey_Space:
        return VK_SPACE;
    case ImGuiKey_Escape:
        return VK_ESCAPE;
    case ImGuiKey_Enter:
        return VK_RETURN;
    case ImGuiKey_Tab:
        return VK_TAB;
    case ImGuiKey_LeftArrow:
        return VK_LEFT;
    case ImGuiKey_RightArrow:
        return VK_RIGHT;
    case ImGuiKey_UpArrow:
        return VK_UP;
    case ImGuiKey_DownArrow:
        return VK_DOWN;
    case ImGuiKey_Backspace:
        return VK_BACK;
    case ImGuiKey_Delete:
        return VK_DELETE;
    case ImGuiKey_Insert:
        return VK_INSERT;
    case ImGuiKey_Home:
        return VK_HOME;
    case ImGuiKey_End:
        return VK_END;
    case ImGuiKey_PageUp:
        return VK_PRIOR;
    case ImGuiKey_PageDown:
        return VK_NEXT;
    case ImGuiKey_LeftShift:
        return VK_LSHIFT;
    case ImGuiKey_RightShift:
        return VK_RSHIFT;
    case ImGuiKey_LeftCtrl:
        return VK_LCONTROL;
    case ImGuiKey_RightCtrl:
        return VK_RCONTROL;
    case ImGuiKey_LeftAlt:
        return VK_LMENU;
    case ImGuiKey_RightAlt:
        return VK_RMENU;
    case ImGuiKey_CapsLock:
        return VK_CAPITAL;
    case ImGuiKey_ScrollLock:
        return VK_SCROLL;
    case ImGuiKey_NumLock:
        return VK_NUMLOCK;
    case ImGuiKey_PrintScreen:
        return VK_SNAPSHOT;
    case ImGuiKey_Pause:
        return VK_PAUSE;
    case ImGuiKey_GraveAccent:
        return VK_OEM_3;
    case ImGuiKey_Minus:
        return VK_OEM_MINUS;
    case ImGuiKey_Equal:
        return VK_OEM_PLUS;
    case ImGuiKey_LeftBracket:
        return VK_OEM_4;
    case ImGuiKey_RightBracket:
        return VK_OEM_6;
    case ImGuiKey_Backslash:
        return VK_OEM_5;
    case ImGuiKey_Semicolon:
        return VK_OEM_1;
    case ImGuiKey_Apostrophe:
        return VK_OEM_7;
    case ImGuiKey_Comma:
        return VK_OEM_COMMA;
    case ImGuiKey_Period:
        return VK_OEM_PERIOD;
    case ImGuiKey_Slash:
        return VK_OEM_2;
    case ImGuiKey_Keypad0:
        return VK_NUMPAD0;
    case ImGuiKey_Keypad1:
        return VK_NUMPAD1;
    case ImGuiKey_Keypad2:
        return VK_NUMPAD2;
    case ImGuiKey_Keypad3:
        return VK_NUMPAD3;
    case ImGuiKey_Keypad4:
        return VK_NUMPAD4;
    case ImGuiKey_Keypad5:
        return VK_NUMPAD5;
    case ImGuiKey_Keypad6:
        return VK_NUMPAD6;
    case ImGuiKey_Keypad7:
        return VK_NUMPAD7;
    case ImGuiKey_Keypad8:
        return VK_NUMPAD8;
    case ImGuiKey_Keypad9:
        return VK_NUMPAD9;
    case ImGuiKey_KeypadDecimal:
        return VK_DECIMAL;
    case ImGuiKey_KeypadDivide:
        return VK_DIVIDE;
    case ImGuiKey_KeypadMultiply:
        return VK_MULTIPLY;
    case ImGuiKey_KeypadSubtract:
        return VK_SUBTRACT;
    case ImGuiKey_KeypadAdd:
        return VK_ADD;
    default:
        break;
    }
    if (k >= ImGuiKey_A && k <= ImGuiKey_Z)
        return 'A' + (int)(k - ImGuiKey_A);
    if (k >= ImGuiKey_0 && k <= ImGuiKey_9)
        return '0' + (int)(k - ImGuiKey_0);
    if (k >= ImGuiKey_F1 && k <= ImGuiKey_F24)
        return VK_F1 + (int)(k - ImGuiKey_F1);
    return 0;
}

static void GetVkKeyName(int vk_key, char *buffer)
{
    if (vk_key == 0)
    {
        std::strcpy(buffer, "None");
        return;
    }
    switch (vk_key)
    {
    case VK_XBUTTON1:
        std::strcpy(buffer, "M4");
        return;
    case VK_XBUTTON2:
        std::strcpy(buffer, "M5");
        return;
    case VK_LBUTTON:
        std::strcpy(buffer, "M1");
        return;
    case VK_MBUTTON:
        std::strcpy(buffer, "M3");
        return;
    case VK_RBUTTON:
        std::strcpy(buffer, "M2");
        return;
    }
    UINT scan_code = MapVirtualKeyA((UINT)vk_key, MAPVK_VK_TO_VSC);
    switch (vk_key)
    {
    case VK_LEFT:
    case VK_UP:
    case VK_RIGHT:
    case VK_DOWN:
    case VK_RCONTROL:
    case VK_RMENU:
    case VK_LWIN:
    case VK_RWIN:
    case VK_APPS:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_END:
    case VK_HOME:
    case VK_DIVIDE:
    case VK_NUMLOCK:
        scan_code |= KF_EXTENDED;
        break;
    case VK_INSERT:
        scan_code |= KF_EXTENDED;
        std::strcpy(buffer, "INS");
        return;
    case VK_DELETE:
        scan_code |= KF_EXTENDED;
        std::strcpy(buffer, "DEL");
        return;
    }
    if (GetKeyNameTextA((int)(scan_code << 16), buffer, 128) == 0)
    {
        std::strcpy(buffer, "...");
        return;
    }
    for (int i = 0; i < (int)std::strlen(buffer); i++)
        buffer[i] = (char)std::tolower((unsigned char)buffer[i]);
    buffer[0] = (char)std::toupper((unsigned char)buffer[0]);
}

class CKeybindSelectable : public CObject
{
public:
    CKeybindSelectable(std::string name, std::function<void()> callback, int index, int *currentMode)
    {
        SetName(name);
        this->callback = callback;
        this->index = index;
        this->currentMode = currentMode;
    }

    void Render() override
    {
        Vector2 text_size = Render::CalcTextSize(GetName());
        SetSize({GetParent()->GetSize().x - Style::padding * 2.0f, text_size.y + 6.0f});

        bool hovered = Input::IsMouseOverRect(GetPosition(), GetSize());
        if (hovered && Input::IsMouseClicked())
        {
            if (callback)
                callback();
        }

        bool is_active = (currentMode && *currentMode == index);

        hoverAlpha = std::clamp(hoverAlpha + (10.0f * ImGui::GetIO().DeltaTime * (hovered ? 1.0f : -1.0f)), 0.0f, 1.0f);

        Color text_col = Style::dimmedText.Lerp(Style::text, hoverAlpha);
        if (is_active)
            text_col = Style::accentColor;

        // Render::AddRect(GetPosition(), GetSize(), Color::Red());

        Render::AddText(GetName(), GetPosition() + Vector2{GetSize().x * 0.5f, GetSize().y * 0.5f}, text_col, TEXT_FLAG_NONE, {0.5f, 0.5f});
    }
private:
    std::function<void()> callback;
    int index;
    int *currentMode;
    float hoverAlpha = 0.0f;
};

void CKeybind::Render()
{
    SetContainedAlpha(Render::alpha);

    float base_field_target = 80.0f;
    Vector2 text_size = Render::CalcTextSize(GetName());
    Vector2 size = Vector2{GetParent()->GetSize().x - (Style::padding * 2.0f), 20.0f};
    SetSize(size);

    Render::AddText(GetName(), GetPosition() + Vector2{0.0f, size.y * 0.5f}, Style::dimmedText.Lerp(Style::text, GetEnabledAlpha()), TEXT_FLAG_NONE,
                    {0.0f, 0.5f});

    char buf[128] = {};
    int key = value->key;
    if (listening)
        std::strcpy(buf, "...");
    else if (key < 0)
        GetVkKeyName(0, buf);
    else if (value->mode == 3)
        std::strcpy(buf, "Always");
    else
        GetVkKeyName(key, buf);
    std::string summary = buf;

    Vector2 field_size = {base_field_target, size.y};
    Vector2 field_pos = GetPosition() + Vector2(size.x - base_field_target, 0.0f);

    bool hovered = Input::IsMouseOverRect(field_pos, field_size) && !UI::BeingBlocked(GetParent()) && !GetHidden();

    if (hovered && Input::IsMouseClicked() && value->mode != 3)
    {
        listening = true;
        awaitRelease = true;
        UI::SetBlockingObject(this, GetParent());
    }

    if (hovered && Input::IsMouseClicked(1))
    {
        if (!statePopup)
        {
            statePopup = std::make_unique<CPopup>();
            statePopup->SetParent(this);

            statePopup->SetSize({field_size.x, 0.0f});
            statePopup->SetOverrideSize(true);

            const char *modes[] = {("Disabled"), ("Hold"), ("Toggled"), ("Always")};
            for (int i = 0; i < 4; ++i)
            {
                statePopup->AddObject<CKeybindSelectable>(
                    modes[i],
                    [this, i]()
                    {
                        value->mode = i;
                        statePopup->SetOpened(false);
                        UI::SetBlockingObject(nullptr, GetParent());
                    },
                    i, &value->mode);
            }
        }

        if (statePopup)
        {
            statePopup->SetOpened(true);
            statePopup->SetPosition(field_pos + Vector2{0.0f, field_size.y + Style::padding});

            statePopup->SetSize({field_size.x, statePopup->GetSize().y});

            UI::SetBlockingObject(statePopup.get(), GetParent());
        }
    }

    float target_active = (listening || (statePopup && (statePopup)->GetOpened())) ? 1.0f : 0.0f;
    SetFieldActiveAlpha(std::lerp(GetFieldActiveAlpha(), target_active, 15.0f * ImGui::GetIO().DeltaTime));
    SetAlwaysAlpha(std::lerp(GetAlwaysAlpha(), value->mode == 3, 15.0f * ImGui::GetIO().DeltaTime));

    Render::AddShadowRect(field_pos, field_size, Style::shadow, 15.0f, Style::rounding);
    Render::AddRectFilled(field_pos, field_size, Style::widgetBackground, Style::rounding);
    Render::AddRect(field_pos, field_size, Style::outline, Style::rounding);

    Color text_color = Style::text.Lerp(Style::accentColor, GetFieldActiveAlpha() + GetAlwaysAlpha());
    Render::AddText(summary, field_pos + field_size / 2.0f, text_color, TEXT_FLAG_NONE, {0.5f, 0.5f});

    if (listening)
    {
        if (awaitRelease)
        {
            if (!Input::IsKeyDown(VK_LBUTTON))
                awaitRelease = false;
        }
        else
        {
            if (Input::IsKeyPressed(VK_ESCAPE))
            {
                if (value)
                    value->key = 0;
                listening = false;
                UI::SetBlockingObject(nullptr, GetParent());
            }
            else
            {
                for (std::size_t i = 0; i < 256; i++)
                {
                    if (Input::IsKeyPressed(i))
                    {
                        value->key = i;
                        listening = false;
                        UI::SetBlockingObject(nullptr, GetParent());
                        break;
                    }
                }
            }
        }
    }

    if (statePopup)
    {
        float padding_save = Style::padding;
        Style::padding = 0.0f;
        float alpha_save = Render::alpha;
        Render::alpha *= GetFieldActiveAlpha();
        statePopup->Render();
        Render::alpha = alpha_save;
        Style::padding = padding_save;
    }

    Vector2 base_size = Vector2{size.y, size.y};
    Vector2 base_position = GetPosition() + Vector2{size.x - (80.0f + (Style::padding * 3.0f)), 0.0f};

    for (auto c : GetChildren())
    {
        CColorPicker *cp = dynamic_cast<CColorPicker *>(c);
        if (cp)
        {
            Render::AddShadowRect(base_position, base_size,
                                  Style::shadow.Lerp(cp->GetValue()->OverrideAlpha(1.0f * Render::alpha), cp->GetOpenAlpha())
                                      .ScaleAlpha(0.5f + (0.5f * cp->GetOpenAlpha())),
                                  15.0f, Style::rounding);
            Render::AddRectFilled(base_position, base_size, cp->GetValue()->OverrideAlpha(1.0f * Render::alpha), Style::rounding);
            bool cp_hovered = Input::IsMouseOverRect(base_position, base_size) && !UI::BeingBlocked(GetParent()) && !GetHidden();
            if (cp_hovered && Input::IsMouseClicked())
            {
                cp->SetPosition(ImGui::GetIO().MousePos);
                cp->SetOpened(true);
                UI::SetBlockingObject(cp, GetParent());
            }

            if (cp_hovered)
            {
                if (ImGui::GetIO().KeyCtrl && (Input::IsKeyDown('C') || ImGui::IsKeyPressed(ImGuiKey_C)))
                    Style::copiedColor = *cp->GetValue();

                if (ImGui::GetIO().KeyCtrl && (Input::IsKeyDown('V') || ImGui::IsKeyPressed(ImGuiKey_V)))
                    *cp->GetValue() = Style::copiedColor;
            }

            // this is cancer
            float alpha_save = Render::alpha;
            Render::alpha *= cp->GetOpenAlpha();
            cp->Render();
            Render::alpha = alpha_save;

            base_position.x -= base_size.x + Style::padding;
        }

        CPopup *pu = dynamic_cast<CPopup *>(c);
        if (pu)
        {
            Render::AddShadowCircle(base_position + base_size / 2.0f, 7.0f, Style::shadow.ScaleAlpha(0.5f + (0.5f * pu->GetOpenAlpha())), 30.0f);
            Render::AddText(ICON_FA_GEAR, base_position + base_size / 2.0f, Style::dimmedText.Lerp(Style::text, pu->GetOpenAlpha()), TEXT_FLAG_NONE,
                            {0.5f, 0.5f}, Render::Fonts::fontAwesome14px);

            bool cp_hovered = Input::IsMouseOverRect(base_position, base_size) && !UI::BeingBlocked(GetParent()) && !GetHidden();
            if (cp_hovered && Input::IsMouseClicked())
            {
                pu->SetPosition(ImGui::GetIO().MousePos);
                pu->SetOpened(true);
                UI::SetBlockingObject(pu, GetParent());
            }

            // this is cancer
            float alpha_save = Render::alpha;
            Render::alpha *= pu->GetOpenAlpha();
            pu->Render();
            Render::alpha = alpha_save;

            base_position.x -= base_size.x + Style::padding;
        }
    }

    SetHoverAlpha(std::clamp(GetHoverAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (hovered ? 1.0f : -1.0f)), 0.0f, 1.0f));
    SetEnabledAlpha(std::clamp(GetEnabledAlpha() + (10.0f * ImGui::GetIO().DeltaTime * (value->Enabled() ? 1.0f : -1.0f)), 0.0f, 1.0f));
}

bool Keybind::GetState(std::function<void()> stateChangeCallback)
{
    switch (mode)
    {
    case 1:
        return GetAsyncKeyState(key) & 0x8000;
        break;
    case 2:
        if (GetAsyncKeyState(key) & 0x8000)
        {
            while (GetAsyncKeyState(key) & 0x8000)
                ;

            toggleState = !toggleState;
            constCb = true;

            if (stateChangeCallback)
                stateChangeCallback();
        }
        return toggleState;
        break;
    case 3:
        return true;
        break;
    }

    return false;
}

bool Keybind::GetConstState(std::function<void()> stateChangeCallback)
{
    switch (mode)
    {
    case 1:
        return GetAsyncKeyState(key) & 0x8000;
        break;
    case 2:
        if (constCb)
        {
            constCb = false;

            if (stateChangeCallback)
                stateChangeCallback();
        }
        return toggleState;
        break;
    case 3:
        return true;
        break;
    }

    return false;
}
