#pragma once

#include <dependencies/includes.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dwmapi.h>
#include "font/font.h"
#include "font/tahbold.h"
#include "font/smallestpix.h"
#include <src/utility/global/global.cuh>
#include <src/cheat/esp/esp.cuh>
#include <src/cheat/esp/helpers.cuh>
#include <src/cheat/aimbot/aimbot.cuh>
#include <src/cheat/loot/loot.cuh>
#include <src/overlay/hijack.cuh>
#include <dependencies/remake/host.hxx>

inline ID3D11Device * d3d_device = nullptr;
inline ID3D11DeviceContext * d3d_device_ctx = nullptr;
inline IDXGISwapChain * d3d_swap_chain = nullptr;
inline ID3D11RenderTargetView * d3d_render_target = nullptr;
inline HWND window_handle = nullptr;
inline HWND game_hwnd = nullptr;

inline bool show_menu = false;
inline bool insert_was_down = false;
inline HHOOK g_menu_mouse_hook = nullptr;

inline LRESULT CALLBACK menu_mouse_ll( int code , WPARAM wParam , LPARAM lParam )
{
    if ( code == HC_ACTION && show_menu )
    {
        if ( wParam == WM_MOUSEWHEEL )
        {
            const auto* info = reinterpret_cast<MSLLHOOKSTRUCT*>( lParam );
            const float delta = static_cast<float>( GET_WHEEL_DELTA_WPARAM( info->mouseData ) ) / static_cast<float>( WHEEL_DELTA );
            core::gui::push_wheel( delta , 0.f );
        }
        else if ( wParam == WM_MOUSEHWHEEL )
        {
            const auto* info = reinterpret_cast<MSLLHOOKSTRUCT*>( lParam );
            const float delta = static_cast<float>( GET_WHEEL_DELTA_WPARAM( info->mouseData ) ) / static_cast<float>( WHEEL_DELTA );
            core::gui::push_wheel( 0.f , delta );
        }
    }
    return CallNextHookEx( g_menu_mouse_hook , code , wParam , lParam );
}

inline auto set_menu_mouse_hook( bool enabled ) -> void
{
    if ( enabled && !g_menu_mouse_hook )
        g_menu_mouse_hook = SetWindowsHookExW( WH_MOUSE_LL , menu_mouse_ll , GetModuleHandleW( nullptr ) , 0 );
    else if ( !enabled && g_menu_mouse_hook )
    {
        UnhookWindowsHookEx( g_menu_mouse_hook );
        g_menu_mouse_hook = nullptr;
    }
}

extern ImGuiKey ImGui_ImplWin32_KeyEventToImGuiKey( WPARAM wParam , LPARAM lParam );

