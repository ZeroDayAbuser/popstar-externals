#include "app.hpp"
#include "../utils/debug.hpp"
#include <Windows.h>

namespace app {

void handle_debug_hotkey() {
    static bool was_down = false;
    const bool now_down = (::GetAsyncKeyState(VK_F12) & 0x8000) != 0;
    if (now_down && !was_down) {
        debug::enabled = !debug::enabled;
        DBG("[debug] {}", debug::enabled ? "enabled" : "disabled");
    }
    was_down = now_down;
}

}
