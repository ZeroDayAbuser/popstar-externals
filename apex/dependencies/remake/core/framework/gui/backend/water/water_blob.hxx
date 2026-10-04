#pragma once

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <imgui.h>

#include <core/framework/gui/backend/render/device.hxx>
#include <core/framework/gui/backend/math/math.hxx>
#include <core/framework/gui/frontend/widgets/settings.hxx>

namespace core::gui 
{
	class c_water_blob
	{
	public:
		~c_water_blob( ) { shutdown( ); }

		bool initialize( )
		{
			if ( !g_device || !g_device->m_device || !g_device->m_context )
				return false;
			if ( m_ready )
				return true;

			m_device = g_device->m_device;
			m_context = g_device->m_context;

			static const char k_hlsl[] = R"(
struct VSOut { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; };

VSOut VSMain( uint id : SV_VertexID )
{
	VSOut o;
	o.uv  = float2( ( id << 1 ) & 2, id & 2 );
	o.pos = float4( o.uv * float2( 2.0, -2.0 ) + float2( -1.0, 1.0 ), 0.0, 1.0 );
	return o;
}

cbuffer WaterCB : register( b0 )
{
	float  time;
	float  strength;
	float2 resolution;
	float4 color_a;
	float4 color_b;
	float4 accent;
	float4 menu; // xy = min uv, zw = size uv (display space)
};

float4 PSMain( VSOut i ) : SV_Target
{
	float2 uv = i.uv;
	float2 local = ( uv - menu.xy ) / max( menu.zw, float2( 1e-4, 1e-4 ) );
	local = saturate( local );

	float t = time;

	// Body rises / falls through the menu
	float base = 0.62
		+ sin( t * 0.85 ) * 0.16
		+ sin( t * 0.42 + 1.4 ) * 0.08;

	// Slosh + soft splash peaks traveling across
	float w1 = sin( local.x * 6.28318 * 1.15 + t * 1.35 ) * 0.065;
	float w2 = sin( local.x * 6.28318 * 2.10 - t * 1.05 + 0.8 ) * 0.040;
	float w3 = sin( local.x * 6.28318 * 0.55 + t * 0.60 + 2.1 ) * 0.085;
	float w4 = sin( local.x * 6.28318 * 3.40 + t * 1.90 ) * 0.018;
	float surface = base + w1 + w2 + w3 + w4;

	float d = local.y - surface;

	float body = 1.0 - smoothstep( -0.015, 0.22, d );
	float crest = exp( -d * d * 120.0 );
	float mist = exp( -max( -d, 0.0 ) * max( -d, 0.0 ) * 28.0 ) * 0.25;

	float mask = saturate( body * 0.70 + crest * 0.95 + mist * 0.40 );

	float depth = saturate( d * 2.4 + 0.4 );
	float3 deep = color_a.rgb;
	float3 lift = color_b.rgb;
	float3 tip  = accent.rgb;

	float3 col = lerp( lift, deep, depth );
	col = lerp( col, tip, crest * 0.38 );
	col = lerp( col, float3( 1.0, 1.0, 1.0 ), crest * 0.12 );

	float alpha = mask * ( 0.20 + body * 0.22 + crest * 0.18 ) * strength;
	return float4( col * alpha, alpha );
}
)";

			ID3DBlob* vs_blob = nullptr;
			ID3DBlob* ps_blob = nullptr;
			if ( !compile( k_hlsl, "VSMain", "vs_5_0", &vs_blob ) ||
				!compile( k_hlsl, "PSMain", "ps_5_0", &ps_blob ) )
			{
				release( vs_blob );
				release( ps_blob );
				return false;
			}

			const bool ok =
				SUCCEEDED( m_device->CreateVertexShader( vs_blob->GetBufferPointer( ), vs_blob->GetBufferSize( ), nullptr, &m_vs ) ) &&
				SUCCEEDED( m_device->CreatePixelShader( ps_blob->GetBufferPointer( ), ps_blob->GetBufferSize( ), nullptr, &m_ps ) );

			release( vs_blob );
			release( ps_blob );
			if ( !ok )
			{
				shutdown( );
				return false;
			}

			D3D11_SAMPLER_DESC sd {};
			sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			sd.MaxLOD = D3D11_FLOAT32_MAX;

