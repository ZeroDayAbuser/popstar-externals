#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>
#include <vector>

#include <imgui.h>

#include <core/framework/gui/frontend/widgets/classes/text_input.hxx>
#include <core/framework/gui/frontend/widgets/classes/context.hxx>
#include <core/framework/gui/frontend/widgets/settings.hxx>
#include <core/framework/gui/backend/render/render.hxx>
#include <core/framework/gui/backend/inputs/inputs.hxx>

namespace core::gui
{
	class c_config
	{
	public:
		void paint( c_vector_2d box_pos, c_vector_2d box_size )
		{
			if ( !g_render || !g_style )
				return;

			m_box = box_pos;
			m_box_size = box_size;

			const bool hovered = g_input && g_input->mouse_in_region( m_box, m_box_size );
			const float dt = ImGui::GetIO( ).DeltaTime;

			m_hover += ( ( hovered || m_open ? 1.f : 0.f ) - m_hover ) * ( 1.f - std::exp( -20.f * dt ) );
			m_anim += ( ( m_open ? 1.f : 0.f ) - m_anim ) * ( 1.f - std::exp( -18.f * dt ) );
			if ( std::fabs( m_anim - ( m_open ? 1.f : 0.f ) ) < 0.0008f )
				m_anim = m_open ? 1.f : 0.f;

			const float he = c_render::ease_out_cubic( m_hover );
			const float oe = c_render::ease_out_cubic( m_anim );
			const c_color fill = c_color( 17, 19, 27 )
				.lerp( c_color( 26, 31, 44 ), he * 0.9f )
				.lerp( c_color( 24, 30, 46 ), oe * 0.55f );
			const c_color border = c_color( 28, 31, 41 )
				.lerp( g_style->accent, he * 0.55f + oe * 0.45f );
			g_render->rect_filled( static_cast<int>( m_box.x ), static_cast<int>( m_box.y ), static_cast<int>( m_box_size.x ), static_cast<int>( m_box_size.y ), fill, 6.f );
			g_render->rect( static_cast<int>( m_box.x ), static_cast<int>( m_box.y ), static_cast<int>( m_box_size.x ), static_cast<int>( m_box_size.y ), border, 6.f );

			const c_color icon_col = c_color( 194, 197, 206 ).lerp( g_style->accent, he * 0.35f + oe * 0.55f );
			g_render->text( c_fonts::k_icons_key, c_vector_2d( m_box.x + 12.f, m_box.y + 8.f ), icon_col, "\xef\x83\x87", 13.f );

			g_render->line(
				c_vector_2d( m_box.x + 34.f, m_box.y + 8.f ),
				c_vector_2d( m_box.x + 34.f, m_box.y + 22.f ),
				c_color( 48, 52, 64, 190 ).lerp( g_style->accent.with_alpha( 120 ), oe * 0.4f ),
				1.f );

			const std::string cfg_name = ( m_index >= 0 && m_index < static_cast<int>( m_configs.size( ) ) )
				? m_configs[static_cast<std::size_t>( m_index )]
				: "config";
			c_text_input::morph_string( m_chip_label, m_chip_anim, cfg_name, g_style->text_control );
			c_text_input::tick_text_anims( m_chip_anim, dt );

			const float label_x = m_box.x + 44.f;
			const float label_y = m_box.y + 7.f;
			const float label_w = 100.f;
			const c_color label_col = c_color( 184, 187, 197 ).lerp( g_style->text_bright, he * 0.45f + oe * 0.35f );
			if ( ImDrawList* cdl = g_render->draw_list( ) )
			{
				cdl->PushClipRect(
					ImVec2( label_x - 1.f, m_box.y + 2.f ),
					ImVec2( label_x + label_w, m_box.y + 28.f ),
					true );
				c_text_input::paint_text_chars(
					m_chip_label,
					m_chip_anim,
					label_x,
					label_y,
					g_style->text_control,
					label_col );
				cdl->PopClipRect( );
			}

			const float ax = m_box.x + 152.f;
			const float ay = m_box.y + 15.f;
			const c_color chev_col = c_color( 130, 135, 146 ).lerp( g_style->accent, he * 0.4f + oe * 0.5f );
			const float chev_up = oe;
			const float chev_dn = 1.f - oe;
			if ( chev_dn > 0.02f )
				g_render->text( c_fonts::k_icons_key, c_vector_2d( ax - 4.f, ay - 5.f + chev_up * 3.f ), chev_col.with_alpha( static_cast<int>( 255.f * chev_dn ) ), "\xef\x81\xb8", 10.f );
			if ( chev_up > 0.02f )
				g_render->text( c_fonts::k_icons_key, c_vector_2d( ax - 4.f, ay - 5.f - chev_dn * 3.f ), chev_col.with_alpha( static_cast<int>( 255.f * chev_up ) ), "\xef\x81\xb7", 10.f );

			if ( m_anim > 0.01f && g_ctx && g_ctx->m_open && g_ctx->m_open_anim > 0.05f )
				g_render->defer( [this]( ) { paint_popup( ); } );
		}