inline auto feed_imgui_keyboard( ImGuiIO& io ) -> void
{
    static bool key_down[ 256 ] = {};
    static DWORD key_repeat_at[ 256 ] = {};

    const bool ctrl = ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) != 0;
    const bool shift = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;
    const bool alt = ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) != 0;
    const bool super = ( GetAsyncKeyState( VK_LWIN ) & 0x8000 ) != 0 || ( GetAsyncKeyState( VK_RWIN ) & 0x8000 ) != 0;

    io.AddKeyEvent( ImGuiMod_Ctrl , ctrl );
    io.AddKeyEvent( ImGuiMod_Shift , shift );
    io.AddKeyEvent( ImGuiMod_Alt , alt );
    io.AddKeyEvent( ImGuiMod_Super , super );

    BYTE kb[ 256 ];
    for ( int i = 0; i < 256; ++i )
        kb[ i ] = ( GetAsyncKeyState( i ) & 0x8000 ) ? 0x80 : 0;

    const DWORD now = GetTickCount( );

    auto emit_chars = [ & ]( int vk )
    {
        // don't inject chars while ctrl/alt held — kills Ctrl+C / Ctrl+V / Ctrl+A
        if ( ctrl || alt )
            return;

        WCHAR chars[ 8 ] = {};
        const int n = ToUnicode( ( UINT )vk , MapVirtualKeyW( ( UINT )vk , MAPVK_VK_TO_VSC ) , kb , chars , 8 , 0 );
        if ( n <= 0 )
            return;
        for ( int i = 0; i < n; ++i )
        {
            if ( chars[ i ] >= 32 && chars[ i ] != 127 )
                io.AddInputCharacterUTF16( chars[ i ] );
        }
        for ( int i = 0; i < 256; ++i )
            kb[ i ] = ( GetAsyncKeyState( i ) & 0x8000 ) ? 0x80 : 0;
    };

    for ( int vk = 1; vk < 256; ++vk )
    {
        if ( vk == VK_LBUTTON || vk == VK_RBUTTON || vk == VK_MBUTTON ||
             vk == VK_XBUTTON1 || vk == VK_XBUTTON2 || vk == VK_SHIFT ||
             vk == VK_CONTROL || vk == VK_MENU || vk == VK_LWIN || vk == VK_RWIN ||
             vk == VK_PROCESSKEY || vk == VK_PACKET || vk == VK_INSERT )
            continue;

        const bool down = ( GetAsyncKeyState( vk ) & 0x8000 ) != 0;
        if ( down && !key_down[ vk ] )
        {
            const ImGuiKey key = ImGui_ImplWin32_KeyEventToImGuiKey( ( WPARAM )vk , 0 );
            if ( key != ImGuiKey_None )
                io.AddKeyEvent( key , true );
            emit_chars( vk );
            key_repeat_at[ vk ] = now + 400;
        }
        else if ( down && key_down[ vk ] && now >= key_repeat_at[ vk ] )
        {
            emit_chars( vk );
            const ImGuiKey key = ImGui_ImplWin32_KeyEventToImGuiKey( ( WPARAM )vk , 0 );
            if ( key == ImGuiKey_Backspace || key == ImGuiKey_Delete )
            {
                io.AddKeyEvent( key , false );
                io.AddKeyEvent( key , true );
            }
            key_repeat_at[ vk ] = now + 35;
        }
        else if ( !down && key_down[ vk ] )
        {
            const ImGuiKey key = ImGui_ImplWin32_KeyEventToImGuiKey( ( WPARAM )vk , 0 );
            if ( key != ImGuiKey_None )
                io.AddKeyEvent( key , false );
        }
        key_down[ vk ] = down;
    }
}

inline auto color_to_hex( const float* col , char* out , int out_sz ) -> void
{
    ImFormatString( out , out_sz , "#%02X%02X%02X%02X" ,
        ( int )( col[ 0 ] * 255.f + 0.5f ) ,
        ( int )( col[ 1 ] * 255.f + 0.5f ) ,
        ( int )( col[ 2 ] * 255.f + 0.5f ) ,
        ( int )( col[ 3 ] * 255.f + 0.5f ) );
}

inline auto hex_to_color( const char* text , float* col ) -> bool
{
    if ( !text || !text[ 0 ] )
        return false;

    while ( *text == ' ' || *text == '\t' || *text == '\n' || *text == '\r' )
        ++text;

    if ( *text == '#' )
        ++text;

    unsigned int r = 0 , g = 0 , b = 0 , a = 255;
    const int n = sscanf_s( text , "%02x%02x%02x%02x" , &r , &g , &b , &a );
    if ( n < 3 )
    {
        // try without leading zeros / float form
        float fr , fg , fb , fa = 1.f;
        if ( sscanf_s( text , "%f,%f,%f,%f" , &fr , &fg , &fb , &fa ) >= 3 ||
             sscanf_s( text , "(%f,%f,%f,%f)" , &fr , &fg , &fb , &fa ) >= 3 ||
             sscanf_s( text , "%f %f %f %f" , &fr , &fg , &fb , &fa ) >= 3 )
        {
            col[ 0 ] = fr; col[ 1 ] = fg; col[ 2 ] = fb; col[ 3 ] = fa;
            return true;
        }
        return false;
    }
    if ( n == 3 )
        a = 255;

    col[ 0 ] = r / 255.f;
    col[ 1 ] = g / 255.f;
    col[ 2 ] = b / 255.f;
    col[ 3 ] = a / 255.f;
    return true;
}

