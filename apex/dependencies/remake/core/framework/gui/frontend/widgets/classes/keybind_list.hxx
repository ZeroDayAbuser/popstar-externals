#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include <core/framework/gui/backend/blur/blur.hxx>
#include <core/framework/gui/backend/manager/keybinds/keybinds.hxx>
#include <core/framework/gui/backend/math/math.hxx>
#include <core/framework/gui/backend/render/render.hxx>
#include <core/framework/gui/frontend/widgets/classes/bind_popup.hxx>
#include <core/framework/gui/frontend/widgets/settings.hxx>

namespace core::gui
{
	// On-screen bind strip. Painted in the main pass (same acrylic path as c_window).
	class c_keybind_list
	{
	public:
		void set_enabled( bool* enabled ) { m_enabled = enabled; }
		void set_only_active( bool* only_active ) { m_only_active = only_active; }

		void paint( )
		{
			if ( !m_enabled || !*m_enabled || !g_render || !g_style )
				return;

			collect( );
			const float dt = ImGui::GetIO( ).DeltaTime;
			const float target = 1.f;
			m_open_anim += ( target - m_open_anim ) * ( 1.f - std::exp( -5.2f * dt ) );
			if ( std::fabs( m_open_anim - target ) < 0.0008f )
				m_open_anim = target;
			if ( m_open_anim < 0.01f )
				return;

			for ( row_t& row : m_rows )
				tick_row_anim( row.label, row.active, dt );

			layout( );
			handle_drag( );
			draw_panel( );
		}

	private:
		struct row_t
		{
			std::string label {};
			std::string key_label {};
			bool active { false };
		};

		bool* m_enabled { nullptr };
		bool* m_only_active { nullptr };
		c_vector_2d m_pos { 18.f, 18.f };
		c_vector_2d m_size {};
		float m_open_anim { 0.f };
		bool m_dragging { false };
		c_vector_2d m_drag_off {};
		std::vector<row_t> m_rows {};
		std::vector<std::pair<std::string, float>> m_row_anim {};

		static constexpr float k_w = 200.f;
		static constexpr float k_pad = 12.f;
		static constexpr float k_header = 32.f;
		static constexpr float k_row = 26.f;
		static constexpr float k_round = 8.f;

		bool only_active( ) const
		{
			return m_only_active && *m_only_active;
		}

		float row_anim( const std::string& label ) const
		{
			for ( const auto& a : m_row_anim )
			{
				if ( a.first == label )
					return a.second;
			}
			return 0.f;
		}

		void tick_row_anim( const std::string& label, bool active, float dt )
		{
			for ( auto& a : m_row_anim )
			{
				if ( a.first != label )
					continue;
				a.second += ( ( active ? 1.f : 0.f ) - a.second ) * ( 1.f - std::exp( -14.f * dt ) );
				return;
			}
			m_row_anim.push_back( { label, active ? 1.f : 0.f } );
		}

		void push_row( std::string label, const key_var_t& key, bool active )
		{
			if ( key.key <= 0 && key.mode != key_mode_t::always )
				return;
			if ( only_active( ) && !active )
				return;

			char key_buf[48] {};
			if ( key.mode == key_mode_t::always )
				std::snprintf( key_buf, sizeof( key_buf ), "ON" );
			else
			{
				const char* n = key.name( );
				// Uppercase mouse / short keys for the chip
				char tmp[32] {};
				std::snprintf( tmp, sizeof( tmp ), "%s", n );
				for ( char* p = tmp; *p; ++p )
				{
					if ( *p >= 'a' && *p <= 'z' )
						*p = static_cast<char>( *p - 32 );
				}
				std::snprintf( key_buf, sizeof( key_buf ), "%s", tmp );
			}

			for ( row_t& existing : m_rows )
			{
				if ( existing.label == label )
				{
					existing.active = existing.active || active;
					if ( active )
						existing.key_label = key_buf;
					return;
				}
			}
			m_rows.push_back( row_t { std::move( label ), key_buf, active } );
		}

		void collect( )
		{
			m_rows.clear( );

			if ( g_keybinds )
			{
				for ( const named_bind_t& b : g_keybinds->binds( ) )
				{
					if ( !b.key )
						continue;
					const char* label = b.label.empty( ) ? "Bind" : b.label.c_str( );
					push_row( label, *b.key, b.key->active( ) );
				}
			}

			if ( g_bind_hub )
			{
				for ( c_bind_popup* p : g_bind_hub->popups( ) )
				{
					if ( !p )
						continue;
					for ( const config_bind_t& b : p->binds( ) )
					{
						if ( b.removing )
							continue;
						push_row( p->slot( ).label, b.key, b.key.active( ) || b.applied );
					}
				}
			}

			std::sort( m_rows.begin( ), m_rows.end( ), []( const row_t& a, const row_t& b )
			{
				if ( a.active != b.active )
					return a.active > b.active;
				return a.label < b.label;
			} );
		}

		void layout( )
		{
			const float body = static_cast<float>( ( std::max )( std::size_t { 1 }, m_rows.size( ) ) ) * k_row;
			m_size = c_vector_2d( k_w, k_header + body + k_pad );

			const ImVec2 display = ImGui::GetIO( ).DisplaySize;
			m_pos.x = ImClamp( m_pos.x, 8.f, ( std::max )( 8.f, display.x - m_size.x - 8.f ) );
			m_pos.y = ImClamp( m_pos.y, 8.f, ( std::max )( 8.f, display.y - m_size.y - 8.f ) );
		}