		void input( )
		{
			if ( !g_input || !g_style )
				return;

			if ( g_input->mouse_in_region( m_box, m_box_size ) && g_input->clicked( mouse_buttons::left ) )
			{
				m_open = !m_open;
				if ( !m_open )
				{
					m_search_focus = false;
					m_dots_idx = -1;
					commit_rename( );
				}
				return;
			}

			if ( !m_open || m_anim < 0.25f )
				return;

			if ( m_dots_idx >= 0 && m_dots_anim > 0.2f )
			{
				constexpr float k_mw = 132.f;
				constexpr float k_row = 28.f;
				constexpr float k_pad = 6.f;
				float y = m_dots_menu_pos.y + k_pad;
				c_vector_2d delete_rp {};
				c_vector_2d delete_rs {};
				for ( int i = 0; i < 3; ++i )
				{
					if ( i == 1 )
						y += 8.f;
					const c_vector_2d rp( m_dots_menu_pos.x + 4.f, y );
					const c_vector_2d rs( k_mw - 8.f, k_row );
					if ( i == 2 )
					{
						delete_rp = rp;
						delete_rs = rs;
					}

					if ( g_input->mouse_in_region( rp, rs ) && g_input->clicked( mouse_buttons::left ) )
					{
						const int cfg = m_dots_idx;
						if ( i == 0 )
						{
							m_dots_idx = -1;
							m_delete_hold = false;
						}
						else if ( i == 1 && cfg >= 0 && cfg < static_cast<int>( m_configs.size( ) ) )
						{
							duplicate( cfg );
							m_dots_idx = -1;
							m_delete_hold = false;
						}
						else if ( i == 2 )
						{
							const int target = next_deletable( cfg );
							if ( target >= 0 )
								begin_remove( target );
							m_delete_hold = true;
							m_delete_hold_t = 0.f;
						}
						return;
					}
					y += k_row;
				}

				if ( m_delete_hold
					&& g_input->click_down( mouse_buttons::left )
					&& g_input->mouse_in_region( delete_rp, delete_rs ) )
				{
					m_delete_hold_t += ImGui::GetIO( ).DeltaTime;
					if ( m_delete_hold_t >= 0.2f )
					{
						const int target = next_deletable( m_dots_idx );
						if ( target >= 0 )
						{
							begin_remove( target );
							m_delete_hold_t = 0.f;
						}
						else
						{
							m_delete_hold = false;
							m_dots_idx = -1;
						}
					}
					return;
				}

				if ( !g_input->click_down( mouse_buttons::left ) )
					m_delete_hold = false;

				if ( g_input->clicked( mouse_buttons::left )
					&& !g_input->mouse_in_region( m_dots_menu_pos, m_dots_menu_size ) )
				{
					m_dots_idx = -1;
					m_delete_hold = false;
					return;
				}

				if ( m_dots_idx >= 0 )
					return;
			}
			else
			{
				m_delete_hold = false;
			}

			if ( m_search_focus )
			{
				c_text_input::consume_typing(
					m_search,
					nullptr,
					48,
					&m_search_anim,
					g_style->text_control,
					ImGui::GetIO( ).DeltaTime );
				if ( g_input->key_pressed( VK_ESCAPE ) || g_input->key_pressed( VK_RETURN ) )
					m_search_focus = false;
			}

			if ( m_rename_idx >= 0 )
			{
				c_text_input::consume_typing(
					m_rename_buf,
					nullptr,
					48,
					&m_rename_anim,
					g_style->text_control,
					ImGui::GetIO( ).DeltaTime );
				if ( g_input->key_pressed( VK_RETURN ) )
					commit_rename( );
				else if ( g_input->key_pressed( VK_ESCAPE ) )
				{
					m_rename_idx = -1;
					m_rename_anim.clear( );
				}
			}

			if ( g_input->mouse_in_region( m_create_box, m_create_size ) && g_input->clicked( mouse_buttons::left ) )
			{
				m_create_press = 1.f;
				create( );
				return;
			}

			if ( g_input->mouse_in_region( m_search_box, m_search_size ) && g_input->clicked( mouse_buttons::left ) )
			{
				m_search_focus = true;
				commit_rename( );
				m_dots_idx = -1;
				return;
			}

			constexpr float k_dots = 18.f;
			bool hit = false;
			for ( const row_rect_t& row : m_row_rects )
			{
				if ( row.index >= 0
					&& row.index < static_cast<int>( m_removing.size( ) )
					&& m_removing[static_cast<std::size_t>( row.index )] )
					continue;

				const c_vector_2d dots( row.pos.x + row.size.x - k_dots - 4.f, row.pos.y + std::floor( ( row.size.y - k_dots ) * 0.5f ) );
				if ( g_input->mouse_in_region( dots, c_vector_2d( k_dots, k_dots ) ) && g_input->clicked( mouse_buttons::left ) )
				{
					m_dots_idx = ( m_dots_idx == row.index ) ? -1 : row.index;
					m_dots_anchor = dots;
					m_search_focus = false;
					commit_rename( );
					hit = true;
					break;
				}

				const float name_w = dots.x - row.pos.x - 14.f;
				const c_vector_2d name_pos( row.pos.x + 6.f, row.pos.y + 4.f );
				const c_vector_2d name_size( name_w, row.size.y - 8.f );
				if ( g_input->mouse_in_region( name_pos, name_size ) && g_input->clicked( mouse_buttons::left ) )
				{
					if ( m_index == row.index )
					{
						commit_rename( );
						m_rename_idx = row.index;
						m_rename_buf = m_configs[static_cast<std::size_t>( row.index )];
						m_rename_anim.clear( );
						m_rename_anim.enter.assign( m_rename_buf.size( ), 1.f );
						m_search_focus = false;
						m_dots_idx = -1;
					}
					else
					{
						m_index = row.index;
						if ( row.index >= 0 && row.index < static_cast<int>( m_row_press.size( ) ) )
							m_row_press[static_cast<std::size_t>( row.index )] = 1.f;
						commit_rename( );
						m_search_focus = false;
						m_dots_idx = -1;
					}
					hit = true;
					break;
				}

				if ( g_input->mouse_in_region( row.pos, row.size ) && g_input->clicked( mouse_buttons::left ) )
				{
					m_index = row.index;
					if ( row.index >= 0 && row.index < static_cast<int>( m_row_press.size( ) ) )
						m_row_press[static_cast<std::size_t>( row.index )] = 1.f;
					commit_rename( );
					m_search_focus = false;
					m_dots_idx = -1;
					hit = true;
					break;
				}
			}

			if ( !hit && g_input->clicked( mouse_buttons::left ) )
			{
				if ( !g_input->mouse_in_region( m_popup_pos, m_popup_size )
					&& !g_input->mouse_in_region( m_box, m_box_size ) )
				{
					m_open = false;
					m_search_focus = false;
					m_dots_idx = -1;
					commit_rename( );
				}
				else if ( !g_input->mouse_in_region( m_search_box, m_search_size ) )
				{
					m_search_focus = false;
					if ( m_rename_idx >= 0 )
						commit_rename( );
				}
			}
		}