inline auto set_clipboard_hwnd( const char* text ) -> void
{
    if ( !text )
        return;
    // nullptr owner — hijacked overlay hwnd often fails OpenClipboard
    if ( !OpenClipboard( nullptr ) )
        return;
    EmptyClipboard( );

    const int wide_len = MultiByteToWideChar( CP_UTF8 , 0 , text , -1 , nullptr , 0 );
    if ( wide_len <= 0 )
    {
        CloseClipboard( );
        return;
    }

    HGLOBAL mem = GlobalAlloc( GMEM_MOVEABLE , wide_len * sizeof( wchar_t ) );
    if ( !mem )
    {
        CloseClipboard( );
        return;
    }

    wchar_t* locked = ( wchar_t* )GlobalLock( mem );
    if ( locked )
    {
        MultiByteToWideChar( CP_UTF8 , 0 , text , -1 , locked , wide_len );
        GlobalUnlock( mem );
        SetClipboardData( CF_UNICODETEXT , mem );
    }
    CloseClipboard( );
}

inline auto get_clipboard_hwnd( ) -> const char*
{
    static char buf[ 256 ];
    buf[ 0 ] = 0;
    if ( !OpenClipboard( nullptr ) )
        return buf;

    HANDLE data = GetClipboardData( CF_UNICODETEXT );
    if ( data )
    {
        const wchar_t* text = ( const wchar_t* )GlobalLock( data );
        if ( text )
        {
            WideCharToMultiByte( CP_UTF8 , 0 , text , -1 , buf , ( int )sizeof( buf ) , nullptr , nullptr );
            GlobalUnlock( data );
        }
    }
    else
    {
        data = GetClipboardData( CF_TEXT );
        if ( data )
        {
            const char* text = ( const char* )GlobalLock( data );
            if ( text )
                strncpy_s( buf , text , _TRUNCATE );
            if ( data )
                GlobalUnlock( data );
        }
    }
    CloseClipboard( );
    return buf;
}

