#pragma once
#include <imgui.h>

#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_internal.h"

#include <d3d11.h>
#include <D3DX11tex.h>
#pragma comment(lib, "D3DX11.lib")

namespace ui {
    inline float UI_ALPHA = 0.9f;
    inline float dpi_scale = 1.f;
    inline ID3D11Device* g_pd3dDevice = nullptr;
    inline ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
    inline IDXGISwapChain* g_pSwapChain = nullptr;
    inline UINT            g_ResizeWidth = 0, g_ResizeHeight = 0;
    inline ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

    namespace colors {
        inline ImVec4 background = ImColor(16, 16, 19, int(255 * UI_ALPHA));
        inline ImVec4 background_dark = ImColor(15, 15, 18, int(255 * UI_ALPHA));
        inline ImVec4 background_light = ImColor(20, 20, 24, int(200 * UI_ALPHA));
        inline ImVec4 outline = ImColor(255, 255, 255, int(10 * UI_ALPHA));
        inline ImVec4 main = ImColor( 237 , 180 , 0, int(255 * UI_ALPHA));
        inline ImVec4 text = ImColor(224, 224, 229, int(255 * UI_ALPHA));
        inline ImVec4 text_disabled = ImColor(90, 90, 96, int(255 * UI_ALPHA));

        namespace child {
            inline ImVec4 top_background = ImColor(27, 27, 34, int(200 * UI_ALPHA));
            inline ImVec4 outline = ImColor(255, 255, 255, int(10 * UI_ALPHA));
        }

        namespace checkbox {
            inline ImVec4 circle_inactive = ImColor(24, 24, 30, int(255 * UI_ALPHA));
            inline ImVec4 background = ImColor(27, 27, 34, int(255 * UI_ALPHA));
            inline ImVec4 outline_background = ImColor(255, 255, 255, int(10 * UI_ALPHA));
            inline float   rounding = 6.0f;
        }

        namespace slider {
            inline ImVec4 background = ImColor(27, 27, 34, int(255 * UI_ALPHA));
            inline ImVec4 outline_background = ImColor(255, 255, 255, int(10 * UI_ALPHA));
            inline ImVec4 grabber = ImColor(255, 255, 255, int(255 * UI_ALPHA));
        }

        namespace combo {
            inline ImVec4 background = ImColor(27, 27, 34, int(255 * UI_ALPHA));
            inline ImVec4 outline_background = ImColor(255, 255, 255, int(10 * UI_ALPHA));
        }

        namespace keybind {
            inline ImVec4 background = ImColor(27, 27, 34, int(255 * UI_ALPHA));
            inline ImVec4 outline_background = ImColor(255, 255, 255, int(10 * UI_ALPHA));
        }

        namespace button {
            inline ImVec4 background = ImColor(24, 24, 30, int(255 * UI_ALPHA));
            inline ImVec4 background_light = ImColor(32, 32, 38, int(255 * UI_ALPHA));
            inline ImVec4 outline_background = ImColor(255, 255, 255, int(10 * UI_ALPHA));
        }

        namespace input {
            inline ImVec4 background = ImColor(27, 27, 34, int(255 * UI_ALPHA));
            inline ImVec4 background_light = ImColor(34, 34, 40, int(255 * UI_ALPHA));
            inline ImVec4 outline_background = ImColor(255, 255, 255, int(10 * UI_ALPHA));
        }

        namespace picker {
            inline ImVec4 background = ImColor(27, 27, 34, int(255 * UI_ALPHA));
            inline ImVec4 outline_background = ImColor(255, 255, 255, int(10 * UI_ALPHA));
        }
    };




        inline void UpdateMenuColors() {
            using namespace colors;
            background = ImColor(16, 16, 19, int(255 * UI_ALPHA));
            background_dark = ImColor(15, 15, 18, int(255 * UI_ALPHA));
            background_light = ImColor(20, 20, 24, int(200 * UI_ALPHA));
            outline = ImColor(255, 255, 255, int(10 * UI_ALPHA));
            text = ImColor(224, 224, 229, int(255 * UI_ALPHA));
            text_disabled = ImColor(90, 90, 96, int(255 * UI_ALPHA));

            child::top_background = ImColor(27, 27, 34, int(200 * UI_ALPHA));
            child::outline = ImColor(255, 255, 255, int(10 * UI_ALPHA));

            checkbox::circle_inactive = ImColor(24, 24, 30, int(255 * UI_ALPHA));
            checkbox::background = ImColor(27, 27, 34, int(255 * UI_ALPHA));
            checkbox::outline_background = ImColor(255, 255, 255, int(10 * UI_ALPHA));

            slider::background = ImColor(27, 27, 34, int(255 * UI_ALPHA));
            slider::outline_background = ImColor(255, 255, 255, int(10 * UI_ALPHA));
            slider::grabber = ImColor(255, 255, 255, int(255 * UI_ALPHA));

            combo::background = ImColor(27, 27, 34, int(255 * UI_ALPHA));
            combo::outline_background = ImColor(255, 255, 255, int(10 * UI_ALPHA));

            keybind::background = ImColor(27, 27, 34, int(255 * UI_ALPHA));
            keybind::outline_background = ImColor(255, 255, 255, int(10 * UI_ALPHA));

            button::background = ImColor(24, 24, 30, int(255 * UI_ALPHA));
            button::background_light = ImColor(32, 32, 38, int(255 * UI_ALPHA));
            button::outline_background = ImColor(255, 255, 255, int(10 * UI_ALPHA));

            input::background = ImColor(27, 27, 34, int(255 * UI_ALPHA));
            input::background_light = ImColor(34, 34, 40, int(255 * UI_ALPHA));
            input::outline_background = ImColor(255, 255, 255, int(10 * UI_ALPHA));

            picker::background = ImColor(27, 27, 34, int(255 * UI_ALPHA));
            picker::outline_background = ImColor(255, 255, 255, int(10 * UI_ALPHA));
        }
};