		bool blocks_menu( ) const
		{
			return m_open || m_anim > 0.05f;
		}

		bool is_open( ) const
		{
			return m_open;
		}

		c_vector_2d box_pos( ) const
		{
			return m_box;
		}

		c_vector_2d box_size( ) const
		{
			return m_box_size;
		}

		void close( )
		{
			m_open = false;
			m_anim = 0.f;
			m_search_focus = false;
			m_dots_idx = -1;
			m_dots_anim = 0.f;
			m_rename_idx = -1;
			m_delete_hold = false;
		}

	private:
		struct row_rect_t
		{
			c_vector_2d pos {};
			c_vector_2d size {};
			int index { -1 };
		};

		void paint_popup( )
		{
			constexpr float k_w = 236.f;
			constexpr float k_pad = 10.f;
			constexpr float k_header_h = 28.f;
			constexpr float k_search_h = 28.f;
			constexpr float k_row_h = 28.f;
			constexpr float k_row_gap = 4.f;
			constexpr float k_sep = 10.f;
			constexpr float k_dots = 18.f;
			constexpr float k_icon = 12.f;

			const float dt = ImGui::GetIO( ).DeltaTime;
			tick_list_anims( dt );
			m_create_press += ( 0.f - m_create_press ) * ( 1.f - std::exp( -20.f * dt ) );
			if ( m_create_press < 0.002f )
				m_create_press = 0.f;

			const float e = c_render::ease_out_cubic( m_anim );
			const int vtx = g_render->vtx_count( );
			const float fr = 8.f;
			const c_vector_2d pivot( m_box.x + m_box_size.x * 0.5f, m_box.y + m_box_size.y * 0.5f );

			std::vector<int> visible;
			visible.reserve( m_configs.size( ) );
			float list_h = 0.f;
			for ( int i = 0; i < static_cast<int>( m_configs.size( ) ); ++i )
			{
				if ( !matches_search( i ) )
					continue;
				visible.push_back( i );
				const float ap = i < static_cast<int>( m_appear.size( ) ) ? m_appear[static_cast<std::size_t>( i )] : 1.f;
				const float rm = i < static_cast<int>( m_remove.size( ) ) ? m_remove[static_cast<std::size_t>( i )] : 0.f;
				const float vis = c_render::ease_out_cubic( ap ) * ( 1.f - c_render::ease_out_cubic( rm ) );
				list_h += ( k_row_h + k_row_gap ) * vis;
			}
			if ( list_h < k_row_h * 0.35f )
				list_h = k_row_h;
			else if ( !visible.empty( ) )
				list_h -= k_row_gap * 0.35f;

			const float body_h = k_pad + k_header_h + k_sep + k_search_h + k_sep + list_h + k_pad;
			float px = std::floor( m_box.x + m_box_size.x - k_w );
			float py = std::floor( m_box.y + m_box_size.y + 8.f );
			const ImVec2 display = ImGui::GetIO( ).DisplaySize;
			if ( px < 8.f )
				px = 8.f;
			if ( px + k_w > display.x - 8.f )
				px = display.x - k_w - 8.f;
			if ( py + body_h > display.y - 8.f )
				py = std::floor( m_box.y - body_h - 8.f );

			m_popup_pos = c_vector_2d( px, py );
			m_popup_size = c_vector_2d( k_w, body_h );

			g_render->frosted_panel( px, py, k_w, body_h, fr, true );

			const float header_y = py + k_pad;
			const char* present_icon = "\xef\x83\x82";
			const c_vector_2d present_ts = g_render->measure_text( c_fonts::k_default_key, "Present", g_style->text_control );
			const c_vector_2d present_is = g_render->measure_text( c_fonts::k_icons_key, present_icon, k_icon );
			constexpr float chip_pad_x = 9.f;
			constexpr float chip_gap = 7.f;
			const float chip_w = std::floor( chip_pad_x + present_is.x + chip_gap + present_ts.x + chip_pad_x );
			const float chip_x = px + k_pad;
			g_render->rect_filled_f( chip_x, header_y, chip_w, k_header_h, c_color( 20, 23, 32 ), 7.f );
			g_render->rect_f( chip_x, header_y, chip_w, k_header_h, c_color( 42, 46, 58 ), 7.f, 1.f );
			g_render->text(
				c_fonts::k_icons_key,
				c_vector_2d(
					std::floor( chip_x + chip_pad_x ),
					std::floor( header_y + ( k_header_h - present_is.y ) * 0.5f ) ),
				g_style->accent.with_alpha( static_cast<int>( 255.f * e ) ),
				present_icon,
				k_icon );
			g_render->text(
				c_fonts::k_default_key,
				c_vector_2d(
					std::floor( chip_x + chip_pad_x + present_is.x + chip_gap ),
					std::floor( header_y + ( k_header_h - present_ts.y ) * 0.5f ) ),
				g_style->accent.with_alpha( static_cast<int>( 255.f * e ) ),
				"Present",
				g_style->text_control );

			const char* plus_icon = "\xef\x81\xa7";
			m_create_box = c_vector_2d( px + k_w - k_pad - k_header_h, header_y );
			m_create_size = c_vector_2d( k_header_h, k_header_h );
			const bool create_hov = g_input && g_input->mouse_in_region( m_create_box, m_create_size );
			const bool create_down = create_hov && g_input && g_input->click_down( mouse_buttons::left );
			m_create_hover += ( ( create_hov ? 1.f : 0.f ) - m_create_hover ) * ( 1.f - std::exp( -22.f * dt ) );
			const float che = c_render::ease_out_cubic( m_create_hover );
			const float cpe = c_render::ease_out_cubic( m_create_press );
			const float press = ( std::max )( cpe, create_down ? 0.55f : 0.f );
			const float inset = press * 1.5f;
			const float bx = m_create_box.x + inset;
			const float by = m_create_box.y + inset;
			const float bw = k_header_h - inset * 2.f;
			const float bh = k_header_h - inset * 2.f;
			const c_color create_fill = c_color( 20, 23, 32 )
				.lerp( c_color( 32, 38, 54 ), che * 0.85f )
				.lerp( g_style->accent, press * 0.28f );
			const c_color create_border = c_color( 42, 46, 58 )
				.lerp( g_style->accent, che * 0.55f + press * 0.45f );
			g_render->rect_filled_f( bx, by, bw, bh, create_fill, 7.f );
			g_render->rect_f( bx, by, bw, bh, create_border, 7.f, 1.f );
			const c_vector_2d plus_is = g_render->measure_text( c_fonts::k_icons_key, plus_icon, k_icon );

			g_render->text(
				c_fonts::k_icons_key,
				c_vector_2d(
					std::floor( m_create_box.x + ( k_header_h - plus_is.x ) * 0.5f ),
					std::floor( m_create_box.y + ( k_header_h - plus_is.y ) * 0.5f ) ),
				c_color( 200, 204, 214 ).lerp( g_style->accent, che * 0.7f + press * 0.3f ),
				plus_icon,
				k_icon );

			float sep_y = header_y + k_header_h + k_sep * 0.5f;
			g_render->line( c_vector_2d( px + k_pad, sep_y ), c_vector_2d( px + k_w - k_pad, sep_y ), c_color( 48, 52, 64, static_cast<int>( 140.f * e ) ) );

			m_search_box = c_vector_2d( px + k_pad, header_y + k_header_h + k_sep );
			m_search_size = c_vector_2d( k_w - k_pad * 2.f, k_search_h );
			m_search_focus_anim += ( ( m_search_focus ? 1.f : 0.f ) - m_search_focus_anim ) * ( 1.f - std::exp( -20.f * dt ) );
			const float sfe = c_render::ease_out_cubic( m_search_focus_anim );
			const bool search_hov = g_input && g_input->mouse_in_region( m_search_box, m_search_size );
			m_search_hover += ( ( search_hov || m_search_focus ? 1.f : 0.f ) - m_search_hover ) * ( 1.f - std::exp( -22.f * dt ) );
			const float she = c_render::ease_out_cubic( m_search_hover );
			const c_color sfill = c_color( 18, 20, 28 )
				.lerp( c_color( 26, 30, 42 ), she * 0.75f )
				.lerp( c_color( 22, 28, 44 ), sfe );
			const c_color sborder = c_color( 40, 44, 56 ).lerp( g_style->accent, sfe * 0.95f );
			g_render->rect_filled_f( m_search_box.x, m_search_box.y, m_search_size.x, m_search_size.y, sfill, 6.f );
			g_render->rect_f( m_search_box.x, m_search_box.y, m_search_size.x, m_search_size.y, sborder, 6.f, 1.f );

			const char* search_icon = "\xef\x80\x82";
			const c_vector_2d search_is = g_render->measure_text( c_fonts::k_icons_key, search_icon, k_icon );
			g_render->text(
				c_fonts::k_icons_key,
				c_vector_2d(
					std::floor( m_search_box.x + 9.f ),
					std::floor( m_search_box.y + ( k_search_h - search_is.y ) * 0.5f ) ),
				c_color( 120, 126, 140 ).lerp( g_style->accent, sfe ),
				search_icon,
				k_icon );

			const float text_x = m_search_box.x + 9.f + search_is.x + 8.f;
			const float text_y = m_search_box.y + std::floor( ( k_search_h - g_style->text_control ) * 0.5f );
			const bool show_ph = m_search.empty( ) && !m_search_focus && m_search_anim.exit.empty( );
			m_search_ph_anim += ( ( show_ph ? 1.f : 0.f ) - m_search_ph_anim ) * ( 1.f - std::exp( -20.f * dt ) );
			const float phe = c_render::ease_out_cubic( m_search_ph_anim );
			if ( phe > 0.01f )
			{
				g_render->text(
					c_fonts::k_default_key,
					c_vector_2d( text_x, text_y + ( 1.f - phe ) * 10.f ),
					c_color( 96, 102, 116, static_cast<int>( 255.f * phe ) ),
					"Search",
					g_style->text_control );
			}

			c_text_input::tick_text_anims( m_search_anim, dt );
			if ( ImDrawList* dl = g_render->draw_list( ) )
			{
				dl->PushClipRect(
					ImVec2( text_x - 1.f, m_search_box.y + 1.f ),
					ImVec2( m_search_box.x + m_search_size.x - 8.f, m_search_box.y + k_search_h - 1.f ),
					true );
				c_text_input::paint_text_chars(
					m_search,
					m_search_anim,
					text_x,
					text_y,
					g_style->text_control,
					g_style->text_bright );
				if ( m_search_focus )
				{
					m_search_caret_t += dt;
					if ( std::fmod( m_search_caret_t, 1.05f ) < 0.55f )
					{
						const c_vector_2d ps = g_render->measure_text( c_fonts::k_default_key, m_search.c_str( ), g_style->text_control );
						const float cx = text_x + ps.x;
						g_render->line(
							c_vector_2d( cx, m_search_box.y + 6.f ),
							c_vector_2d( cx, m_search_box.y + k_search_h - 6.f ),
							g_style->accent,
							1.2f );
					}
				}
				dl->PopClipRect( );
			}

			sep_y = m_search_box.y + k_search_h + k_sep * 0.5f;
			g_render->line( c_vector_2d( px + k_pad, sep_y ), c_vector_2d( px + k_w - k_pad, sep_y ), c_color( 48, 52, 64, static_cast<int>( 140.f * e ) ) );

			float row_y = m_search_box.y + k_search_h + k_sep;
			m_row_rects.clear( );
			ensure_anims( );

			struct row_layout_t
			{
				int i { -1 };
				float y { 0.f };
				float rh { 0.f };
				float slide { 0.f };
				float vis { 0.f };
			};
			std::vector<row_layout_t> layouts;
			layouts.reserve( visible.size( ) );

			float sel_target_y = row_y;
			float sel_target_h = k_row_h;
			bool sel_visible = false;

			for ( int i : visible )
			{
				const float ap = i < static_cast<int>( m_appear.size( ) ) ? m_appear[static_cast<std::size_t>( i )] : 1.f;
				const float rm = i < static_cast<int>( m_remove.size( ) ) ? m_remove[static_cast<std::size_t>( i )] : 0.f;
				const float ape = c_render::ease_out_cubic( ap );
				const float rme = c_render::ease_out_cubic( rm );
				const float row_vis = ape * ( 1.f - rme );
				if ( row_vis < 0.02f )
				{
					row_y += ( k_row_h + k_row_gap ) * row_vis;
					continue;
				}

				const float rh = k_row_h * row_vis;
				const float slide = ( 1.f - ape ) * 14.f - rme * 18.f;
				layouts.push_back( { i, row_y, rh, slide, row_vis } );
				m_row_rects.push_back( { c_vector_2d( px + 8.f, row_y ), c_vector_2d( k_w - 16.f, k_row_h ), i } );

				if ( i == m_index )
				{
					sel_target_y = row_y;
					sel_target_h = rh;
					sel_visible = true;
				}

				row_y += rh + k_row_gap * row_vis;
			}

			m_sel_vis += ( ( sel_visible ? 1.f : 0.f ) - m_sel_vis ) * ( 1.f - std::exp( -20.f * dt ) );
			if ( sel_visible )
			{
				if ( !m_sel_ready )
				{
					m_sel_y = sel_target_y;
					m_sel_h = sel_target_h;
					m_sel_ready = true;
				}
				else
				{
					m_sel_y += ( sel_target_y - m_sel_y ) * ( 1.f - std::exp( -24.f * dt ) );
					m_sel_h += ( sel_target_h - m_sel_h ) * ( 1.f - std::exp( -24.f * dt ) );
				}
			}
			else
			{
				m_sel_ready = false;
			}

			const float sve = c_render::ease_out_cubic( m_sel_vis );
			if ( sve > 0.01f )
			{
				const float sx = px + 8.f;
				const float sw_row = k_w - 16.f;
				g_render->rect_filled_f(
					sx, m_sel_y, sw_row, m_sel_h,
					c_color( 32, 38, 54, static_cast<int>( 210.f * sve ) ),
					6.f );
				const float bar_h = ( std::max )( 4.f, m_sel_h - 6.f );
				const float bar_y = m_sel_y + ( m_sel_h - bar_h ) * 0.5f;
				const float bar_r = ( std::min )( 3.5f, bar_h * 0.5f );
				g_render->rect_filled_f(
					sx + 1.f, bar_y, 3.f, bar_h,
					g_style->accent.with_alpha( static_cast<int>( 255.f * sve ) ),
					bar_r );
			}

			for ( const row_layout_t& lay : layouts )
			{
				const int i = lay.i;
				const float rh = lay.rh;
				const float row_vis = lay.vis;
				const float slide = lay.slide;
				const c_vector_2d rp( px + 8.f + slide, lay.y );
				const c_vector_2d rs( k_w - 16.f, rh );

				const bool sel = i == m_index;
				const bool hovered = g_input && g_input->mouse_in_region( c_vector_2d( px + 8.f, lay.y ), c_vector_2d( k_w - 16.f, k_row_h ) ) && row_vis > 0.75f;

				m_row_hover[static_cast<std::size_t>( i )] += ( ( hovered ? 1.f : 0.f ) - m_row_hover[static_cast<std::size_t>( i )] ) * ( 1.f - std::exp( -24.f * dt ) );
				m_row_active[static_cast<std::size_t>( i )] += ( ( sel ? 1.f : 0.f ) - m_row_active[static_cast<std::size_t>( i )] ) * ( 1.f - std::exp( -20.f * dt ) );
				m_row_press[static_cast<std::size_t>( i )] += ( 0.f - m_row_press[static_cast<std::size_t>( i )] ) * ( 1.f - std::exp( -18.f * dt ) );

				const float hh = c_render::ease_out_cubic( m_row_hover[static_cast<std::size_t>( i )] ) * row_vis;
				const float aa = c_render::ease_out_cubic( m_row_active[static_cast<std::size_t>( i )] ) * row_vis;
				const float pp = c_render::ease_out_cubic( m_row_press[static_cast<std::size_t>( i )] ) * row_vis;

				if ( hh > 0.01f && !sel )
				{
					g_render->rect_filled_f(
						rp.x, rp.y, rs.x, rh,
						c_color( 28, 32, 44, static_cast<int>( 130.f * hh ) ),
						6.f );
				}
				if ( pp > 0.01f )
				{
					g_render->rect_filled_f(
						rp.x, rp.y, rs.x, rh,
						g_style->accent.with_alpha( static_cast<int>( 45.f * pp ) ),
						6.f );
				}

				const c_vector_2d dots( px + 8.f + ( k_w - 16.f ) - k_dots - 2.f, lay.y + std::floor( ( rh - k_dots ) * 0.5f ) );
				const bool renaming = m_rename_idx == i;
				const float name_max_w = dots.x - ( px + 8.f ) - 12.f;
				const int alpha = static_cast<int>( 255.f * row_vis );
				const float text_mix = ( std::min )( 1.f, hh * 0.55f + aa );

				if ( renaming && row_vis > 0.7f )
				{
					c_text_input::tick_text_anims( m_rename_anim, dt );
					const float nx = px + 20.f;
					const float ny = lay.y + std::floor( ( rh - g_style->text_control ) * 0.5f );
					g_render->rect_filled_f( px + 14.f, lay.y + 4.f, name_max_w, rh - 8.f, c_color( 16, 18, 26, alpha ), 5.f );
					g_render->rect_f( px + 14.f, lay.y + 4.f, name_max_w, rh - 8.f, g_style->accent.with_alpha( static_cast<int>( 180.f * row_vis ) ), 5.f, 1.f );
					if ( ImDrawList* rdl = g_render->draw_list( ) )
					{
						rdl->PushClipRect(
							ImVec2( px + 16.f, lay.y + 4.f ),
							ImVec2( px + 14.f + name_max_w - 2.f, lay.y + rh - 4.f ),
							true );
						c_text_input::paint_text_chars(
							m_rename_buf,
							m_rename_anim,
							nx,
							ny,
							g_style->text_control,
							g_style->text_bright.with_alpha( alpha ) );
						rdl->PopClipRect( );
					}
					m_rename_caret_t += dt;
					if ( std::fmod( m_rename_caret_t, 1.05f ) < 0.55f )
					{
						const c_vector_2d nts = g_render->measure_text( c_fonts::k_default_key, m_rename_buf.c_str( ), g_style->text_control );
						const float cx = nx + nts.x;
						g_render->line( c_vector_2d( cx, lay.y + 6.f ), c_vector_2d( cx, lay.y + rh - 6.f ), g_style->accent.with_alpha( alpha ), 1.2f );
					}
				}
				else
				{
					const c_vector_2d nts = g_render->measure_text( c_fonts::k_default_key, m_configs[static_cast<std::size_t>( i )].c_str( ), g_style->text_control );
					const c_color name_col = c_color( 168, 172, 186 )
						.lerp( c_color( 236, 238, 245 ), text_mix )
						.lerp( g_style->accent, aa * 0.22f )
						.with_alpha( alpha );
					const float text_nudge = hh * 1.5f + aa * 2.f;
					g_render->text(
						c_fonts::k_default_key,
						c_vector_2d( px + 18.f + slide * 0.35f + text_nudge, lay.y + std::floor( ( rh - nts.y ) * 0.5f ) ),
						name_col,
						m_configs[static_cast<std::size_t>( i )].c_str( ),
						g_style->text_control );
				}

				if ( row_vis > 0.5f )
				{
					const bool dots_hov = g_input && g_input->mouse_in_region( dots, c_vector_2d( k_dots, k_dots ) );
					const float dots_t = ( dots_hov || m_dots_idx == i ) ? 1.f : 0.f;
					const c_color dcol = c_color( 140, 146, 158 ).lerp( g_style->text_bright, dots_t ).lerp( g_style->accent, aa * 0.35f ).with_alpha( alpha );
					const float dcx = dots.x + k_dots * 0.5f;
					const float dcy = dots.y + k_dots * 0.5f;
					for ( int d = -1; d <= 1; ++d )
						g_render->circle_filled( c_vector_2d( dcx, dcy + static_cast<float>( d ) * 3.6f ), 1.35f, dcol, 10 );
				}
			}

			if ( visible.empty( ) )
			{
				g_render->text(
					c_fonts::k_default_key,
					c_vector_2d( px + k_pad + 2.f, row_y + 6.f ),
					c_color( 110, 116, 130, static_cast<int>( 255.f * e ) ),
					"No configs found",
					g_style->text_control );
			}

			g_render->apply_open( vtx, pivot, m_anim, 0.9f, 12.f );

			m_dots_anim += ( ( m_dots_idx >= 0 ? 1.f : 0.f ) - m_dots_anim ) * ( 1.f - std::exp( -18.f * dt ) );
			if ( std::fabs( m_dots_anim - ( m_dots_idx >= 0 ? 1.f : 0.f ) ) < 0.0008f )
				m_dots_anim = m_dots_idx >= 0 ? 1.f : 0.f;

			if ( m_dots_anim > 0.01f )
				g_render->defer( [this]( ) { paint_dots_menu( ); } );
		}

