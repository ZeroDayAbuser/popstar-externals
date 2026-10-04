#include <Cheat.hpp>
#include "OverlayBridge.hpp"

#include <core/framework/gui/frontend/menu/menu.hxx>
#include <core/framework/gui/backend/render/device.hxx>
#include <core/framework/gui/backend/manager/fonts/fonts.hxx>
#include <core/framework/gui/backend/manager/textures/textures.hxx>
#include <core/framework/gui/backend/blur/blur.hxx>
#include <core/framework/gui/backend/water/water_blob.hxx>
#include <core/framework/gui/frontend/widgets/classes/window.hxx>
#include <settings/settings.hpp>
#include <app/winapp.hpp>
#include <app/backends/directx11/directx11.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_dx11.h>
#include <utils/debug.hpp>

namespace OverlayBridge {

    static bool s_booted = false;

    void Bootstrap(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
    {
        if (s_booted)
            return;

        DBG("[popstar] bootstrap hwnd={} device={} ctx={} swap={} rtv={}",
            static_cast<void*>(hwnd),
            static_cast<void*>(device),
            static_cast<void*>(context),
            static_cast<void*>(winapp::backends::directx11::impl::swap_chain),
            static_cast<void*>(winapp::backends::directx11::impl::render_target));

        auto& d = *core::gui::g_device;
        d.m_hwnd = hwnd;
        d.m_device = device;
        d.m_context = context;
        d.m_swap = winapp::backends::directx11::impl::swap_chain;
        d.m_rtv = winapp::backends::directx11::impl::render_target;
        d.m_borrowed = true;

        const bool fonts_ok = core::gui::g_fonts->start();
        DBG("[popstar] fonts: {}", fonts_ok ? "ok" : "FAILED (default atlas)");
        if (device)
        {
            const bool tex_ok = core::gui::g_textures->initialize(device);
            DBG("[popstar] textures: {}", tex_ok ? "ok" : "FAILED");
        }
        DBG("[popstar] blur: {}", core::gui::g_blur->initialize() ? "ok" : "FAILED");
        DBG("[popstar] blur_ui: {}", core::gui::g_blur_ui->initialize() ? "ok" : "FAILED");
        DBG("[popstar] water: {}", core::gui::g_water->initialize() ? "ok" : "FAILED");
        settings_io::Init();
        settings_io::LoadDefault();
        core::gui::g_menu->initialize();
        DBG("[popstar] menu windows: {}", core::gui::g_menu->windows().size());
        DBG("[popstar] INSERT toggles the menu");
        s_booted = true;
    }

    void PollInput() {}

    bool OnWndProc(UINT, WPARAM, LPARAM) { return false; }

    void Render()
    {
        if (!s_booted)
            return;
        core::gui::g_device->m_rtv = winapp::backends::directx11::impl::render_target;
        if (core::gui::g_blur)
        {
            core::gui::g_blur->capture_backbuffer();
            core::gui::g_blur->process();
        }
        core::gui::g_menu->runtime();
    }

    void FinishFrame()
    {
        if (!s_booted || !core::gui::g_render)
            return;

        if (core::gui::g_blur_ui)
        {
            core::gui::g_blur_ui->capture_backbuffer();
            core::gui::g_blur_ui->process();
        }

        if (!core::gui::g_render->has_deferred())
            return;

        const bool menu_open = core::gui::g_ctx && core::gui::g_ctx->m_open;
        const float menu_a = core::gui::g_ctx ? core::gui::g_ctx->m_open_anim : 0.f;
        if (!menu_open || menu_a < 0.05f)
        {
            core::gui::g_render->clear_deferred();
            return;
        }

        ImGuiIO& io = ImGui::GetIO();
        ImDrawData* full = ImGui::GetDrawData();
        if (!full || !full->Valid)
        {
            core::gui::g_render->clear_deferred();
            return;
        }

        ImDrawList* prev = core::gui::g_render->draw_list();
        const float alpha = core::gui::c_render::ease_in_out_quint(menu_a);
        auto* ctx = winapp::backends::directx11::impl::device_context;
        auto* rtv = winapp::backends::directx11::impl::render_target;

        for (int pass = 0; pass < 6 && core::gui::g_render->has_deferred(); ++pass)
        {
            if (core::gui::g_blur_ui)
            {
                core::gui::g_blur_ui->capture_backbuffer();
                core::gui::g_blur_ui->process();
            }
            if (ctx && rtv)
                ctx->OMSetRenderTargets(1, &rtv, nullptr);

            ImDrawList modal_dl(ImGui::GetDrawListSharedData());
            modal_dl._ResetForNewFrame();
            modal_dl.PushClipRectFullScreen();
            if (io.Fonts)
                modal_dl.PushTexture(io.Fonts->TexRef);

            core::gui::g_render->set_draw_list(&modal_dl);
            core::gui::g_render->set_alpha(alpha);
            core::gui::g_render->flush_deferred_once();
            core::gui::g_render->set_alpha(1.f);

            if (io.Fonts)
                modal_dl.PopTexture();

            if (modal_dl.VtxBuffer.Size > 0)
            {
                ImDrawData modal{};
                modal.Valid = true;
                modal.DisplayPos = full->DisplayPos;
                modal.DisplaySize = full->DisplaySize;
                modal.FramebufferScale = full->FramebufferScale;
                modal.OwnerViewport = full->OwnerViewport;
                modal.AddDrawList(&modal_dl);
                ImGui_ImplDX11_RenderDrawData(&modal);
            }
        }

        core::gui::g_render->set_draw_list(prev);
        core::gui::g_render->clear_deferred();
    }

    void Shutdown()
    {
        if (core::gui::g_device)
            core::gui::g_device->shutdown();
        s_booted = false;
    }

    bool IsMenuOpen()
    {
        return core::gui::g_ctx && core::gui::g_ctx->m_open;
    }

    ShapeRect CurrentShape()
    {
        if (!IsMenuOpen() || core::gui::g_menu->windows().empty())
            return ShapeRect{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        auto& w = core::gui::g_menu->windows().front();
        if (!w)
            return ShapeRect{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        const auto p = w->pos();
        const auto s = w->size();
        return ShapeRect{ p.x, p.y, s.x, s.y, 8.0f };
    }

}
