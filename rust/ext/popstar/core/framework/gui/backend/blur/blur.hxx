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

namespace core::gui
{
	class c_blur
	{
	public:
		~c_blur( ) { shutdown( ); }

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
	o.pos = float4( o.uv.x * 2.0 - 1.0, 1.0 - o.uv.y * 2.0, 0.0, 1.0 );
	return o;
}

Texture2D    src_tex : register( t0 );
SamplerState samp    : register( s0 );
cbuffer BlurCB : register( b0 ) { float2 texel_dir; float2 _pad; };

float4 PSMain( VSOut i ) : SV_Target
{
	// Optimized 5-tap Gaussian (sigma ~1.8), pre-weighted
	float4 c = src_tex.Sample( samp, i.uv ) * 0.2270270270;
	c += src_tex.Sample( samp, i.uv + texel_dir * 1.3846153846 ) * 0.3162162162;
	c += src_tex.Sample( samp, i.uv - texel_dir * 1.3846153846 ) * 0.3162162162;
	c += src_tex.Sample( samp, i.uv + texel_dir * 3.2307692308 ) * 0.0702702703;
	c += src_tex.Sample( samp, i.uv - texel_dir * 3.2307692308 ) * 0.0702702703;
	return c;
}

// Dual Kawase-style downsample (avg 5 taps) for stronger frost at low cost
float4 PSDown( VSOut i ) : SV_Target
{
	float2 half_px = texel_dir;
	float4 c = src_tex.Sample( samp, i.uv ) * 4.0;
	c += src_tex.Sample( samp, i.uv + float2( -half_px.x, -half_px.y ) );
	c += src_tex.Sample( samp, i.uv + float2(  half_px.x, -half_px.y ) );
	c += src_tex.Sample( samp, i.uv + float2( -half_px.x,  half_px.y ) );
	c += src_tex.Sample( samp, i.uv + float2(  half_px.x,  half_px.y ) );
	return c * 0.125;
}
)";

			ID3DBlob* vs_blob = nullptr;
			ID3DBlob* ps_blob = nullptr;
			ID3DBlob* ps_down = nullptr;
			if ( !compile( k_hlsl, "VSMain", "vs_5_0", &vs_blob ) ||
				!compile( k_hlsl, "PSMain", "ps_5_0", &ps_blob ) ||
				!compile( k_hlsl, "PSDown", "ps_5_0", &ps_down ) )
			{
				release( vs_blob );
				release( ps_blob );
				release( ps_down );
				return false;
			}

			const bool ok =
				SUCCEEDED( m_device->CreateVertexShader( vs_blob->GetBufferPointer( ), vs_blob->GetBufferSize( ), nullptr, &m_vs ) ) &&
				SUCCEEDED( m_device->CreatePixelShader( ps_blob->GetBufferPointer( ), ps_blob->GetBufferSize( ), nullptr, &m_ps ) ) &&
				SUCCEEDED( m_device->CreatePixelShader( ps_down->GetBufferPointer( ), ps_down->GetBufferSize( ), nullptr, &m_ps_down ) );

			release( vs_blob );
			release( ps_blob );
			release( ps_down );
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
			bd.ByteWidth = 16;
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			if ( FAILED( m_device->CreateSamplerState( &sd, &m_sampler ) ) ||
				FAILED( m_device->CreateBuffer( &bd, nullptr, &m_cb ) ) )
			{
				shutdown( );
				return false;
			}

			m_ready = true;
			return true;
		}

		void shutdown( )
		{
			release_targets( );
			release( m_bb_copy );
			release( m_bb_srv );
			release( m_cb );
			release( m_sampler );
			release( m_ps_down );
			release( m_ps );
			release( m_vs );
			m_device = nullptr;
			m_context = nullptr;
			m_ready = false;
			m_dirty = true;
			m_frame_ready = false;
		}

		void capture_backbuffer( )
		{
			if ( !m_ready || !g_device || !g_device->m_swap || !m_context )
				return;

			ID3D11Texture2D* back = nullptr;
			if ( FAILED( g_device->m_swap->GetBuffer( 0, IID_PPV_ARGS( &back ) ) ) || !back )
				return;

			D3D11_TEXTURE2D_DESC desc {};
			back->GetDesc( &desc );

			if ( !m_bb_copy || m_bb_w != desc.Width || m_bb_h != desc.Height )
			{
				release( m_bb_srv );
				release( m_bb_copy );
				D3D11_TEXTURE2D_DESC cd = desc;
				cd.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
				cd.MiscFlags = 0;
				cd.CPUAccessFlags = 0;
				cd.Usage = D3D11_USAGE_DEFAULT;
				if ( FAILED( m_device->CreateTexture2D( &cd, nullptr, &m_bb_copy ) ) ||
					FAILED( m_device->CreateShaderResourceView( m_bb_copy, nullptr, &m_bb_srv ) ) )
				{
					release( m_bb_srv );
					release( m_bb_copy );
					back->Release( );
					return;
				}
				m_bb_w = desc.Width;
				m_bb_h = desc.Height;
			}

			m_context->CopyResource( m_bb_copy, back );
			back->Release( );
			m_dirty = true;
			m_frame_ready = false;
		}

		bool process( )
		{
			if ( !m_ready || !m_bb_srv || !m_dirty )
				return m_blur_srv != nullptr;

			const UINT full_w = m_bb_w;
			const UINT full_h = m_bb_h;
			if ( full_w < 2 || full_h < 2 )
				return false;

			const UINT w = full_w;
			const UINT h = full_h;
			if ( !ensure_targets( w, h ) )
				return false;

			D3D11_VIEWPORT old_vp[ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE ] {};
			UINT old_vp_count = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
			m_context->RSGetViewports( &old_vp_count, old_vp );

			ID3D11RenderTargetView* old_rtv = nullptr;
			ID3D11DepthStencilView* old_dsv = nullptr;
			m_context->OMGetRenderTargets( 1, &old_rtv, &old_dsv );

			ID3D11ShaderResourceView* null_srv[ 1 ] { nullptr };
			ID3D11Buffer* null_cb[ 1 ] { nullptr };

			D3D11_VIEWPORT vp {};
			vp.Width = static_cast<float>( w );
			vp.Height = static_cast<float>( h );
			vp.MaxDepth = 1.f;
			m_context->RSSetViewports( 1, &vp );
			m_context->IASetInputLayout( nullptr );
			m_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			m_context->VSSetShader( m_vs, nullptr, 0 );
			m_context->PSSetSamplers( 0, 1, &m_sampler );
			m_context->PSSetConstantBuffers( 0, 1, &m_cb );

			m_context->PSSetShader( m_ps, nullptr, 0 );
			set_dir( 0.f, 0.f );
			draw_pass( m_bb_srv, m_rt_a );

			for ( int i = 0; i < 3; ++i )
			{
				const float spread = 1.f + static_cast<float>( i ) * 0.85f;
				set_dir( spread / static_cast<float>( w ), 0.f );
				draw_pass( m_srv_a, m_rt_b );
				set_dir( 0.f, spread / static_cast<float>( h ) );
				draw_pass( m_srv_b, m_rt_a );
			}

			m_context->PSSetShaderResources( 0, 1, null_srv );
			m_context->PSSetConstantBuffers( 0, 1, null_cb );
			m_context->OMSetRenderTargets( 1, &old_rtv, old_dsv );
			if ( old_vp_count )
				m_context->RSSetViewports( old_vp_count, old_vp );
			release( old_rtv );
			release( old_dsv );

			m_blur_srv = m_srv_a;
			m_dirty = false;
			m_frame_ready = true;
			return true;
		}

		bool ready( ) const { return m_frame_ready && m_blur_srv; }

		void draw_region( ImDrawList* list, c_vector_2d min, c_vector_2d max, float rounding, c_color tint ) const
		{
			if ( !list || !m_blur_srv || !m_frame_ready )
				return;

			const float dw = m_bb_w > 0 ? static_cast<float>( m_bb_w ) : ImGui::GetIO( ).DisplaySize.x;
			const float dh = m_bb_h > 0 ? static_cast<float>( m_bb_h ) : ImGui::GetIO( ).DisplaySize.y;
			if ( dw <= 0.f || dh <= 0.f )
				return;

			const ImVec2 uv0( min.x / dw, min.y / dh );
			const ImVec2 uv1( max.x / dw, max.y / dh );
			const ImU32 col = IM_COL32( tint.r, tint.g, tint.b, tint.a );
			list->AddImageRounded(
				reinterpret_cast<ImTextureID>( m_blur_srv ),
				ImVec2( min.x, min.y ),
				ImVec2( max.x, max.y ),
				uv0,
				uv1,
				col,
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
				"blur.hlsl",
				nullptr,
				nullptr,
				entry,
				profile,
				D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_ENABLE_STRICTNESS,
				0,
				blob,
				&errors );
			if ( errors )
				errors->Release( );
			return SUCCEEDED( hr );
		}

		void release_targets( )
		{
			release( m_srv_a );
			release( m_rt_a );
			release( m_tex_a );
			release( m_srv_b );
			release( m_rt_b );
			release( m_tex_b );
			m_blur_srv = nullptr;
			m_tw = m_th = 0;
		}

		bool ensure_targets( UINT w, UINT h )
		{
			if ( m_tex_a && m_tw == w && m_th == h )
				return true;

			release_targets( );

			D3D11_TEXTURE2D_DESC td {};
			td.Width = w;
			td.Height = h;
			td.MipLevels = 1;
			td.ArraySize = 1;
			td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			td.SampleDesc.Count = 1;
			td.Usage = D3D11_USAGE_DEFAULT;
			td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

			if ( FAILED( m_device->CreateTexture2D( &td, nullptr, &m_tex_a ) ) ||
				FAILED( m_device->CreateRenderTargetView( m_tex_a, nullptr, &m_rt_a ) ) ||
				FAILED( m_device->CreateShaderResourceView( m_tex_a, nullptr, &m_srv_a ) ) ||
				FAILED( m_device->CreateTexture2D( &td, nullptr, &m_tex_b ) ) ||
				FAILED( m_device->CreateRenderTargetView( m_tex_b, nullptr, &m_rt_b ) ) ||
				FAILED( m_device->CreateShaderResourceView( m_tex_b, nullptr, &m_srv_b ) ) )
			{
				release_targets( );
				return false;
			}

			m_tw = w;
			m_th = h;
			return true;
		}

		void set_dir( float x, float y )
		{
			D3D11_MAPPED_SUBRESOURCE mapped {};
			if ( FAILED( m_context->Map( m_cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) ) )
				return;
			float* f = static_cast<float*>( mapped.pData );
			f[0] = x;
			f[1] = y;
			f[2] = 0.f;
			f[3] = 0.f;
			m_context->Unmap( m_cb, 0 );
		}

		void draw_pass( ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output )
		{
			ID3D11ShaderResourceView* null_srv[ 1 ] { nullptr };
			m_context->PSSetShaderResources( 0, 1, null_srv );
			m_context->OMSetRenderTargets( 1, &output, nullptr );
			m_context->PSSetShaderResources( 0, 1, &input );
			m_context->Draw( 3, 0 );
			m_context->PSSetShaderResources( 0, 1, null_srv );
		}

		ID3D11Device* m_device { nullptr };
		ID3D11DeviceContext* m_context { nullptr };
		ID3D11VertexShader* m_vs { nullptr };
		ID3D11PixelShader* m_ps { nullptr };
		ID3D11PixelShader* m_ps_down { nullptr };
		ID3D11SamplerState* m_sampler { nullptr };
		ID3D11Buffer* m_cb { nullptr };

		ID3D11Texture2D* m_bb_copy { nullptr };
		ID3D11ShaderResourceView* m_bb_srv { nullptr };
		UINT m_bb_w { 0 };
		UINT m_bb_h { 0 };

		ID3D11Texture2D* m_tex_a { nullptr };
		ID3D11RenderTargetView* m_rt_a { nullptr };
		ID3D11ShaderResourceView* m_srv_a { nullptr };
		ID3D11Texture2D* m_tex_b { nullptr };
		ID3D11RenderTargetView* m_rt_b { nullptr };
		ID3D11ShaderResourceView* m_srv_b { nullptr };
		ID3D11ShaderResourceView* m_blur_srv { nullptr };
		UINT m_tw { 0 };
		UINT m_th { 0 };

		bool m_ready { false };
		bool m_dirty { true };
		bool m_frame_ready { false };
	};

	inline std::shared_ptr<c_blur> g_blur = std::make_shared<c_blur>( );
	inline std::shared_ptr<c_blur> g_blur_ui = std::make_shared<c_blur>( );
}