		void paint_dots_menu( )
		{
			constexpr float k_mw = 132.f;
			constexpr float k_row = 28.f;
			constexpr float k_pad = 6.f;
			const float menu_h = k_pad * 2.f + k_row * 3.f + 8.f;
			const float e = c_render::ease_out_cubic( m_dots_anim );
			const int vtx = g_render->vtx_count( );

			c_vector_2d anchor = m_dots_anchor;
			float mx = anchor.x - k_mw + 18.f;
			float my = anchor.y + 20.f;
			const ImVec2 display = ImGui::GetIO( ).DisplaySize;
			if ( mx < 8.f )
				mx = 8.f;
			if ( my + menu_h > display.y - 8.f )
				my = anchor.y - menu_h - 6.f;

			m_dots_menu_pos = c_vector_2d( mx, my );
			m_dots_menu_size = c_vector_2d( k_mw, menu_h );

			g_render->frosted_panel( mx, my, k_mw, menu_h, 7.f, true );

			const char* labels[3] = { "Reload", "Duplicate", "Delete" };
			const char* icons[3] = { "\xef\x80\xa1", "\xef\x83\x85", "\xef\x87\xb8" };
			const bool danger[3] = { false, false, true };

			float y = my + k_pad;
			for ( int i = 0; i < 3; ++i )
			{
				if ( i == 1 )
				{
					g_render->line(
						c_vector_2d( mx + 8.f, y + 0.5f ),
						c_vector_2d( mx + k_mw - 8.f, y + 0.5f ),
						c_color( 48, 52, 64, static_cast<int>( 150.f * e ) ) );
					y += 8.f;
				}

				const c_vector_2d rp( mx + 4.f, y );
				const c_vector_2d rs( k_mw - 8.f, k_row );
				const bool hov = g_input && g_input->mouse_in_region( rp, rs );
				m_dots_item_hover[i] += ( ( hov ? 1.f : 0.f ) - m_dots_item_hover[i] ) * ( 1.f - std::exp( -22.f * ImGui::GetIO( ).DeltaTime ) );
				const float ih = c_render::ease_out_cubic( m_dots_item_hover[i] );
				if ( ih > 0.01f )
				{
					g_render->rect_filled_f(
						rp.x, rp.y, rs.x, rs.y,
						danger[i]
							? c_color( 60, 24, 30, static_cast<int>( 180.f * ih ) )
							: c_color( 28, 32, 44, static_cast<int>( 180.f * ih ) ),
						5.f );
				}

				const c_color col = danger[i]
					? c_color( 230, 96, 108 ).lerp( c_color( 255, 130, 140 ), ih )
					: c_color( 190, 194, 206 ).lerp( g_style->text_bright, ih );
				g_render->text( c_fonts::k_icons_key, c_vector_2d( rp.x + 10.f, rp.y + 7.f ), col, icons[i], 11.f );
				const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, labels[i], g_style->text_control );
				g_render->text(
					c_fonts::k_default_key,
					c_vector_2d( rp.x + 30.f, rp.y + std::floor( ( k_row - ts.y ) * 0.5f ) ),
					col,
					labels[i],
					g_style->text_control );

				y += k_row;
			}