inline auto vk_name( int vk , char* out , int out_sz ) -> void
{
	if ( !out || out_sz <= 0 )
		return;
	out[ 0 ] = 0;
	if ( vk <= 0 )
	{
		strncpy_s( out , out_sz , "-" , _TRUNCATE );
		return;
	}

	switch ( vk )
	{
	case VK_LBUTTON:  strncpy_s( out , out_sz , "M1" , _TRUNCATE ); return;
	case VK_RBUTTON:  strncpy_s( out , out_sz , "M2" , _TRUNCATE ); return;
	case VK_MBUTTON:  strncpy_s( out , out_sz , "M3" , _TRUNCATE ); return;
	case VK_XBUTTON1: strncpy_s( out , out_sz , "M4" , _TRUNCATE ); return;
	case VK_XBUTTON2: strncpy_s( out , out_sz , "M5" , _TRUNCATE ); return;
	case VK_SHIFT:    strncpy_s( out , out_sz , "SHIFT" , _TRUNCATE ); return;
	case VK_LSHIFT:   strncpy_s( out , out_sz , "LSHIFT" , _TRUNCATE ); return;
	case VK_RSHIFT:   strncpy_s( out , out_sz , "RSHIFT" , _TRUNCATE ); return;
	case VK_CONTROL:  strncpy_s( out , out_sz , "CTRL" , _TRUNCATE ); return;
	case VK_LCONTROL: strncpy_s( out , out_sz , "LCTRL" , _TRUNCATE ); return;
	case VK_RCONTROL: strncpy_s( out , out_sz , "RCTRL" , _TRUNCATE ); return;
	case VK_MENU:     strncpy_s( out , out_sz , "ALT" , _TRUNCATE ); return;
	case VK_LMENU:    strncpy_s( out , out_sz , "LALT" , _TRUNCATE ); return;
	case VK_RMENU:    strncpy_s( out , out_sz , "RALT" , _TRUNCATE ); return;
	case VK_SPACE:    strncpy_s( out , out_sz , "SPACE" , _TRUNCATE ); return;
	case VK_TAB:      strncpy_s( out , out_sz , "TAB" , _TRUNCATE ); return;
	case VK_CAPITAL:  strncpy_s( out , out_sz , "CAPS" , _TRUNCATE ); return;
	case VK_ESCAPE:   strncpy_s( out , out_sz , "ESC" , _TRUNCATE ); return;
	case VK_RETURN:   strncpy_s( out , out_sz , "ENTER" , _TRUNCATE ); return;
	case VK_BACK:     strncpy_s( out , out_sz , "BACK" , _TRUNCATE ); return;
	case VK_INSERT:   strncpy_s( out , out_sz , "INS" , _TRUNCATE ); return;
	case VK_DELETE:   strncpy_s( out , out_sz , "DEL" , _TRUNCATE ); return;
	case VK_HOME:     strncpy_s( out , out_sz , "HOME" , _TRUNCATE ); return;
	case VK_END:      strncpy_s( out , out_sz , "END" , _TRUNCATE ); return;
	case VK_PRIOR:    strncpy_s( out , out_sz , "PGUP" , _TRUNCATE ); return;
	case VK_NEXT:     strncpy_s( out , out_sz , "PGDN" , _TRUNCATE ); return;
	case VK_LEFT:     strncpy_s( out , out_sz , "LEFT" , _TRUNCATE ); return;
	case VK_RIGHT:    strncpy_s( out , out_sz , "RIGHT" , _TRUNCATE ); return;
	case VK_UP:       strncpy_s( out , out_sz , "UP" , _TRUNCATE ); return;
	case VK_DOWN:     strncpy_s( out , out_sz , "DOWN" , _TRUNCATE ); return;
	default: break;
	}

	const UINT scan = MapVirtualKeyA( ( UINT )vk , MAPVK_VK_TO_VSC );
	if ( scan )
	{
		const LONG lparam = ( LONG )( scan << 16 );
		if ( GetKeyNameTextA( lparam , out , out_sz ) > 0 && out[ 0 ] )
			return;
	}

	if ( vk >= 'A' && vk <= 'Z' )
	{
		out[ 0 ] = ( char )vk;
		out[ 1 ] = 0;
		return;
	}
	if ( vk >= '0' && vk <= '9' )
	{
		out[ 0 ] = ( char )vk;
		out[ 1 ] = 0;
		return;
	}

	ImFormatString( out , out_sz , "0x%02X" , vk );
}

// click → listen for any vk / mouse button. esc cancels.
inline auto keybind_widget( const char* id , int* key ) -> void
{
	if ( !key )
		return;

	static bool listening = false;
	static int* listen_key = nullptr;
	static bool wait_lmb_up = false;

	char name[ 32 ];
	char label[ 48 ];
	if ( listening && listen_key == key )
		ImFormatString( label , IM_ARRAYSIZE( label ) , "[ ... ]##%s" , id );
	else
	{
		vk_name( *key , name , IM_ARRAYSIZE( name ) );
		ImFormatString( label , IM_ARRAYSIZE( label ) , "[ %s ]##%s" , name , id );
	}

	if ( ImGui::Button( label , ImVec2( 72.f , 0.f ) ) && !( listening && listen_key == key ) )
	{
		listening = true;
		listen_key = key;
		wait_lmb_up = true;
	}

	if ( !( listening && listen_key == key ) )
		return;

	if ( wait_lmb_up )
	{
		if ( !( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) )
			wait_lmb_up = false;
		return;
	}

	if ( GetAsyncKeyState( VK_ESCAPE ) & 0x8000 )
	{
		listening = false;
		listen_key = nullptr;
		return;
	}

	for ( int vk = 1; vk < 256; ++vk )
	{
		if ( vk == VK_ESCAPE )
			continue;
		if ( !( GetAsyncKeyState( vk ) & 0x8000 ) )
			continue;

		*key = vk;
		listening = false;
		listen_key = nullptr;
		break;
	}
}

