#pragma once

#include <cmath>
#include <string>

#include <core/framework/gui/backend/math/math.hxx>
#include <core/framework/gui/frontend/widgets/classes/base_element.hxx>
#include <core/framework/gui/frontend/widgets/settings.hxx>
#include <core/framework/gui/backend/render/render.hxx>

namespace core::gui
{
	class c_colorpicker : public c_base_element
	{
	public:
		c_colorpicker( std::string label, c_color* value, bool hide_label = false, bool* enabled = nullptr )
			: m_value( value ), m_enabled( enabled )
		{
			m_label = std::move( label );
			m_type = element_type::colorpicker;
			m_visible = true;
			m_focus_priority = focus_priority::modal;
			m_layer = render_layer::modal;
			m_size = c_vector_2d( 200.f, g_style ? g_style->row_height : 37.f );
			if ( hide_label )
				this->hide_label( );
			if ( m_enabled && *m_enabled )
				m_toggle_anim = 1.f;
			m_swatch_anim = 1.f;
			sync_hsv_from_value( );
		}

		void play_intro( ) override
		{
			m_reveal = 0.f;
			m_toggle_anim = 0.f;
			m_swatch_anim = 0.f;
		}

		void close_overlay( ) override
		{
			m_open = false;
			m_drag = drag_mode::none;
			m_ignore_click = false;
			if ( g_ctx )
				g_ctx->clear_modal( this );
		}

		void draw( ) override
		{
			if ( !is_visible( ) || !g_render || !g_style )
				return;

			const float row_h = g_style->row_height;
			m_size.y = row_h;
			const float width = m_parent_width > 0.f ? m_parent_width : m_size.x;
			m_size.x = width;

			if ( !m_hide_label )
			{
				const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, m_label.c_str( ), g_style->text_body );
				g_render->text( c_fonts::k_default_key, c_vector_2d( m_pos.x + g_style->label_pad, m_pos.y + std::floor( ( row_h - ts.y ) * 0.5f ) ), g_style->text, m_label.c_str( ), g_style->text_body );
			}

			layout_controls( width );

			const float dt = ImGui::GetIO( ).DeltaTime;
			if ( g_ctx && !g_ctx->m_open )
			{
				m_open = false;
				m_open_anim = 0.f;
			}
			else
			{
				m_open_anim += ( ( m_open ? 1.f : 0.f ) - m_open_anim ) * ( 1.f - std::exp( -3.4f * dt ) );
				if ( std::fabs( m_open_anim - ( m_open ? 1.f : 0.f ) ) < 0.0008f )
					m_open_anim = m_open ? 1.f : 0.f;
			}

			if ( tick_intro( dt ) )
			{
				const float target = ( m_enabled && *m_enabled ) ? 1.f : ( m_enabled ? 0.f : 1.f );
				m_toggle_anim += ( target - m_toggle_anim ) * ( 1.f - std::exp( -10.f * dt ) );
				m_swatch_anim += ( 1.f - m_swatch_anim ) * ( 1.f - std::exp( -3.6f * dt ) );
				if ( std::fabs( 1.f - m_swatch_anim ) < 0.001f )
					m_swatch_anim = 1.f;
			}

			draw_swatch( );

			if ( m_enabled )
			{
				const c_color track = g_style->toggle_off.lerp( g_style->accent, m_toggle_anim );
				g_render->rect_filled_f( m_toggle.x, m_toggle.y, g_style->toggle_w, g_style->toggle_h, track, g_style->toggle_h * 0.5f );
				g_render->circle_filled( c_vector_2d( m_toggle.x + ImLerp( 9.f, 20.f, m_toggle_anim ), m_toggle.y + g_style->toggle_h * 0.5f ), 7.f, g_style->knob_off.lerp( g_style->knob_on, m_toggle_anim ), 32 );
			}

			if ( m_open_anim > 0.01f && g_ctx && g_ctx->m_open && g_ctx->m_open_anim > 0.05f )
				g_render->defer( [this]( ) { draw_popup( ); } );
		}