			D3D11_BUFFER_DESC bd {};
			bd.ByteWidth = 96; // 6 * float4
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			D3D11_BLEND_DESC blend {};
			blend.RenderTarget[0].BlendEnable = FALSE;
			blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			if ( FAILED( m_device->CreateSamplerState( &sd, &m_sampler ) ) ||
				FAILED( m_device->CreateBuffer( &bd, nullptr, &m_cb ) ) ||
				FAILED( m_device->CreateBlendState( &blend, &m_blend ) ) )
			{
				shutdown( );
				return false;
			}

			m_ready = true;
			return true;
		}

		void shutdown( )
		{
			release_target( );
			release( m_blend );
			release( m_cb );
			release( m_sampler );
			release( m_ps );
			release( m_vs );
			m_device = nullptr;
			m_context = nullptr;
			m_ready = false;
			m_frame_ready = false;
		}

		bool process( float time_sec, float strength, c_vector_2d menu_min, c_vector_2d menu_max )
		{
			if ( !m_ready || !m_context || !g_device )
				return false;

			const ImVec2 disp = ImGui::GetIO( ).DisplaySize;
			const UINT full_w = static_cast<UINT>( ( std::max )( 2.f, disp.x ) );
			const UINT full_h = static_cast<UINT>( ( std::max )( 2.f, disp.y ) );
			const UINT w = ( std::max )( 2u, full_w / 2u );
			const UINT h = ( std::max )( 2u, full_h / 2u );
			if ( !ensure_target( w, h ) )
				return false;

			m_disp_w = full_w;
			m_disp_h = full_h;

			const float dw = static_cast<float>( full_w );
			const float dh = static_cast<float>( full_h );
			const float menu_uv_x = menu_min.x / dw;
			const float menu_uv_y = menu_min.y / dh;
			const float menu_uv_w = ( std::max )( 1.f, menu_max.x - menu_min.x ) / dw;
			const float menu_uv_h = ( std::max )( 1.f, menu_max.y - menu_min.y ) / dh;

			c_color accent = g_style ? g_style->accent : c_color( 90, 170, 210 );
			const c_color deep( 32, 70, 96 );
			const c_color lift( 110, 158, 182 );

			D3D11_MAPPED_SUBRESOURCE mapped {};
			if ( SUCCEEDED( m_context->Map( m_cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) ) )
			{
				float* f = static_cast<float*>( mapped.pData );
				f[0] = time_sec;
				f[1] = ( std::max )( 0.f, ( std::min )( 1.5f, strength ) );
				f[2] = static_cast<float>( w );
				f[3] = static_cast<float>( h );
				f[4] = deep.r / 255.f; f[5] = deep.g / 255.f; f[6] = deep.b / 255.f; f[7] = 1.f;
				f[8] = lift.r / 255.f; f[9] = lift.g / 255.f; f[10] = lift.b / 255.f; f[11] = 1.f;
				f[12] = accent.r / 255.f; f[13] = accent.g / 255.f; f[14] = accent.b / 255.f; f[15] = 1.f;
				f[16] = menu_uv_x; f[17] = menu_uv_y; f[18] = menu_uv_w; f[19] = menu_uv_h;
				m_context->Unmap( m_cb, 0 );
			}

			D3D11_VIEWPORT old_vp[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] {};
			UINT old_vp_count = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
			m_context->RSGetViewports( &old_vp_count, old_vp );

			ID3D11RenderTargetView* old_rtv = nullptr;
			ID3D11DepthStencilView* old_dsv = nullptr;
			m_context->OMGetRenderTargets( 1, &old_rtv, &old_dsv );

			ID3D11BlendState* old_blend = nullptr;
			FLOAT old_factor[4] {};
			UINT old_mask = 0;
			m_context->OMGetBlendState( &old_blend, old_factor, &old_mask );

			D3D11_VIEWPORT vp {};
			vp.Width = static_cast<float>( w );
			vp.Height = static_cast<float>( h );
			vp.MaxDepth = 1.f;
			m_context->RSSetViewports( 1, &vp );
			m_context->IASetInputLayout( nullptr );
			m_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			m_context->VSSetShader( m_vs, nullptr, 0 );
			m_context->PSSetShader( m_ps, nullptr, 0 );
			m_context->PSSetSamplers( 0, 1, &m_sampler );
			m_context->PSSetConstantBuffers( 0, 1, &m_cb );
			m_context->OMSetBlendState( m_blend, nullptr, 0xffffffff );
			m_context->OMSetRenderTargets( 1, &m_rtv, nullptr );

			const float clear[4] { 0.f, 0.f, 0.f, 0.f };
			m_context->ClearRenderTargetView( m_rtv, clear );
			m_context->Draw( 3, 0 );

			ID3D11Buffer* null_cb[1] { nullptr };
			m_context->PSSetConstantBuffers( 0, 1, null_cb );
			m_context->OMSetRenderTargets( 1, &old_rtv, old_dsv );
			m_context->OMSetBlendState( old_blend, old_factor, old_mask );
			if ( old_vp_count )
				m_context->RSSetViewports( old_vp_count, old_vp );
			release( old_rtv );
			release( old_dsv );
			release( old_blend );

			m_frame_ready = true;
			return true;
		}