inline auto refresh_game_hwnd( ) -> void
{
    if ( game_hwnd && ::IsWindow( game_hwnd ) )
        return;
    game_hwnd = ::FindWindowA( nullptr , "Apex Legends" );
}

inline auto game_allows_draw( ) -> bool
{
    refresh_game_hwnd( );
    const bool game_alive =
        game_hwnd &&
        ::IsWindow( game_hwnd ) &&
        !::IsIconic( game_hwnd );
    if ( !game_alive )
        return false;

    const HWND foreground = ::GetForegroundWindow( );
    const bool game_focused =
        foreground == game_hwnd ||
        ::GetAncestor( foreground , GA_ROOT ) == game_hwnd;
    const bool overlay_focused =
        window_handle &&
        foreground == window_handle;

    return game_focused || ( show_menu && overlay_focused );
}

inline auto recreate_render_target( ) -> bool
{
	if ( !d3d_swap_chain || !d3d_device )
		return false;

	if ( core::gui::g_device )
		core::gui::g_device->m_rtv = nullptr;

	if ( d3d_render_target )
	{
		d3d_render_target->Release( );
		d3d_render_target = nullptr;
	}

	ID3D11Texture2D* back = nullptr;
	if ( FAILED( d3d_swap_chain->GetBuffer( 0 , IID_PPV_ARGS( &back ) ) ) || !back )
		return false;

	const HRESULT hr = d3d_device->CreateRenderTargetView( back , nullptr , &d3d_render_target );
	back->Release( );
	if ( FAILED( hr ) || !d3d_render_target )
		return false;

	if ( core::gui::g_device )
		core::gui::g_device->m_rtv = d3d_render_target;

	return true;
}

inline auto sync_overlay_to_game( ) -> void
{
	if ( !window_handle || !::IsWindow( window_handle ) )
		return;

	int x = 0 , y = 0 , w = 0 , h = 0;
	if ( !overlay::c_hijack::game_client_bounds( game_hwnd , x , y , w , h ) )
	{
		w = ::GetSystemMetrics( SM_CXSCREEN );
		h = ::GetSystemMetrics( SM_CYSCREEN );
		x = 0;
		y = 0;
	}

	::SetWindowPos( window_handle , HWND_TOPMOST , x , y , w , h ,
		SWP_NOACTIVATE | SWP_SHOWWINDOW );

	static int last_w = 0;
	static int last_h = 0;
	if ( w == last_w && h == last_h )
	{
		data::width_ = w;
		data::height_ = h;
		return;
	}

	last_w = w;
	last_h = h;
	data::width_ = w;
	data::height_ = h;

	if ( !d3d_swap_chain || !d3d_device_ctx )
		return;

	d3d_device_ctx->OMSetRenderTargets( 0 , nullptr , nullptr );

	if ( core::gui::g_device )
		core::gui::g_device->m_rtv = nullptr;

	if ( d3d_render_target )
	{
		d3d_render_target->Release( );
		d3d_render_target = nullptr;
	}

	if ( FAILED( d3d_swap_chain->ResizeBuffers( 0 , static_cast<UINT>( w ) , static_cast<UINT>( h ) , DXGI_FORMAT_UNKNOWN , 0 ) ) )
		return;

	if ( !recreate_render_target( ) )
		return;

	D3D11_VIEWPORT vp {};
	vp.Width = static_cast<float>( w );
	vp.Height = static_cast<float>( h );
	vp.MinDepth = 0.f;
	vp.MaxDepth = 1.f;
	d3d_device_ctx->RSSetViewports( 1 , &vp );

	if ( ImGui::GetCurrentContext( ) )
		ImGui::GetIO( ).DisplaySize = ImVec2( static_cast<float>( w ) , static_cast<float>( h ) );
}

using namespace ImGui;


namespace overlay {
    class c_overlay {
    public:
        auto setup( HWND hwnd ) -> bool
        {
            if ( !hwnd || !::IsWindow( hwnd ) )
                return false;

            window_handle = hwnd;
            return init_imgui( );
        }

