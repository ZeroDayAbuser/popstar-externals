#pragma once

namespace OverlayUI {

    void Init();
    void Render();
    void Shutdown();

    struct ShapeRect {
        float x, y, w, h, radius;
    };
    ShapeRect CurrentShape();

    bool GetPlayerFlag(int idx);

    // Walks kTabs looking for a WT::Keybind widget whose label matches
    // exactly. Returns the Win32 VK_* code currently bound to it, or 0
    // if the row is set to "None" / no match found.
    int  GetKeybindVk(const char* widget_label);

    bool IsWritesEnabled();
    void SetWritesEnabled(bool v);

}  // namespace OverlayUI
