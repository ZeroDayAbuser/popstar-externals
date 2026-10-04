#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <memory>

namespace core::gui
{
	class c_device
	{
	public:
		HWND m_hwnd { nullptr };
		ID3D11Device* m_device { nullptr };
		ID3D11DeviceContext* m_context { nullptr };
		IDXGISwapChain* m_swap { nullptr };
		ID3D11RenderTargetView* m_rtv { nullptr };
		D3D_FEATURE_LEVEL m_feature_level {};
		bool m_borrowed { false };

		bool valid( ) const
		{
			return m_device && m_context && m_rtv && m_hwnd;
		}

		void release_rtv( )
		{
			if ( m_rtv )
			{
				m_rtv->Release( );
				m_rtv = nullptr;
			}
		}

		void create_rtv( )
		{
			ID3D11Texture2D* back_buffer = nullptr;
			if ( FAILED( m_swap->GetBuffer( 0, IID_PPV_ARGS( &back_buffer ) ) ) || !back_buffer )
				return;
			m_device->CreateRenderTargetView( back_buffer, nullptr, &m_rtv );
			back_buffer->Release( );
		}

		bool create( HWND hwnd )
		{
			DXGI_SWAP_CHAIN_DESC sd {};
			sd.BufferCount = 2;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.OutputWindow = hwnd;
			sd.SampleDesc.Count = 1;
			sd.Windowed = TRUE;
			sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

			const D3D_FEATURE_LEVEL levels[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
			UINT flags = 0;
#ifdef _DEBUG
			flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

			HRESULT hr = D3D11CreateDeviceAndSwapChain( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, levels, 2, D3D11_SDK_VERSION, &sd, &m_swap, &m_device, &m_feature_level, &m_context );
			if ( hr == DXGI_ERROR_UNSUPPORTED )
				hr = D3D11CreateDeviceAndSwapChain( nullptr, D3D_DRIVER_TYPE_WARP, nullptr, flags, levels, 2, D3D11_SDK_VERSION, &sd, &m_swap, &m_device, &m_feature_level, &m_context );

			if ( FAILED( hr ) )
				return false;

			m_hwnd = hwnd;
			create_rtv( );
			return valid( );
		}

		void resize( UINT w, UINT h )
		{
			if ( !m_device || !m_swap )
				return;
			release_rtv( );
			m_swap->ResizeBuffers( 0, w, h, DXGI_FORMAT_UNKNOWN, 0 );
			create_rtv( );
		}

		void shutdown( )
		{
			if ( m_borrowed )
			{
				m_hwnd = nullptr;
				m_device = nullptr;
				m_context = nullptr;
				m_swap = nullptr;
				m_rtv = nullptr;
				m_borrowed = false;
				return;
			}
			release_rtv( );
			if ( m_swap ) { m_swap->Release( ); m_swap = nullptr; }
			if ( m_context ) { m_context->Release( ); m_context = nullptr; }
			if ( m_device ) { m_device->Release( ); m_device = nullptr; }
			m_hwnd = nullptr;
		}

		~c_device( ) { shutdown( ); }
	};

	inline std::shared_ptr<c_device> g_device = std::make_shared<c_device>( );
}