			g_render->apply_open( vtx, c_vector_2d( anchor.x, anchor.y ), m_dots_anim, 0.9f, 10.f );
		}

		bool matches_search( int index ) const
		{
			if ( m_search.empty( ) )
				return true;
			if ( index < 0 || index >= static_cast<int>( m_configs.size( ) ) )
				return false;
			std::string name = m_configs[static_cast<std::size_t>( index )];
			std::string q = m_search;
			for ( char& c : name )
				c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
			for ( char& c : q )
				c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
			return name.find( q ) != std::string::npos;
		}

		void commit_rename( )
		{
			if ( m_rename_idx < 0 || m_rename_idx >= static_cast<int>( m_configs.size( ) ) )
			{
				m_rename_idx = -1;
				m_rename_anim.clear( );
				return;
			}
			if ( !m_rename_buf.empty( ) )
				m_configs[static_cast<std::size_t>( m_rename_idx )] = m_rename_buf;
			m_rename_idx = -1;
			m_rename_anim.clear( );
		}

		void create( )
		{
			ensure_anims( );
			int n = 1;
			std::string name;
			for ( ;; )
			{
				name = "Config " + std::to_string( n );
				bool taken = false;
				for ( const std::string& c : m_configs )
				{
					if ( c == name )
					{
						taken = true;
						break;
					}
				}
				if ( !taken )
					break;
				++n;
			}
			m_configs.push_back( name );
			m_appear.push_back( 0.f );
			m_remove.push_back( 0.f );
			m_removing.push_back( false );
			m_row_hover.push_back( 0.f );
			m_row_active.push_back( 0.f );
			m_row_press.push_back( 0.f );
			m_index = static_cast<int>( m_configs.size( ) ) - 1;
			m_search.clear( );
			m_search_anim.clear( );
		}