		bool ready( ) const { return m_frame_ready && m_srv; }

		void draw_region( ImDrawList* list, c_vector_2d min, c_vector_2d max, float rounding, float alpha ) const
		{
			if ( !list || !m_srv || !m_frame_ready )
				return;

			const float dw = m_disp_w > 0 ? static_cast<float>( m_disp_w ) : ImGui::GetIO( ).DisplaySize.x;
			const float dh = m_disp_h > 0 ? static_cast<float>( m_disp_h ) : ImGui::GetIO( ).DisplaySize.y;
			if ( dw <= 0.f || dh <= 0.f )
				return;

			const ImVec2 uv0( min.x / dw, min.y / dh );
			const ImVec2 uv1( max.x / dw, max.y / dh );
			const int a = static_cast<int>( 255.f * ( std::max )( 0.f, ( std::min )( 1.f, alpha ) ) );
			list->AddImageRounded(
				reinterpret_cast<ImTextureID>( m_srv ),
				ImVec2( min.x, min.y ),
				ImVec2( max.x, max.y ),
				uv0,
				uv1,
				IM_COL32( 255, 255, 255, a ),
				rounding );
		}

	private:
		template <typename T>
		static void release( T*& p )
		{
			if ( p )
			{
				p->Release( );
				p = nullptr;
			}
		}

		static bool compile( const char* src, const char* entry, const char* profile, ID3DBlob** blob )
		{
			ID3DBlob* errors = nullptr;
			const HRESULT hr = D3DCompile(
				src,
				std::strlen( src ),
				"water_blob.hlsl",
				nullptr,
				nullptr,
				entry,
				profile,
				D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_ENABLE_STRICTNESS,
				0,
				blob,
				&errors );
			if ( FAILED( hr ) && errors )
			{
				// keep silent in release path; blob stays null
			}
			if ( errors )
				errors->Release( );
			return SUCCEEDED( hr );
		}

		void release_target( )
		{
			release( m_srv );
			release( m_rtv );
			release( m_tex );
			m_tw = m_th = 0;
			m_frame_ready = false;
		}

		bool ensure_target( UINT w, UINT h )
		{
			if ( m_tex && m_tw == w && m_th == h )
				return true;

			release_target( );

			D3D11_TEXTURE2D_DESC td {};
			td.Width = w;
			td.Height = h;
			td.MipLevels = 1;
			td.ArraySize = 1;
			td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			td.SampleDesc.Count = 1;
			td.Usage = D3D11_USAGE_DEFAULT;
			td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

			if ( FAILED( m_device->CreateTexture2D( &td, nullptr, &m_tex ) ) ||
				FAILED( m_device->CreateRenderTargetView( m_tex, nullptr, &m_rtv ) ) ||
				FAILED( m_device->CreateShaderResourceView( m_tex, nullptr, &m_srv ) ) )
			{
				release_target( );
				return false;
			}

			m_tw = w;
			m_th = h;
			return true;
		}

		ID3D11Device* m_device { nullptr };
		ID3D11DeviceContext* m_context { nullptr };
		ID3D11VertexShader* m_vs { nullptr };
		ID3D11PixelShader* m_ps { nullptr };
		ID3D11SamplerState* m_sampler { nullptr };
		ID3D11Buffer* m_cb { nullptr };
		ID3D11BlendState* m_blend { nullptr };

		ID3D11Texture2D* m_tex { nullptr };
		ID3D11RenderTargetView* m_rtv { nullptr };
		ID3D11ShaderResourceView* m_srv { nullptr };
		UINT m_tw { 0 };
		UINT m_th { 0 };
		UINT m_disp_w { 0 };
		UINT m_disp_h { 0 };

		bool m_ready { false };
		bool m_frame_ready { false }; // ayo if the frame isnt ready then dont render bitch ass nigga i hate u nigga deadass u ruined my whole fucking day stupid private member variable
	};

	inline std::shared_ptr<c_water_blob> g_water = std::make_shared<c_water_blob>( );
}