		void input( ) override
		{
			if ( !is_visible( ) || !g_input || !g_ctx || !g_style )
				return;
			if ( !g_ctx->m_open )
				return;

			const float width = m_parent_width > 0.f ? m_parent_width : m_size.x;
			layout_controls( width );

			if ( m_enabled )
			{
				if ( g_ctx->can_interact( this, focus_priority::interactive ) || g_ctx->m_modal_owner == this )
				{
					if ( g_input->mouse_in_region( c_vector_2d( m_toggle.x - 4.f, m_toggle.y - 6.f ), c_vector_2d( 37.f, 30.f ) ) && g_input->clicked( mouse_buttons::left ) )
					{
						*m_enabled = !*m_enabled;
						return;
					}
				}
			}

			const bool can = g_ctx->can_interact( this, m_focus_priority ) || g_ctx->m_modal_owner == this;
			if ( !can )
				return;

			const bool hit_swatch = g_input->mouse_in_region( m_swatch, c_vector_2d( k_swatch, k_swatch ) );
			const bool hit_row = g_input->mouse_in_region( m_pos, c_vector_2d( width, g_style->row_height ) )
				&& !( m_enabled && g_input->mouse_in_region( c_vector_2d( m_toggle.x - 4.f, m_toggle.y - 6.f ), c_vector_2d( 37.f, 30.f ) ) );

			if ( ( hit_swatch || hit_row ) && g_input->clicked( mouse_buttons::left ) )
			{
				if ( !m_open )
				{
					m_open = true;
					m_ignore_click = true;
					sync_hsv_from_value( );
					g_ctx->set_modal( this );
				}
				else if ( hit_swatch )
				{
					m_open = false;
					g_ctx->clear_modal( this );
				}
				return;
			}

			if ( !m_open )
				return;

			if ( m_ignore_click )
			{
				if ( !g_input->click_down( mouse_buttons::left ) )
					m_ignore_click = false;
				return;
			}

			const c_vector_2d pp = popup_pos( );
			const c_vector_2d psz = popup_size( );
			const c_vector_2d sv( pp.x + k_pad, pp.y + k_pad );
			const float hue_y = sv.y + k_sv + k_gap;
			const float alpha_y = hue_y + k_bar_h + k_gap;
			const float hit = 6.f;

			if ( g_input->click_down( mouse_buttons::left ) )
			{
				if ( m_drag == drag_mode::sv || g_input->mouse_in_region( sv, c_vector_2d( k_sv, k_sv ) ) )
				{
					const c_vector_2d m = g_input->get_mouse_position( );
					m_sat = ImClamp( ( m.x - sv.x ) / ( k_sv - 1.f ), 0.f, 1.f );
					m_val = 1.f - ImClamp( ( m.y - sv.y ) / ( k_sv - 1.f ), 0.f, 1.f );
					m_drag = drag_mode::sv;
					write_color( );
				}
				else if ( m_drag == drag_mode::hue || g_input->mouse_in_region( c_vector_2d( sv.x, hue_y - hit ), c_vector_2d( k_sv, k_bar_h + hit * 2.f ) ) )
				{
					m_hue = ImClamp( ( g_input->get_mouse_position( ).x - sv.x ) / ( k_sv - 1.f ), 0.f, 1.f ) * 360.f;
					m_drag = drag_mode::hue;
					write_color( );
				}
				else if ( m_drag == drag_mode::alpha || g_input->mouse_in_region( c_vector_2d( sv.x, alpha_y - hit ), c_vector_2d( k_sv, k_bar_h + hit * 2.f ) ) )
				{
					m_alpha = ImClamp( ( g_input->get_mouse_position( ).x - sv.x ) / ( k_sv - 1.f ), 0.f, 1.f );
					m_drag = drag_mode::alpha;
					write_color( );
				}
			}
			else
			{
				m_drag = drag_mode::none;
			}

			if ( g_input->clicked( mouse_buttons::left ) )
			{
				if ( !g_input->mouse_in_region( pp, psz ) && !g_input->mouse_in_region( m_swatch, c_vector_2d( k_swatch, k_swatch ) ) )
				{
					m_open = false;
					g_ctx->clear_modal( this );
				}
			}
		}