		void ensure_anims( )
		{
			while ( m_appear.size( ) < m_configs.size( ) )
				m_appear.push_back( 1.f );
			while ( m_remove.size( ) < m_configs.size( ) )
				m_remove.push_back( 0.f );
			while ( m_removing.size( ) < m_configs.size( ) )
				m_removing.push_back( false );
			while ( m_row_hover.size( ) < m_configs.size( ) )
				m_row_hover.push_back( 0.f );
			while ( m_row_active.size( ) < m_configs.size( ) )
				m_row_active.push_back( 0.f );
			while ( m_row_press.size( ) < m_configs.size( ) )
				m_row_press.push_back( 0.f );
			if ( m_appear.size( ) > m_configs.size( ) )
				m_appear.resize( m_configs.size( ) );
			if ( m_remove.size( ) > m_configs.size( ) )
				m_remove.resize( m_configs.size( ) );
			if ( m_removing.size( ) > m_configs.size( ) )
				m_removing.resize( m_configs.size( ) );
			if ( m_row_hover.size( ) > m_configs.size( ) )
				m_row_hover.resize( m_configs.size( ) );
			if ( m_row_active.size( ) > m_configs.size( ) )
				m_row_active.resize( m_configs.size( ) );
			if ( m_row_press.size( ) > m_configs.size( ) )
				m_row_press.resize( m_configs.size( ) );
		}

