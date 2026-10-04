#include <Cheat.hpp>
#include <memory/memory.hpp>

bool Input::OnWndProc(UINT msg, WPARAM w_param, LPARAM l_param)
{
    int current_key = 0;
    e_key_state current_state = e_key_state::KEYSTATE_NONE;

    switch (msg)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (w_param < 256U)
        {
            current_key = w_param;
            current_state = e_key_state::KEYSTATE_DOWN;
        }
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (w_param < 256U)
        {
            current_key = w_param;
            current_state = e_key_state::KEYSTATE_UP;
        }
        break;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
        current_key = VK_LBUTTON;
        current_state = msg == WM_LBUTTONUP ? e_key_state::KEYSTATE_UP : e_key_state::KEYSTATE_DOWN;
        break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
        current_key = VK_RBUTTON;
        current_state = msg == WM_RBUTTONUP ? e_key_state::KEYSTATE_UP : e_key_state::KEYSTATE_DOWN;
        break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
        current_key = VK_MBUTTON;
        current_state = msg == WM_MBUTTONUP ? e_key_state::KEYSTATE_UP : e_key_state::KEYSTATE_DOWN;
        break;
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_XBUTTONDBLCLK:
        current_key = (GET_XBUTTON_WPARAM(w_param) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2);
        current_state = msg == WM_XBUTTONUP ? e_key_state::KEYSTATE_UP : e_key_state::KEYSTATE_DOWN;
        break;
    default:
        return false;
    }

    keyStates[current_key] = (current_state == e_key_state::KEYSTATE_DOWN)
                                 ? (keyStates[current_key] == e_key_state::KEYSTATE_NONE || keyStates[current_key] == e_key_state::KEYSTATE_UP
                                                                                     ? e_key_state::KEYSTATE_PRESSED
                                                                                     : e_key_state::KEYSTATE_DOWN)
                                                                              : e_key_state::KEYSTATE_UP;

    return true;
}

bool Input::IsMouseOverRect(const Vector2 &position, const Vector2 &size)
{
    return ImGui::IsMouseHoveringRect(position.Floored(), Vector2(position + size).Floored(), false);
}

void Input::Poll()
{
    for (int i = 0; i < 256; i++)
    {
        const bool down = GetAsyncKeyState(i) & 0x8000;
        e_key_state &state = keyStates[i];

        if (down)
        {
            if (state == e_key_state::KEYSTATE_NONE || state == e_key_state::KEYSTATE_UP || state == e_key_state::KEYSTATE_RELEASED)
            {
                state = e_key_state::KEYSTATE_PRESSED;
            }
            else
            {
                state = e_key_state::KEYSTATE_DOWN;
            }
        }
        else
        {
            if (state == e_key_state::KEYSTATE_DOWN || state == e_key_state::KEYSTATE_PRESSED)
            {
                state = e_key_state::KEYSTATE_RELEASED;
            }
            else
            {
                state = e_key_state::KEYSTATE_UP;
            }
        }
    }
}

void Input::PostMessageQuick(const std::int32_t &virtualKey)
{
    PostMessage(memory::impl::game_hwnd, WM_KEYDOWN, virtualKey, 0);
    PostMessage(memory::impl::game_hwnd, WM_KEYUP, virtualKey, 0);
}

void Input::SendInputCommand(int inputKey, bool down, bool up)
{
    INPUT Input = {0};
    Input.type = INPUT_KEYBOARD;
    Input.ki.wVk = static_cast<WORD>(inputKey);

    if (down)
    {
        Input.ki.dwFlags = 0;
        SendInput(1, &Input, sizeof(Input));
    }

    if (up)
    {
        Input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &Input, sizeof(Input));
    }
}