#pragma once

#include <Windows.h>
#include "find-window/window.h"
#include "../../../dependencies/oxorany/oxorany.h"
#include "../../../dependencies/protection/imports/lazy-importer.h"

#include "dwmapi.h"
#include <sstream>
#include <D3DX11.h>

#pragma comment ( lib , "d3d10.lib" )
#pragma comment ( lib , "d3d11.lib" )
#pragma comment ( lib , "d3dx11.lib" )
#pragma comment ( lib , "dwmapi.lib" )

namespace direct_x {
    IDXGISwapChain* p_swapChain = nullptr;
    ID3D11Device* p_device = nullptr;
    ID3D11DeviceContext* p_context = nullptr;
    ID3D11RenderTargetView* p_renderTargetView = nullptr;
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    MSG messager = {};
    HWND my_wnd = nullptr;
    HWND game_wnd = nullptr;
    DWORD processID = 0;
}

struct find_window_data {
    unsigned long pid;
    std::string class_name;
    std::string window_name;
    HWND hwnd;
};

BOOL __stdcall enum_windows_proc ( HWND hwnd , LPARAM l_param ) {
    find_window_data* data = ( find_window_data* ) l_param;

    DWORD pid = 0;
    GetWindowThreadProcessId ( hwnd , &pid );

    if ( pid == data->pid ) {
        char class_name [ 256 ];
        GetClassNameA ( hwnd , class_name , sizeof ( class_name ) );
        if ( data->class_name == class_name ) {
            char window_name [ 256 ];
            GetWindowTextA ( hwnd , window_name , sizeof ( window_name ) );
            if ( data->window_name == window_name ) {
                data->hwnd = hwnd;
                return false;
            }
        }
    }
    return true;
}

HWND find_child_window_from_parent ( HWND parent , const char* class_name , const char* window_name ) {
    DWORD pid = 0;
    GetWindowThreadProcessId ( parent , &pid );

    if ( pid == 0 )
        return nullptr;

    find_window_data data = { pid, class_name, window_name, nullptr };
    EnumWindows ( enum_windows_proc , reinterpret_cast< LPARAM >( &data ) );
    return data.hwnd;
}

RECT get_client_area_and_size ( HWND hwnd ) {
    RECT rect;
    if ( GetClientRect ( hwnd , &rect ) ) {
        POINT top_left = { rect.left, rect.top };
        POINT bottom_right = { rect.right, rect.bottom };

        ClientToScreen ( hwnd , &top_left );
        ClientToScreen ( hwnd , &bottom_right );

        rect.left = top_left.x;
        rect.top = top_left.y;
        rect.right = bottom_right.x;
        rect.bottom = bottom_right.y;
    }
    else {
        rect = { 0, 0, 0, 0 };
    }

    return rect;
}

namespace hijack {

    bool setup ( ) {



        HWND game_hwnd = overlay::find_window ( ( "GameOverlay" ) ,  ( "GameOverlay" ) );

        direct_x::my_wnd = find_child_window_from_parent ( game_hwnd ,  ( "IME" ) ,  ( "Default IME" ) );

        if ( !direct_x::my_wnd ) {
            MessageBoxA ( nullptr , ( "Please Launch SteelSeries Sonar Overlay." ) , ( "[!] Error" ) , MB_OK );
            ExitProcess ( 1 );
            return false;
        }

        RECT rect = get_client_area_and_size ( game_hwnd );
        if ( !SetWindowPos ( direct_x::my_wnd , nullptr , rect.left , rect.top , rect.right - rect.left , rect.bottom - rect.top , SWP_NOZORDER ) )
            return false;

        GetWindowRect ( GetDesktopWindow ( ) , &rect );
        SetWindowLong ( direct_x::my_wnd , GWL_EXSTYLE , WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW );

        MARGINS window_margin { -1 };
        DwmExtendFrameIntoClientArea ( direct_x::my_wnd , &window_margin );
        SetLayeredWindowAttributes ( direct_x::my_wnd , 0 , 255 , LWA_ALPHA );

        UpdateWindow ( direct_x::my_wnd );
        ShowWindow ( direct_x::my_wnd , SW_SHOW );

        logging::print( oxorany ( "hijack setup successful." ) );

        return true;
    }


}