		void tick_list_anims( float dt )
		{
			ensure_anims( );
			for ( std::size_t i = 0; i < m_configs.size( ); ++i )
			{
				const float target = m_removing[i] ? 0.f : 1.f;
				m_appear[i] += ( target - m_appear[i] ) * ( 1.f - std::exp( -14.f * dt ) );
				if ( !m_removing[i] && m_appear[i] > 0.999f )
					m_appear[i] = 1.f;

				const float rtarget = m_removing[i] ? 1.f : 0.f;
				m_remove[i] += ( rtarget - m_remove[i] ) * ( 1.f - std::exp( -16.f * dt ) );
			}

			for ( int i = static_cast<int>( m_configs.size( ) ) - 1; i >= 0; --i )
			{
				if ( !m_removing[static_cast<std::size_t>( i )] )
					continue;
				if ( m_remove[static_cast<std::size_t>( i )] < 0.97f )
					continue;

				m_configs.erase( m_configs.begin( ) + i );
				m_appear.erase( m_appear.begin( ) + i );
				m_remove.erase( m_remove.begin( ) + i );
				m_removing.erase( m_removing.begin( ) + i );
				if ( i < static_cast<int>( m_row_hover.size( ) ) )
					m_row_hover.erase( m_row_hover.begin( ) + i );
				if ( i < static_cast<int>( m_row_active.size( ) ) )
					m_row_active.erase( m_row_active.begin( ) + i );
				if ( i < static_cast<int>( m_row_press.size( ) ) )
					m_row_press.erase( m_row_press.begin( ) + i );
				if ( m_rename_idx == i )
					m_rename_idx = -1;
				else if ( m_rename_idx > i )
					--m_rename_idx;
				if ( m_dots_idx == i )
				{
					if ( i >= static_cast<int>( m_configs.size( ) ) )
						m_dots_idx = static_cast<int>( m_configs.size( ) ) - 1;
				}
				else if ( m_dots_idx > i )
					--m_dots_idx;
				if ( m_index >= static_cast<int>( m_configs.size( ) ) )
					m_index = static_cast<int>( m_configs.size( ) ) - 1;
				else if ( m_index > i )
					--m_index;
			}
		}