        auto init_imgui( ) -> bool
        {
            RECT rc{ };
            GetClientRect( window_handle , &rc );
            UINT w = rc.right  > 0 ? static_cast<UINT>( rc.right  ) : 0;
            UINT h = rc.bottom > 0 ? static_cast<UINT>( rc.bottom ) : 0;
            if ( data::width_ > 0 && data::height_ > 0 )
            {
                w = static_cast<UINT>( data::width_ );
                h = static_cast<UINT>( data::height_ );
            }
            if ( w == 0 ) w = static_cast<UINT>( GetSystemMetrics( SM_CXSCREEN ) );
            if ( h == 0 ) h = static_cast<UINT>( GetSystemMetrics( SM_CYSCREEN ) );
            data::width_ = static_cast<int>( w );
            data::height_ = static_cast<int>( h );

            DXGI_SWAP_CHAIN_DESC desc = {};
            desc.BufferCount = 2;
            desc.BufferDesc.Width = w;
            desc.BufferDesc.Height = h;
            desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.BufferDesc.RefreshRate.Numerator = 60;
            desc.BufferDesc.RefreshRate.Denominator = 1;
            desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            desc.OutputWindow = window_handle;
            desc.SampleDesc.Count = 1;
            desc.Windowed = TRUE;
            desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

            D3D_FEATURE_LEVEL featureLevel;
            const D3D_FEATURE_LEVEL featureArray [ 1 ] = { D3D_FEATURE_LEVEL_11_0 };
            if ( FAILED( D3D11CreateDeviceAndSwapChain( nullptr , D3D_DRIVER_TYPE_HARDWARE , nullptr , 0 ,
                featureArray , 1 , D3D11_SDK_VERSION , &desc ,
                &d3d_swap_chain , &d3d_device , &featureLevel , &d3d_device_ctx ) ) )
                return false;

            ID3D11Texture2D * backBuffer = nullptr;
            d3d_swap_chain->GetBuffer( 0 , IID_PPV_ARGS( &backBuffer ) );
            if ( !backBuffer )
                return false;
            d3d_device->CreateRenderTargetView( backBuffer , NULL , &d3d_render_target );
            backBuffer->Release( );
            if ( !d3d_render_target )
                return false;

            IMGUI_CHECKVERSION( );
            ImGui::CreateContext( );
            ImGuiIO & io = ImGui::GetIO( );
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.DisplaySize = ImVec2( static_cast<float>( w ) , static_cast<float>( h ) );

            ImGuiStyle & style = ImGui::GetStyle( );
            style.AntiAliasedLines = true;
            style.AntiAliasedLinesUseTex = true;
            style.AntiAliasedFill = true;

            ImGui_ImplWin32_Init( window_handle );
            ImGui_ImplDX11_Init( d3d_device , d3d_device_ctx );

            remake_host::init( window_handle , d3d_device , d3d_device_ctx , d3d_swap_chain , d3d_render_target );

            ImFontConfig cfg;
            cfg.FontLoaderFlags = ImGuiFreeTypeBuilderFlags_NoHinting | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor;
            cfg.FontLoaderData = nullptr;

            ImFont * inter = io.Fonts->AddFontFromMemoryTTF( inter_bold_data , sizeof( inter_bold_data ) , 13.3f , &cfg , io.Fonts->GetGlyphRangesJapanese( ) );
            if ( inter )
                io.FontDefault = inter;

            ImFontConfig name_cfg;
            name_cfg.OversampleH = 1;
            name_cfg.OversampleV = 1;
            name_cfg.PixelSnapH = true;
            name_cfg.FontLoaderFlags = ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;
            name_cfg.FontLoaderData = nullptr;
            esp::name_font = io.Fonts->AddFontFromFileTTF(
                "C:\\Windows\\Fonts\\tahoma.ttf" , 13.f , &name_cfg );

            ImFontConfig dist_cfg;
            dist_cfg.FontDataOwnedByAtlas = false;
            dist_cfg.FontLoaderData = nullptr;
            esp::distance_font = io.Fonts->AddFontFromMemoryTTF(
                ( void* )Tahoma_Bold , sizeof( Tahoma_Bold ) , 13.f , &dist_cfg );

            ImFontConfig flag_cfg;
            flag_cfg.FontDataOwnedByAtlas = false;
            flag_cfg.FontLoaderData = nullptr;
            esp::flags_font = io.Fonts->AddFontFromMemoryTTF(
                ( void* )font_sp7 , sizeof( font_sp7 ) , 10.f , &flag_cfg );

            return true;
        }