	private:
		// Neverlose-style picker: rounded SV, horizontal hue + alpha bars, 15px swatch.
		static constexpr float k_swatch = 22.f;
		static constexpr float k_pad = 10.f;
		static constexpr float k_sv = 176.f;
		static constexpr float k_bar_h = 10.f;
		static constexpr float k_gap = 8.f;
		static constexpr float k_round = 4.f;
		static constexpr float k_bar_round = 4.f;
		static constexpr float k_panel_round = 8.f;

		enum class drag_mode : int { none, sv, hue, alpha };

		c_color* m_value { nullptr };
		bool* m_enabled { nullptr };
		float m_toggle_anim { 0.f };
		float m_swatch_anim { 1.f };
		float m_open_anim { 0.f };
		bool m_open { false };
		bool m_ignore_click { false };
		c_vector_2d m_swatch {};
		c_vector_2d m_toggle {};
		float m_hue { 0.f };
		float m_sat { 1.f };
		float m_val { 1.f };
		float m_alpha { 1.f };
		drag_mode m_drag { drag_mode::none };

		void layout_controls( float width )
		{
			const float row_h = g_style->row_height;
			const float cw = g_style->control_w_for( width );
			const float cx = g_style->control_x( m_pos.x, width );
			const float sy = std::floor( m_pos.y + ( row_h - k_swatch ) * 0.5f );

			if ( m_enabled )
			{
				m_toggle = c_vector_2d( std::floor( g_style->toggle_x( m_pos.x, width ) ), std::floor( m_pos.y + ( row_h - g_style->toggle_h ) * 0.5f ) );
				m_swatch = c_vector_2d( std::floor( m_toggle.x - 8.f - k_swatch ), sy );
			}
			else
			{
				m_swatch = c_vector_2d( std::floor( cx + cw - k_swatch ), sy );
			}
		}

		c_vector_2d popup_size( ) const
		{
			return c_vector_2d( k_pad * 2.f + k_sv, k_pad + k_sv + k_gap + k_bar_h + k_gap + k_bar_h + k_pad );
		}

		c_vector_2d popup_pos( ) const
		{
			const c_vector_2d psz = popup_size( );
			float x = std::floor( m_swatch.x + k_swatch - psz.x );
			float y = std::floor( m_swatch.y + k_swatch + 8.f );
			const ImVec2 display = ImGui::GetIO( ).DisplaySize;
			if ( x < 8.f )
				x = 8.f;
			if ( x + psz.x > display.x - 8.f )
				x = display.x - psz.x - 8.f;
			if ( y + psz.y > display.y - 8.f )
				y = std::floor( m_swatch.y - psz.y - 8.f );
			return c_vector_2d( x, y );
		}

		void sync_hsv_from_value( )
		{
			if ( !m_value )
				return;
			m_value->to_hsv( m_hue, m_sat, m_val );
			m_alpha = static_cast<float>( m_value->a ) / 255.f;
		}

		void write_color( )
		{
			if ( !m_value )
				return;
			*m_value = c_color::from_hsv( m_hue, m_sat, m_val, static_cast<int>( m_alpha * 255.f + 0.5f ) );
		}

		void seal_corners( ImDrawList* dl, ImVec2 a, ImVec2 b, float rounding, ImU32 bg, ImDrawFlags corners = ImDrawFlags_RoundCornersAll )
		{
			if ( !dl || rounding <= 0.f )
				return;
			const float tl = ( corners & ImDrawFlags_RoundCornersTopLeft ) ? rounding : 0.f;
			const float tr = ( corners & ImDrawFlags_RoundCornersTopRight ) ? rounding : 0.f;
			const float br = ( corners & ImDrawFlags_RoundCornersBottomRight ) ? rounding : 0.f;
			const float bl = ( corners & ImDrawFlags_RoundCornersBottomLeft ) ? rounding : 0.f;

			if ( tl > 0.f ) { dl->PathLineTo( a ); dl->PathArcTo( ImVec2( a.x + tl, a.y + tl ), tl, 4.820f, 3.100f ); dl->PathFillConvex( bg ); }
			if ( tr > 0.f ) { dl->PathLineTo( ImVec2( b.x, a.y ) ); dl->PathArcTo( ImVec2( b.x - tr, a.y + tr ), tr, 6.340f, 4.620f ); dl->PathFillConvex( bg ); }
			if ( br > 0.f ) { dl->PathLineTo( b ); dl->PathArcTo( ImVec2( b.x - br, b.y - br ), br, 7.960f, 6.240f ); dl->PathFillConvex( bg ); }
			if ( bl > 0.f ) { dl->PathLineTo( ImVec2( a.x, b.y ) ); dl->PathArcTo( ImVec2( a.x + bl, b.y - bl ), bl, 9.5f, 7.770f ); dl->PathFillConvex( bg ); }
		}

