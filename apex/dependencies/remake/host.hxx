#pragma once

#include <dependencies/remake/includes.hxx>
#include <imgui_internal.h>
#include <filesystem>
#include <string>

namespace remake_host
{
	inline bool g_ready = false;

	inline auto fonts_dir( ) -> std::string
	{
		char mod[ MAX_PATH ] {};
		GetModuleFileNameA( nullptr , mod , MAX_PATH );
		std::filesystem::path p( mod );
		p = p.parent_path( );

		const std::filesystem::path candidates[] = {
			p / "assets" / "fonts" ,
			p / "dependencies" / "remake" / "assets" / "fonts" ,
			std::filesystem::path( "dependencies" ) / "remake" / "assets" / "fonts" ,
		};

		for ( const auto& c : candidates )
		{
			if ( std::filesystem::exists( c ) )
				return c.string( );
		}
		return ( p / "assets" / "fonts" ).string( );
	}

	inline auto init(
		HWND hwnd ,
		ID3D11Device* device ,
		ID3D11DeviceContext* ctx ,
		IDXGISwapChain* swap ,
		ID3D11RenderTargetView* rtv ) -> bool
	{
		using namespace core::gui;

		g_device->bind( hwnd , device , ctx , swap , rtv );
		if ( !g_textures->initialize( device ) )
			return false;

		g_fonts->start( fonts_dir( ) );
		g_blur->initialize( );
		g_blur_ui->initialize( );
		g_water->initialize( );
		g_menu->initialize( );
		g_ready = true;
		return true;
	}

	inline auto set_open( bool open ) -> void
	{
		if ( core::gui::g_ctx )
			core::gui::g_ctx->m_open = open;
	}

	inline auto open( ) -> bool
	{
		return core::gui::g_ctx && core::gui::g_ctx->m_open;
	}

	inline auto runtime( ) -> void
	{
		if ( !g_ready || !core::gui::g_menu )
			return;
		core::gui::g_menu->runtime( );
	}

	inline auto present( ID3D11DeviceContext* ctx , ID3D11RenderTargetView* rtv ) -> void
	{
		using namespace core::gui;

		ImGui::Render( );
		const float clear[ 4 ] = { 0.f , 0.f , 0.f , 0.f };
		ctx->OMSetRenderTargets( 1 , &rtv , nullptr );
		ctx->ClearRenderTargetView( rtv , clear );

		ImDrawData* full = ImGui::GetDrawData( );
		if ( !full || !full->Valid )
			return;

		ImDrawData scene {};
		scene.Valid = true;
		scene.DisplayPos = full->DisplayPos;
		scene.DisplaySize = full->DisplaySize;
		scene.FramebufferScale = full->FramebufferScale;
		scene.OwnerViewport = full->OwnerViewport;
		scene.AddDrawList( ImGui::GetBackgroundDrawList( ) );
		ImGui_ImplDX11_RenderDrawData( &scene );

		ctx->OMSetRenderTargets( 0 , nullptr , nullptr );
		if ( g_blur )
		{
			g_blur->capture_backbuffer( );
			g_blur->process( );
		}
		ctx->OMSetRenderTargets( 1 , &rtv , nullptr );

		ImGui_ImplDX11_RenderDrawData( full );

		if ( g_render && g_render->has_deferred( ) )
		{
			const bool menu_open = g_ctx && g_ctx->m_open;
			const float menu_a = g_ctx ? g_ctx->m_open_anim : 0.f;
			const bool list_only = !menu_open || menu_a < 0.05f;
			const float alpha = list_only ? 1.f : c_render::ease_in_out_quint( menu_a );

			ImDrawList* prev = g_render->draw_list( );

			for ( int pass = 0; pass < 6 && g_render->has_deferred( ); ++pass )
			{
				ctx->OMSetRenderTargets( 0 , nullptr , nullptr );
				if ( g_blur_ui )
				{
					g_blur_ui->capture_backbuffer( );
					g_blur_ui->process( );
				}
				ctx->OMSetRenderTargets( 1 , &rtv , nullptr );

				ImDrawListSharedData* shared = ImGui::GetDrawListSharedData( );
				ImDrawList temp( shared );
				temp._ResetForNewFrame( );
				temp.PushClipRectFullScreen( );
				temp.Flags |= ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedFill;
				if ( ImFontAtlas* atlas = ImGui::GetIO( ).Fonts )
					temp.PushTexture( atlas->TexRef );
				g_render->set_draw_list( &temp );
				g_render->flush_deferred_once( );
				g_render->set_draw_list( prev );

				if ( temp.VtxBuffer.Size > 0 )
				{
					for ( int i = 0; i < temp.VtxBuffer.Size; ++i )
					{
						ImDrawVert& v = temp.VtxBuffer.Data[ i ];
						const ImU32 a = ( v.col >> IM_COL32_A_SHIFT ) & 0xff;
						const ImU32 na = static_cast<ImU32>( a * alpha );
						v.col = ( v.col & ~IM_COL32_A_MASK ) | ( na << IM_COL32_A_SHIFT );
					}

					ImDrawData modal {};
					modal.Valid = true;
					modal.DisplayPos = full->DisplayPos;
					modal.DisplaySize = full->DisplaySize;
					modal.FramebufferScale = full->FramebufferScale;
					modal.OwnerViewport = full->OwnerViewport;
					modal.AddDrawList( &temp );
					ImGui_ImplDX11_RenderDrawData( &modal );
				}
			}
			g_render->clear_deferred( );
		}
	}
}
