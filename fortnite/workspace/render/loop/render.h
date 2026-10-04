#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include "../overlay/overlay.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <algorithm>
#include <imgui_settings.h>
#include <vector>
#include "../menu/menu.h"
#include <gui.h>

#include <winhttp.h>
#include "../../core/unreal-engine/settings/settings.h"
#include "../../core/unreal-engine/sdk/sdk.h"
#include "../../core/unreal-engine/actor/loop.h"
#include "../../../impl/controller/detection.h"

#pragma comment(lib, "winhttp.lib")

ImFont* JetBrains = nullptr;


std::vector<uint8_t> DownloadToMemory ( const std::wstring& url )
{
	std::vector<uint8_t> buffer;

	URL_COMPONENTS urlComp {};
	urlComp.dwStructSize = sizeof ( urlComp );

	wchar_t hostName [ 256 ];
	wchar_t urlPath [ 1024 ];

	urlComp.lpszHostName = hostName;
	urlComp.dwHostNameLength = _countof ( hostName );

	urlComp.lpszUrlPath = urlPath;
	urlComp.dwUrlPathLength = _countof ( urlPath );

	if ( !WinHttpCrackUrl ( url.c_str ( ) , 0 , 0 , &urlComp ) )
		return buffer;

	HINTERNET hSession = WinHttpOpen ( oxorany ( L"imgui-font-loader/1.0" ) ,
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY ,
		WINHTTP_NO_PROXY_NAME ,
		WINHTTP_NO_PROXY_BYPASS , 0 );

	if ( !hSession )
		return buffer;

	HINTERNET hConnect = WinHttpConnect ( hSession ,
		std::wstring ( urlComp.lpszHostName , urlComp.dwHostNameLength ).c_str ( ) ,
		urlComp.nPort , 0 );

	if ( !hConnect )
	{
		WinHttpCloseHandle ( hSession );
		return buffer;
	}

	HINTERNET hRequest = WinHttpOpenRequest ( hConnect , L"GET" ,
		std::wstring ( urlComp.lpszUrlPath , urlComp.dwUrlPathLength ).c_str ( ) ,
		nullptr , WINHTTP_NO_REFERER ,
		WINHTTP_DEFAULT_ACCEPT_TYPES ,
		( urlComp.nScheme == INTERNET_SCHEME_HTTPS ) ? WINHTTP_FLAG_SECURE : 0 );

	if ( !hRequest )
	{
		WinHttpCloseHandle ( hConnect );
		WinHttpCloseHandle ( hSession );
		return buffer;
	}

	BOOL bResults = WinHttpSendRequest ( hRequest ,
		WINHTTP_NO_ADDITIONAL_HEADERS , 0 ,
		WINHTTP_NO_REQUEST_DATA , 0 ,
		0 , 0 );

	if ( bResults )
		bResults = WinHttpReceiveResponse ( hRequest , nullptr );

	if ( bResults )
	{
		DWORD dwSize = 0;
		do
		{
			DWORD dwDownloaded = 0;
			if ( !WinHttpQueryDataAvailable ( hRequest , &dwSize ) )
				break;

			if ( dwSize == 0 )
				break;

			std::vector<uint8_t> temp ( dwSize );

			if ( !WinHttpReadData ( hRequest , temp.data ( ) , dwSize , &dwDownloaded ) )
				break;

			buffer.insert ( buffer.end ( ) , temp.begin ( ) , temp.begin ( ) + dwDownloaded );

		} while ( dwSize > 0 );
	}

	WinHttpCloseHandle ( hRequest );
	WinHttpCloseHandle ( hConnect );
	WinHttpCloseHandle ( hSession );

	return buffer;
}

namespace render {


	inline int width = GetSystemMetrics ( SM_CXSCREEN );
	inline int height = GetSystemMetrics ( SM_CYSCREEN );
	inline HHOOK mouse_hook = nullptr;
	inline volatile LONG mouse_wheel_steps = 0;

