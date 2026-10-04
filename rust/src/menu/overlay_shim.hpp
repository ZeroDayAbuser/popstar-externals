#pragma once


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <d3d11.h>

#include <app/backends/directx11/directx11.hpp>

namespace Overlay
{
    inline ID3D11Device*&        g_pd3dDevice        = winapp::backends::directx11::impl::device;
    inline ID3D11DeviceContext*& g_pd3dDeviceContext = winapp::backends::directx11::impl::device_context;
    inline IDXGISwapChain1*&     g_pSwapChain        = winapp::backends::directx11::impl::swap_chain;
}