		void handle_drag( )
		{
			if ( !g_input )
				return;

			const bool over = g_input->mouse_in_region( m_pos, c_vector_2d( m_size.x, k_header ) );
			if ( over && g_input->clicked( mouse_buttons::left ) )
			{
				m_dragging = true;
				m_drag_off = g_input->get_mouse_position( ) - m_pos;
			}
			if ( m_dragging )
			{
				if ( g_input->click_down( mouse_buttons::left ) )
					m_pos = g_input->get_mouse_position( ) - m_drag_off;
				else
					m_dragging = false;
			}
		}

		void draw_key_chip( float right, float cy, const char* text, float act, float e )
		{
			const c_vector_2d ts = g_render->measure_text( c_fonts::k_caption_key, text, 11.f );
			const float pw = ( std::max )( 34.f, ts.x + 14.f );
			const float ph = 18.f;
			const float px = right - pw;
			const float py = cy - ph * 0.5f;

			const c_color fill = c_color( 22, 25, 34, 200 ).lerp( g_style->accent.with_alpha( 70 ), act );
			const c_color border = c_color( 48, 52, 64, 180 ).lerp( g_style->accent, act * 0.8f );
			const c_color text_col = c_color( 200, 204, 214 ).lerp( g_style->text_bright, act );

			g_render->rect_filled_f( px, py, pw, ph, fill.with_alpha( static_cast<int>( fill.a * e ) ), 5.f );
			g_render->rect_f( px, py, pw, ph, border.with_alpha( static_cast<int>( 200.f * e ) ), 5.f );
			g_render->text(
				c_fonts::k_caption_key,
				c_vector_2d( px + std::floor( ( pw - ts.x ) * 0.5f ), py + std::floor( ( ph - ts.y ) * 0.5f ) ),
				text_col.with_alpha( static_cast<int>( 255.f * e ) ),
				text,
				11.f );
		}

		void draw_panel( )
		{
			const float e = c_render::ease_in_out_quint( m_open_anim );
			const float x = m_pos.x;
			const float y = m_pos.y;
			const float w = m_size.x;
			const float h = m_size.y;
			const c_vector_2d bmin( x, y );
			const c_vector_2d bmax( x + w, y + h );
			const int vtx = g_render->vtx_count( );

			g_render->set_alpha( e );

			// Exact same acrylic stack as the main Nirvana window.
			if ( g_blur && g_blur->ready( ) )
			{
				if ( ImDrawList* dl = g_render->draw_list( ) )
					g_blur->draw_region( dl, bmin, bmax, k_round, c_color( 255, 255, 255, 255 ) );
			}

			g_render->rect_filled_f( x, y, w, h, g_style->window_bg, k_round );
			g_render->stroke_rounded( x, y, w, h, k_round, g_style->popup_border, 1.f );

			const char* title = "Keybinds";
			const c_vector_2d title_ts = g_render->measure_text( c_fonts::k_default_key, title, g_style->text_body );
			g_render->text(
				c_fonts::k_default_key,
				c_vector_2d( x + k_pad, y + std::floor( ( k_header - title_ts.y ) * 0.5f ) ),
				g_style->text_bright,
				title,
				g_style->text_body );

			g_render->rect_filled_f(
				x + k_pad,
				y + k_header - 6.f,
				18.f,
				2.f,
				g_style->accent,
				1.f );

			g_render->line(
				c_vector_2d( x + k_pad, y + k_header ),
				c_vector_2d( x + w - k_pad, y + k_header ),
				c_color( 48, 52, 64, 140 ) );

			float row_y = y + k_header;
			if ( m_rows.empty( ) )
			{
				g_render->text(
					c_fonts::k_default_key,
					c_vector_2d( x + k_pad, row_y + std::floor( ( k_row - g_style->text_control ) * 0.5f ) ),
					g_style->text_dim,
					"No binds",
					g_style->text_control );
			}
			else
			{
				for ( std::size_t i = 0; i < m_rows.size( ); ++i )
				{
					const row_t& row = m_rows[i];
					const float act = row_anim( row.label );
					const float cy = row_y + k_row * 0.5f;

					if ( act > 0.02f )
					{
						g_render->rect_filled_f(
							x + 1.f,
							row_y,
							2.f,
							k_row,
							g_style->accent.with_alpha( static_cast<int>( 200.f * act ) ),
							0.f );
					}

					const c_color name_col = g_style->text.lerp( g_style->text_bright, act * 0.5f );
					const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, row.label.c_str( ), g_style->text_control );
					g_render->text(
						c_fonts::k_default_key,
						c_vector_2d( x + k_pad + ( act > 0.02f ? 4.f : 0.f ), cy - ts.y * 0.5f ),
						name_col,
						row.label.c_str( ),
						g_style->text_control );

					draw_key_chip( x + w - k_pad, cy, row.key_label.c_str( ), act, 1.f );
					row_y += k_row;
				}
			}

			g_render->apply_open( vtx, c_vector_2d( x + 12.f, y + 10.f ), m_open_anim, 0.94f, 10.f );
			g_render->set_alpha( 1.f );
		}
	};

	inline std::shared_ptr<c_keybind_list> g_keybind_list = std::make_shared<c_keybind_list>( );
}