		void draw_swatch( )
		{
			const c_color col = m_value ? *m_value : g_style->accent;
			const float reveal = c_render::ease_in_out_quint( m_swatch_anim );
			const float r = 5.f;
			const int a = static_cast<int>( static_cast<float>( col.a ) * reveal + 0.5f );
			g_render->alpha_rect(
				static_cast<int>( m_swatch.x ),
				static_cast<int>( m_swatch.y ),
				static_cast<int>( k_swatch ),
				static_cast<int>( k_swatch ),
				c_color( col.r, col.g, col.b, a ),
				r,
				c_color( 17, 19, 27 ),
				true );
			g_render->rect_f(
				m_swatch.x,
				m_swatch.y,
				k_swatch,
				k_swatch,
				g_style->accent.lerp( c_color( 255, 255, 255 ), m_open_anim * 0.35f ).with_alpha( static_cast<int>( 180.f + 75.f * m_open_anim ) ),
				r );
		}

		void draw_sv_cursor( ImVec2 pos, bool active )
		{
			ImDrawList* dl = g_render->draw_list( );
			if ( !dl )
				return;
			const float rad = active ? 6.f : 4.5f;
			dl->AddCircle( ImVec2( pos.x + 1.f, pos.y + 1.f ), rad, IM_COL32( 0, 0, 0, 70 ), 30 );
			dl->AddCircle( pos, rad, IM_COL32( 255, 255, 255, 255 ), 30 );
		}

		void draw_bar_knob( ImVec2 pos, ImU32 fill )
		{
			ImDrawList* dl = g_render->draw_list( );
			if ( !dl )
				return;
			dl->AddCircleFilled( pos, 6.f, fill, 32 );
			dl->AddCircle( pos, 6.f, IM_COL32( 255, 255, 255, 200 ), 32, 1.2f );
		}

		void draw_sv( c_vector_2d p, float size )
		{
			ImDrawList* dl = g_render->draw_list( );
			if ( !dl )
				return;

			const c_color hue = c_color::from_hsv( m_hue, 1.f, 1.f );
			const ImU32 hue32 = IM_COL32( hue.r, hue.g, hue.b, 255 );
			const ImU32 white = IM_COL32( 255, 255, 255, 255 );
			const ImU32 black = IM_COL32( 0, 0, 0, 255 );
			const ImU32 plate = IM_COL32( 20, 20, 29, 255 );
			const ImVec2 a( p.x, p.y );
			const ImVec2 b( p.x + size, p.y + size );

			dl->AddRectFilledMultiColor( a, b, white, hue32, hue32, white );
			seal_corners( dl, a, b, k_round, plate );
			dl->AddRectFilledMultiColor( a, b, 0, 0, black, black );
			seal_corners( dl, a, b, k_round, plate );

			const ImVec2 cursor(
				ImClamp( IM_ROUND( p.x + ImSaturate( m_sat ) * size ), p.x + 2.f, p.x + size - 2.f ),
				ImClamp( IM_ROUND( p.y + ImSaturate( 1.f - m_val ) * size ), p.y + 2.f, p.y + size - 2.f ) );
			draw_sv_cursor( cursor, m_drag == drag_mode::sv );
		}