        auto overlay( ) -> bool
        {
            MSG msg = { NULL };
            while ( msg.message != WM_QUIT )
            {
                while ( PeekMessageA( &msg , NULL , 0 , 0 , PM_REMOVE ) )
                {
                    if ( msg.message == WM_MOUSEWHEEL )
                        ImGui::GetIO( ).AddMouseWheelEvent( 0.f , ( float )GET_WHEEL_DELTA_WPARAM( msg.wParam ) / ( float )WHEEL_DELTA );
                    else if ( msg.message == WM_MOUSEHWHEEL )
                        ImGui::GetIO( ).AddMouseWheelEvent( ( float )GET_WHEEL_DELTA_WPARAM( msg.wParam ) / ( float )WHEEL_DELTA , 0.f );
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                    if ( msg.message == WM_QUIT )
                        break;
                }
                if ( msg.message == WM_QUIT )
                    break;

                sync_overlay_to_game( );

                ImGuiIO & io = ImGui::GetIO( );
                io.DeltaTime = 1.0f / 60.0f;

                POINT p;
                GetCursorPos( &p );
                if ( window_handle )
                    ScreenToClient( window_handle , &p );
                io.MousePos.x = static_cast< float >( p.x );
                io.MousePos.y = static_cast< float >( p.y );
                io.MouseDown [ 0 ] = ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0;

                bool insert_now = ( GetAsyncKeyState( VK_INSERT ) & 0x8000 ) != 0;
                if ( insert_now && !insert_was_down )
                {
                    show_menu = !show_menu;
                    remake_host::set_open( show_menu );
                    set_menu_mouse_hook( show_menu );
                    SetWindowLongA( window_handle , GWL_EXSTYLE , show_menu
                        ? ( WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW )
                        : ( WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED ) );
                    if ( show_menu )
                    {
                        SetForegroundWindow( window_handle );
                        SetFocus( window_handle );
                    }
                }
                insert_was_down = insert_now;

                if ( show_menu )
                    feed_imgui_keyboard( io );

                const bool allow_draw = game_allows_draw( );
                if ( !allow_draw && !show_menu )
                {
                    const float cc[ 4 ] = { 0.f, 0.f, 0.f, 0.f };
                    d3d_device_ctx->OMSetRenderTargets( 1 , &d3d_render_target , nullptr );
                    d3d_device_ctx->ClearRenderTargetView( d3d_render_target , cc );
                    d3d_swap_chain->Present( 1 , 0 );
                    std::this_thread::sleep_for( std::chrono::milliseconds( 16 ) );
                    continue;
                }

                this->draw_frame( );
                d3d_swap_chain->Present( 1 , 0 );
            }

            ImGui_ImplDX11_Shutdown( );
            ImGui_ImplWin32_Shutdown( );
            ImGui::DestroyContext( );
            set_menu_mouse_hook( false );
            return true;
        }

        auto draw_frame( ) -> void
        {
            ImGui_ImplDX11_NewFrame( );
            ImGui_ImplWin32_NewFrame( );
            ImGui::NewFrame( );

            if ( game_allows_draw( ) )
            {
                esp::tick( );
                loot::tick( );
                aimbot::draw( );
            }

            remake_host::set_open( show_menu );
            remake_host::runtime( );
            remake_host::present( d3d_device_ctx , d3d_render_target );
        }
    };
}

inline overlay::c_overlay * g_overlay = new overlay::c_overlay( );