	inline LRESULT CALLBACK mouse_proc ( int nCode , WPARAM wParam , LPARAM lParam ) {
		if ( nCode == HC_ACTION && wParam == WM_MOUSEWHEEL ) {
			const auto* mouse_info = reinterpret_cast< const MSLLHOOKSTRUCT* >( lParam );
			const SHORT delta = GET_WHEEL_DELTA_WPARAM ( mouse_info->mouseData );
			if ( delta != 0 ) {
				InterlockedAdd ( &mouse_wheel_steps , delta / WHEEL_DELTA );
			}
		}
		return CallNextHookEx ( mouse_hook , nCode , wParam , lParam );
	}

	bool setup ( ) {
		ImGui_ImplWin32_EnableDpiAwareness ( );

		ZeroMemory ( &direct_x::swapChainDesc , sizeof ( direct_x::swapChainDesc ) );
		direct_x::swapChainDesc.BufferCount = 1;
		direct_x::swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		direct_x::swapChainDesc.BufferDesc.Width = width;
		direct_x::swapChainDesc.BufferDesc.Height = height;
		direct_x::swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		direct_x::swapChainDesc.OutputWindow = direct_x::my_wnd;
		direct_x::swapChainDesc.SampleDesc.Count = 1;
		direct_x::swapChainDesc.Windowed = TRUE;
		direct_x::swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		D3D_FEATURE_LEVEL featureLevel;
		const D3D_FEATURE_LEVEL featureLevelArray [ 1 ] = { D3D_FEATURE_LEVEL_11_0 };

		if ( FAILED ( D3D11CreateDeviceAndSwapChain (
			nullptr ,
			D3D_DRIVER_TYPE_HARDWARE ,
			nullptr ,
			0 ,
			featureLevelArray ,
			1 ,
			D3D11_SDK_VERSION ,
			&direct_x::swapChainDesc ,
			&direct_x::p_swapChain ,
			&direct_x::p_device ,
			&featureLevel ,
			&direct_x::p_context ) ) )
		{
			exit ( 4 );
		}

		ID3D11Texture2D* pBackBuffer = nullptr;
		direct_x::p_swapChain->GetBuffer ( 0 , __uuidof( ID3D11Texture2D ) , ( LPVOID* ) &pBackBuffer );
		direct_x::p_device->CreateRenderTargetView ( pBackBuffer , NULL , &direct_x::p_renderTargetView );
		pBackBuffer->Release ( );

		direct_x::p_context->OMSetRenderTargets ( 1 , &direct_x::p_renderTargetView , NULL );

		ImGui::CreateContext ( );

		ImGuiStyle* s = &ImGui::GetStyle ( );
		s->WindowPadding = ImVec2 ( 0 , 0 ) , s->WindowBorderSize = 0;
		s->ItemSpacing = ImVec2 ( 20 , 20 );

		s->ScrollbarSize = 4.f;


		ui::initialize_fonts ( );
		ui::initialize_images ( );
		ui::initialize_tabs ( );

		ImGui_ImplWin32_Init ( direct_x::my_wnd );
		ImGuiIO& io = ImGui::GetIO ( ); ( void ) io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.IniFilename = nullptr;
		ImFontConfig cfg;

		ImGui_ImplDX11_Init ( direct_x::p_device , direct_x::p_context );

		mouse_hook = SetWindowsHookExA ( WH_MOUSE_LL , mouse_proc , GetModuleHandleA ( nullptr ) , 0 );

		static std::vector<std::vector<uint8_t>> g_FontBuffers;

		auto load_font = [ & ] ( const wchar_t* url , float size )
			{
				auto& data = g_FontBuffers.emplace_back ( DownloadToMemory ( url ) );

				if ( data.empty ( ) )
					return ( ImFont* )nullptr;

				return io.Fonts->AddFontFromMemoryTTF (
					data.data ( ) ,
					data.size ( ) ,
					size ,
					nullptr ,
					io.Fonts->GetGlyphRangesCyrillic ( )
				);
			};

		JetBrains = load_font ( oxorany ( L"https://pub-7b0bbaf34e7c423d9b59792711ce4735.r2.dev/font/JetBrainsMono-Bold.ttf" ), 14.0f );


		return S_OK;

	}


