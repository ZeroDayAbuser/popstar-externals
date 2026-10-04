#pragma once
#include <windows.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

// Thin forwarder to the DBD-ported Menu (src/menu/menu.cpp) and its
// Input subsystem (src/menu/framework/input). Kept under the OverlayBridge
// name so winapp.cpp's call sites don't need to change.
namespace OverlayBridge {

    void Bootstrap(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);
    void PollInput();
    bool OnWndProc(UINT msg, WPARAM wParam, LPARAM lParam);
    void Render();
    void FinishFrame();
    void Shutdown();

    bool IsMenuOpen();

    struct ShapeRect { float x, y, w, h, r; };
    ShapeRect CurrentShape();

}