		void draw_hue_bar( c_vector_2d p, float w )
		{
			ImDrawList* dl = g_render->draw_list( );
			if ( !dl )
				return;

			const float h = k_bar_h;
			const ImU32 plate = IM_COL32( 20, 20, 29, 255 );
			const ImU32 hues[7] = {
				IM_COL32( 255, 0, 0, 255 ), IM_COL32( 255, 255, 0, 255 ), IM_COL32( 0, 255, 0, 255 ),
				IM_COL32( 0, 255, 255, 255 ), IM_COL32( 0, 0, 255, 255 ), IM_COL32( 255, 0, 255, 255 ), IM_COL32( 255, 0, 0, 255 )
			};
			const float step = w / 6.f;

			{
				const ImVec2 a( p.x, p.y );
				const ImVec2 b( p.x + step, p.y + h );
				dl->AddRectFilledMultiColor( a, b, hues[0], hues[1], hues[1], hues[0] );
				seal_corners( dl, a, b, k_bar_round, plate, ImDrawFlags_RoundCornersLeft );
			}
			for ( int i = 1; i < 5; ++i )
			{
				dl->AddRectFilledMultiColor(
					ImVec2( p.x + step * i, p.y ),
					ImVec2( p.x + step * ( i + 1 ), p.y + h ),
					hues[i], hues[i + 1], hues[i + 1], hues[i] );
			}
			{
				const ImVec2 a( p.x + step * 5.f, p.y );
				const ImVec2 b( p.x + w, p.y + h );
				dl->AddRectFilledMultiColor( a, b, hues[5], hues[6], hues[6], hues[5] );
				seal_corners( dl, a, b, k_bar_round, plate, ImDrawFlags_RoundCornersRight );
			}

			const c_color hue_col = c_color::from_hsv( m_hue, 1.f, 1.f );
			draw_bar_knob(
				ImVec2( IM_ROUND( p.x + ( m_hue / 360.f ) * w ), p.y + h * 0.5f ),
				IM_COL32( hue_col.r, hue_col.g, hue_col.b, 255 ) );
		}

		void draw_alpha_bar( c_vector_2d p, float w )
		{
			ImDrawList* dl = g_render->draw_list( );
			if ( !dl )
				return;

			const float h = k_bar_h;
			const c_color solid = c_color::from_hsv( m_hue, m_sat, m_val, 255 );
			const ImU32 plate = IM_COL32( 20, 20, 29, 255 );

			g_render->checkerboard_rounded(
				static_cast<int>( p.x ), static_cast<int>( p.y ),
				static_cast<int>( w ), static_cast<int>( h ),
				k_bar_round, 4,
				c_color( 58, 60, 68 ), c_color( 36, 38, 46 ) );

			const ImVec2 a( p.x, p.y );
			const ImVec2 b( p.x + w, p.y + h );
			dl->AddRectFilledMultiColor(
				a, b,
				IM_COL32( solid.r, solid.g, solid.b, 0 ),
				IM_COL32( solid.r, solid.g, solid.b, 255 ),
				IM_COL32( solid.r, solid.g, solid.b, 255 ),
				IM_COL32( solid.r, solid.g, solid.b, 0 ) );
			seal_corners( dl, a, b, k_bar_round, plate );

			draw_bar_knob(
				ImVec2( IM_ROUND( p.x + m_alpha * w ), p.y + h * 0.5f ),
				IM_COL32( solid.r, solid.g, solid.b, 255 ) );
		}

		void draw_popup( )
		{
			const c_vector_2d pp = popup_pos( );
			const c_vector_2d psz = popup_size( );
			const int vtx = g_render->vtx_count( );
			const c_vector_2d pivot( m_swatch.x + k_swatch * 0.5f, m_swatch.y + k_swatch * 0.5f );

			g_render->frosted_panel( pp.x, pp.y, psz.x, psz.y, k_panel_round, true );

			const c_vector_2d sv( pp.x + k_pad, pp.y + k_pad );
			draw_sv( sv, k_sv );
			draw_hue_bar( c_vector_2d( pp.x + k_pad, sv.y + k_sv + k_gap ), k_sv );
			draw_alpha_bar( c_vector_2d( pp.x + k_pad, sv.y + k_sv + k_gap + k_bar_h + k_gap ), k_sv );

			g_render->apply_open( vtx, pivot, m_open_anim, 0.88f, 16.f );
		}
	};
}