		int next_deletable( int prefer ) const
		{
			auto alive = [this]( int i ) -> bool
			{
				if ( i < 0 || i >= static_cast<int>( m_configs.size( ) ) )
					return false;
				if ( i >= static_cast<int>( m_removing.size( ) ) )
					return true;
				return !m_removing[static_cast<std::size_t>( i )];
			};

			if ( alive( prefer ) )
				return prefer;

			for ( int i = prefer + 1; i < static_cast<int>( m_configs.size( ) ); ++i )
			{
				if ( alive( i ) )
					return i;
			}
			for ( int i = 0; i < prefer && i < static_cast<int>( m_configs.size( ) ); ++i )
			{
				if ( alive( i ) )
					return i;
			}
			return -1;
		}

		void begin_remove( int cfg )
		{
			ensure_anims( );
			if ( cfg < 0 || cfg >= static_cast<int>( m_configs.size( ) ) )
				return;
			int alive = 0;
			for ( std::size_t i = 0; i < m_configs.size( ); ++i )
			{
				if ( !m_removing[i] )
					++alive;
			}
			if ( alive <= 1 )
				return;
			m_removing[static_cast<std::size_t>( cfg )] = true;
			if ( m_rename_idx == cfg )
				m_rename_idx = -1;
		}

		void duplicate( int cfg )
		{
			ensure_anims( );
			if ( cfg < 0 || cfg >= static_cast<int>( m_configs.size( ) ) )
				return;
			m_configs.push_back( m_configs[static_cast<std::size_t>( cfg )] + " Copy" );
			m_appear.push_back( 0.f );
			m_remove.push_back( 0.f );
			m_removing.push_back( false );
			m_row_hover.push_back( 0.f );
			m_row_active.push_back( 0.f );
			m_row_press.push_back( 0.f );
			m_index = static_cast<int>( m_configs.size( ) ) - 1;
		}

		std::vector<std::string> m_configs { "Default", "Rage HVH", "Legit", "Tournament" };
		std::vector<float> m_appear { 1.f, 1.f, 1.f, 1.f };
		std::vector<float> m_remove { 0.f, 0.f, 0.f, 0.f };
		std::vector<char> m_removing { 0, 0, 0, 0 };
		int m_index { 0 };
		bool m_open { false };
		float m_hover { 0.f };
		float m_anim { 0.f };
		std::string m_chip_label {};
		text_anim_state_t m_chip_anim {};
		float m_sel_y { 0.f };
		float m_sel_h { 28.f };
		float m_sel_vis { 0.f };
		bool m_sel_ready { false };
		c_vector_2d m_box {};
		c_vector_2d m_box_size {};
		c_vector_2d m_popup_pos {};
		c_vector_2d m_popup_size {};
		c_vector_2d m_create_box {};
		c_vector_2d m_create_size {};
		float m_create_hover { 0.f };
		float m_create_press { 0.f };
		bool m_delete_hold { false };
		float m_delete_hold_t { 0.f };
		c_vector_2d m_search_box {};
		c_vector_2d m_search_size {};
		std::string m_search {};
		text_anim_state_t m_search_anim {};
		bool m_search_focus { false };
		float m_search_focus_anim { 0.f };
		float m_search_hover { 0.f };
		float m_search_ph_anim { 1.f };
		float m_search_caret_t { 0.f };
		int m_rename_idx { -1 };
		std::string m_rename_buf {};
		text_anim_state_t m_rename_anim {};
		float m_rename_caret_t { 0.f };
		int m_dots_idx { -1 };
		float m_dots_anim { 0.f };
		c_vector_2d m_dots_anchor {};
		c_vector_2d m_dots_menu_pos {};
		c_vector_2d m_dots_menu_size {};
		float m_dots_item_hover[3] {};
		std::vector<float> m_row_hover {};
		std::vector<float> m_row_active {};
		std::vector<float> m_row_press {};
		std::vector<row_rect_t> m_row_rects {};
	};
}