	bool loop ( ) {

		static RECT old_rc = {};
		static bool menu_open = true;
		static bool insert_was_down = false;
		ZeroMemory ( &direct_x::messager , sizeof ( MSG ) );

		while ( direct_x::messager.message != WM_QUIT )
		{

			while ( PeekMessage ( &direct_x::messager , direct_x::my_wnd , 0 , 0 , PM_REMOVE ) )
			{
				TranslateMessage ( &direct_x::messager );
				DispatchMessage ( &direct_x::messager );
			}

			HWND foreground = GetForegroundWindow ( );
			bool game_active = ( foreground == direct_x::game_wnd );

			if ( !game_active ) {
				ShowWindow ( direct_x::my_wnd , SW_HIDE );
				if ( mouse_hook ) {
					UnhookWindowsHookEx ( mouse_hook );
					mouse_hook = nullptr;
					InterlockedExchange ( &mouse_wheel_steps , 0 );
				}
			}
			else {

				ShowWindow ( direct_x::my_wnd , SW_SHOW );
				if ( menu_open && !mouse_hook ) {
					mouse_hook = SetWindowsHookExA ( WH_MOUSE_LL , mouse_proc , GetModuleHandleA ( nullptr ) , 0 );
				}

			}

			RECT rc = {};
			rc.left = 0;
			rc.top = 0;
			rc.right = GetSystemMetrics ( SM_CXSCREEN );
			rc.bottom = GetSystemMetrics ( SM_CYSCREEN );
			POINT xy = { 0, 0 };

			ImGuiIO& io = ImGui::GetIO ( );
			static LARGE_INTEGER freq = {} , last = {};
			if ( freq.QuadPart == 0 ) QueryPerformanceFrequency ( &freq );
			LARGE_INTEGER now; QueryPerformanceCounter ( &now );
			if ( last.QuadPart != 0 ) {
				io.DeltaTime = ( float ) ( now.QuadPart - last.QuadPart ) / ( float ) freq.QuadPart;
				io.DeltaTime = std::clamp ( io.DeltaTime , 0.0001f , 0.5f );
			}
			else io.DeltaTime = 1.0f / 60.0f;
			last = now;

			POINT p;
			GetCursorPos ( &p );
			io.MousePos.x = static_cast< float >( p.x );
			io.MousePos.y = static_cast< float >( p.y );

			if ( GetAsyncKeyState ( VK_LBUTTON ) )
			{
				io.MouseDown [ 0 ] = true;
				io.MouseClicked [ 0 ] = true;
				io.MouseClickedPos [ 0 ].x = io.MousePos.x;
				io.MouseClickedPos [ 0 ].y = io.MousePos.y;
			}
			else
			{
				io.MouseDown [ 0 ] = false;
			}

			const bool insert_is_down = ( GetAsyncKeyState ( VK_INSERT ) & 0x8000 ) != 0;
			if ( insert_is_down && !insert_was_down ) {
				menu_open = !menu_open;
				if ( !menu_open && mouse_hook ) {
					UnhookWindowsHookEx ( mouse_hook );
					mouse_hook = nullptr;
					InterlockedExchange ( &mouse_wheel_steps , 0 );
				}
				else if ( menu_open && game_active && !mouse_hook ) {
					mouse_hook = SetWindowsHookExA ( WH_MOUSE_LL , mouse_proc , GetModuleHandleA ( nullptr ) , 0 );
				}
			}
			insert_was_down = insert_is_down;

			const LONG wheel_steps = InterlockedExchange ( &mouse_wheel_steps , 0 );
			if ( wheel_steps != 0 ) {
				io.MouseWheel += static_cast< float >( wheel_steps );
			}


			if ( rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom )
			{
				old_rc = rc;
				width = rc.right;
				height = rc.bottom;

				if ( direct_x::p_swapChain )
				{
					if ( direct_x::p_renderTargetView )
					{
						direct_x::p_renderTargetView->Release ( );
						direct_x::p_renderTargetView = nullptr;
					}

					HRESULT hr = direct_x::p_swapChain->ResizeBuffers ( 0 , width , height , DXGI_FORMAT_UNKNOWN , 0 );

					if ( FAILED ( hr ) )
						exit ( 5 );

					ID3D11Texture2D* pBackBuffer = nullptr;
					hr = direct_x::p_swapChain->GetBuffer ( 0 , __uuidof( ID3D11Texture2D ) , ( LPVOID* ) &pBackBuffer );
					if ( SUCCEEDED ( hr ) )
					{
						direct_x::p_device->CreateRenderTargetView ( pBackBuffer , nullptr , &direct_x::p_renderTargetView );
						pBackBuffer->Release ( );
					}

					SetWindowPos ( direct_x::my_wnd , ( HWND ) 0 , xy.x , xy.y , width , height , SWP_NOREDRAW );
				}
			}

			ImGui_ImplDX11_NewFrame ( );
			ImGui_ImplWin32_NewFrame ( );

			ImGui::NewFrame ( );

			detection::find_controllers ( io.DeltaTime );


			if ( g_aimbot::show_fov )
			{
				auto drawList = ImGui::GetBackgroundDrawList ( );

				ImVec2 display = ImGui::GetIO ( ).DisplaySize;
				ImVec2 center ( display.x * 0.5f , display.y * 0.5f );

				float finalFov = g_aimbot::fov * ( 90.0f / ViewPoint::FieldOfView );

				int dynamicSegments = std::clamp ( static_cast< int >( finalFov * 1.5f ) , 50 , 256	);

				drawList->AddCircle ( center , finalFov , IM_COL32 ( 255 , 255 , 255 , 255 ) , dynamicSegments , 1.0f );
			}

			if ( menu_open )
				menu::show ( );

			ui::show_fps ( );

			ImGui::PushFont ( JetBrains );
			game::loop ( );
			ImGui::PopFont ( );

			ImGui::EndFrame ( );

			if ( direct_x::p_context && direct_x::p_renderTargetView )
			{
				FLOAT clearColor [ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
				direct_x::p_context->ClearRenderTargetView ( direct_x::p_renderTargetView , clearColor );
				direct_x::p_context->OMSetRenderTargets ( 1 , &direct_x::p_renderTargetView , nullptr );

				D3D11_VIEWPORT vp;
				vp.TopLeftX = 0;
				vp.TopLeftY = 0;
				vp.Width = static_cast< FLOAT >( width );
				vp.Height = static_cast< FLOAT >( height );
				vp.MinDepth = 0.0f;
				vp.MaxDepth = 1.0f;
				direct_x::p_context->RSSetViewports ( 1 , &vp );

				ImGui::Render ( );
				ImGui_ImplDX11_RenderDrawData ( ImGui::GetDrawData ( ) );
			}

			const UINT present_interval = g_settings::vsync ? 1u : 0u;
			HRESULT result = direct_x::p_swapChain->Present ( present_interval , 0 );
			if ( FAILED ( result ) )
			{
				if ( result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET )
					break;
			}
		}

		ImGui_ImplDX11_Shutdown ( );
		ImGui_ImplWin32_Shutdown ( );
		ImGui::DestroyContext ( );

		if ( mouse_hook ) {
			UnhookWindowsHookEx ( mouse_hook );
			mouse_hook = nullptr;
		}

		if ( direct_x::p_renderTargetView ) { direct_x::p_renderTargetView->Release ( ); direct_x::p_renderTargetView = nullptr; }
		if ( direct_x::p_swapChain ) { direct_x::p_swapChain->Release ( ); direct_x::p_swapChain = nullptr; }
		if ( direct_x::p_context ) { direct_x::p_context->Release ( ); direct_x::p_context = nullptr; }
		if ( direct_x::p_device ) { direct_x::p_device->Release ( ); direct_x::p_device = nullptr; }

		DestroyWindow ( direct_x::my_wnd );
		return direct_x::messager.wParam;

	}

}
