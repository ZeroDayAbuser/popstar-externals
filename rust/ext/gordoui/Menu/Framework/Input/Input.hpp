#pragma once

enum class e_key_state : int
{
    KEYSTATE_NONE = 0,
    KEYSTATE_DOWN,
    KEYSTATE_UP,
    KEYSTATE_PRESSED,
    KEYSTATE_RELEASED
};

namespace Input
{
    bool OnWndProc(UINT msg, WPARAM w_param, LPARAM l_param);
    void Poll();

    bool IsMouseOverRect(const Vector2 &position, const Vector2 &size);

    void PostMessageQuick(const std::int32_t &virtualKey);

    void SendInputCommand(int inputKey, bool down = true, bool up = true);

    inline std::array<e_key_state, 256U> keyStates = {};

    inline bool IsKeyDown(const std::uint32_t button_code)
    {
        return keyStates[button_code] == e_key_state::KEYSTATE_DOWN;
    }

    inline bool IsKeyReleased(const std::uint32_t button_code)
    {
        if (keyStates[button_code] == e_key_state::KEYSTATE_RELEASED)
        {
            keyStates[button_code] = e_key_state::KEYSTATE_UP;
            return true;
        }

        return false;
    }

    inline bool IsKeyPressed(const std::uint32_t button_code)
    {
        if (keyStates[button_code] == e_key_state::KEYSTATE_PRESSED)
        {
            keyStates[button_code] = e_key_state::KEYSTATE_DOWN;
            return true;
        }
        return false;
    }

    inline bool IsMouseClicked(bool right_click = false)
    {
        return IsKeyPressed(right_click ? VK_RBUTTON : VK_LBUTTON);
    }
} // namespace Input
