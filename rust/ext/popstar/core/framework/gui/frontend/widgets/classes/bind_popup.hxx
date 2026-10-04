#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <core/framework/gui/backend/manager/keybinds/keybinds.hxx>
#include <core/framework/gui/backend/math/math.hxx>
#include <core/framework/gui/backend/render/render.hxx>
#include <core/framework/gui/frontend/widgets/classes/base_element.hxx>
#include <core/framework/gui/frontend/widgets/settings.hxx>

namespace core::gui
{
	enum class bind_kind : int
	{
		boolean,
		integer,
		floating
	};

	struct bind_slot_t
	{
		std::string label {};
		bind_kind kind { bind_kind::boolean };
		void* ptr { nullptr };
		float min_f { 0.f };
		float max_f { 1.f };
		int min_i { 0 };
		int max_i { 1 };
		std::vector<std::string> items {};

		bool* as_bool( ) const { return kind == bind_kind::boolean ? static_cast<bool*>( ptr ) : nullptr; }
		int* as_int( ) const { return kind == bind_kind::integer ? static_cast<int*>( ptr ) : nullptr; }
		float* as_float( ) const { return kind == bind_kind::floating ? static_cast<float*>( ptr ) : nullptr; }

		bool read_bool( ) const
		{
			bool* p = as_bool( );
			return p ? *p : false;
		}

		int read_int( ) const
		{
			int* p = as_int( );
			return p ? *p : 0;
		}

		float read_float( ) const
		{
			float* p = as_float( );
			return p ? *p : 0.f;
		}

		void write_bool( bool v ) const
		{
			if ( bool* p = as_bool( ) )
				*p = v;
		}

		void write_int( int v ) const
		{
			if ( int* p = as_int( ) )
				*p = ( std::max )( min_i, ( std::min )( max_i, v ) );
		}

		void write_float( float v ) const
		{
			if ( float* p = as_float( ) )
				*p = ImClamp( v, min_f, max_f );
		}

		void capture_default( )
		{
			switch ( kind )
			{
			case bind_kind::boolean: m_def_b = read_bool( ); break;
			case bind_kind::integer: m_def_i = read_int( ); break;
			case bind_kind::floating: m_def_f = read_float( ); break;
			}
			m_has_default = true;
		}

		void reset_default( ) const
		{
			if ( !m_has_default )
				return;
			switch ( kind )
			{
			case bind_kind::boolean: write_bool( m_def_b ); break;
			case bind_kind::integer: write_int( m_def_i ); break;
			case bind_kind::floating: write_float( m_def_f ); break;
			}
		}

		bool m_has_default { false };
		bool m_def_b { false };
		int m_def_i { 0 };
		float m_def_f { 0.f };
	};

	struct config_bind_t
	{
		key_var_t key {};
		bool b_val { true };
		int i_val { 0 };
		float f_val { 0.f };

		bool applied { false };
		bool b_restore { false };
		int i_restore { 0 };
		float f_restore { 0.f };

		float anim { 0.f };
		float remove_t { 0.f };
		bool removing { false };
	};

	class c_bind_popup;

	class c_bind_hub
	{
	public:
		void register_popup( c_bind_popup* p )
		{
			if ( !p )
				return;
			for ( c_bind_popup* e : m_popups )
			{
				if ( e == p )
					return;
			}
			m_popups.push_back( p );
		}

		void unregister_popup( c_bind_popup* p )
		{
			m_popups.erase( std::remove( m_popups.begin( ), m_popups.end( ), p ), m_popups.end( ) );
		}

		void update( );
		const std::vector<c_bind_popup*>& popups( ) const { return m_popups; }

	private:
		std::vector<c_bind_popup*> m_popups {};
	};

	inline std::shared_ptr<c_bind_hub> g_bind_hub = std::make_shared<c_bind_hub>( );

	class c_bind_popup : public c_base_element
	{
	public:
		static constexpr float dots_width( ) { return 28.f; }
		static constexpr float dots_height( ) { return 22.f; }
		static constexpr float k_menu_w = 168.f;
		static constexpr float k_bind_row = 26.f;
		static constexpr float k_action_row = 30.f;
		static constexpr float k_pad = 8.f;
		static constexpr float k_editor_w = 220.f;
		static constexpr float k_label_col = 50.f;
		static constexpr float k_hotkeys_w = 240.f;
		static constexpr float k_sep_gap = 6.f;

		c_bind_popup( bind_slot_t slot )
			: m_slot( std::move( slot ) )
		{
			m_label = m_slot.label;
			m_type = element_type::popup;
			m_visible = true;
			m_layer = render_layer::modal;
			m_focus_priority = focus_priority::modal;
			m_size = c_vector_2d( 200.f, g_style ? g_style->row_height : 37.f );
			m_slot.capture_default( );
			seed_override_from_slot( );
			if ( g_bind_hub )
				g_bind_hub->register_popup( this );
		}

		~c_bind_popup( ) override
		{
			if ( g_bind_hub )
				g_bind_hub->unregister_popup( this );
		}

		void set_dots_anchor( c_vector_2d p )
		{
			m_dots = p;
			m_anchored = true;
		}

		bind_slot_t& slot( ) { return m_slot; }
		const bind_slot_t& slot( ) const { return m_slot; }
		std::vector<config_bind_t>& binds( ) { return m_binds; }
		const std::vector<config_bind_t>& binds( ) const { return m_binds; }

		bool hosts( c_base_element* element ) const override
		{
			return element == this;
		}

		void close_overlay( ) override
		{
			force_close( );
		}

		void play_intro( ) override
		{
		}

		void apply_runtime( )
		{
			for ( config_bind_t& b : m_binds )
			{
				if ( b.removing || ( b.key.key <= 0 && b.key.mode != key_mode_t::always ) )
				{
					if ( b.applied )
						restore_bind( b );
					continue;
				}

				b.key.update( );
				const bool on = b.key.active( );
				if ( on && !b.applied )
					apply_bind( b );
				else if ( !on && b.applied )
					restore_bind( b );
			}
		}

		void draw( ) override
		{
			if ( !is_visible( ) || !g_render || !g_style )
				return;

			const float dt = ImGui::GetIO( ).DeltaTime;
			tick_anims( dt );

			const bool hovered = g_input && g_input->mouse_in_region( m_dots, c_vector_2d( dots_width( ), dots_height( ) ) );
			m_hover += ( ( hovered || m_open ? 1.f : 0.f ) - m_hover ) * ( 1.f - std::exp( -18.f * dt ) );
			if ( g_ctx && !g_ctx->m_open )
			{
				m_open = false;
				m_open_anim = 0.f;
				m_editor_anim = 0.f;
				m_hotkeys_anim = 0.f;
			}
			else
			{
				m_open_anim += ( ( m_open ? 1.f : 0.f ) - m_open_anim ) * ( 1.f - std::exp( -3.4f * dt ) );
				if ( std::fabs( m_open_anim - ( m_open ? 1.f : 0.f ) ) < 0.0008f )
					m_open_anim = m_open ? 1.f : 0.f;

				m_editor_anim += ( ( m_selected >= 0 && m_open ? 1.f : 0.f ) - m_editor_anim ) * ( 1.f - std::exp( -3.4f * dt ) );
				m_hotkeys_anim += ( ( m_hotkeys_open && m_open ? 1.f : 0.f ) - m_hotkeys_anim ) * ( 1.f - std::exp( -3.4f * dt ) );
			}

			const float cy = m_dots.y + dots_height( ) * 0.5f;
			const float cx = m_dots.x + dots_width( ) * 0.5f;
			const c_color dot = c_color( 170, 174, 186 ).lerp( g_style->text_bright, m_hover ).lerp( g_style->accent, m_open_anim * 0.65f );
			for ( int i = -1; i <= 1; ++i )
				g_render->circle_filled( c_vector_2d( cx + static_cast<float>( i ) * 5.f, cy ), 1.7f, dot, 12 );

			if ( m_open_anim > 0.01f && g_ctx && g_ctx->m_open && g_ctx->m_open_anim > 0.05f )
				g_render->defer( [this]( ) { draw_panels( ); } );
		}

		void input( ) override
		{
			if ( !is_visible( ) || !g_input || !g_ctx || !g_style )
				return;

			const bool can = g_ctx->can_interact( this, m_focus_priority ) || g_ctx->m_modal_owner == this;
			if ( !can )
				return;

			if ( g_input->mouse_in_region( m_dots, c_vector_2d( dots_width( ), dots_height( ) ) ) && g_input->clicked( mouse_buttons::left ) )
			{
				if ( !m_open )
				{
					m_open = true;
					m_ignore_click = true;
					m_hotkeys_open = false;
					g_ctx->set_modal( this, [this]( c_base_element* el ) { return hosts( el ); } );
				}
				else
				{
					force_close( );
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

			handle_menu_input( );
			if ( m_selected >= 0 && m_selected < static_cast<int>( m_binds.size( ) ) )
				handle_editor_input( );
			if ( m_hotkeys_open )
				handle_hotkeys_input( );

			if ( g_input->clicked( mouse_buttons::left ) )
			{
				const c_vector_2d mp = menu_pos( );
				const c_vector_2d ms = menu_size( );
				bool inside = g_input->mouse_in_region( mp, ms );
				if ( m_editor_anim > 0.2f )
					inside = inside || g_input->mouse_in_region( editor_pos( ), editor_size( ) );
				if ( m_hotkeys_anim > 0.2f )
					inside = inside || g_input->mouse_in_region( hotkeys_pos( ), hotkeys_size( ) );
				if ( !inside && !g_input->mouse_in_region( m_dots, c_vector_2d( dots_width( ), dots_height( ) ) ) )
					force_close( );
			}
		}

	private:
		bind_slot_t m_slot {};
		std::vector<config_bind_t> m_binds {};
		c_vector_2d m_dots {};
		bool m_anchored { false };
		bool m_open { false };
		bool m_ignore_click { false };
		bool m_listening { false };
		bool m_hotkeys_open { false };
		int m_selected { -1 };
		float m_hover { 0.f };
		float m_open_anim { 0.f };
		float m_editor_anim { 0.f };
		float m_hotkeys_anim { 0.f };
		float m_mode_hover[3] {};
		float m_action_hover[3] {};
		float m_sel_y { 0.f };
		float m_sel_h { k_bind_row };
		float m_sel_a { 0.f };
		bool m_sel_init { false };
		float m_slider_anim { 0.f };
		float m_knob_anim { 0.f };
		float m_tip_anim { 0.f };
		bool m_slider_drag { false };

		void seed_override_from_slot( )
		{
			m_seed_b = m_slot.read_bool( );
			m_seed_i = m_slot.read_int( );
			m_seed_f = m_slot.read_float( );
		}

		bool m_seed_b { true };
		int m_seed_i { 0 };
		float m_seed_f { 0.f };

		void force_close( )
		{
			m_open = false;
			m_listening = false;
			m_hotkeys_open = false;
			m_ignore_click = false;
			m_selected = -1;
			if ( g_ctx )
				g_ctx->clear_modal( this );
		}

		void tick_anims( float dt )
		{
			for ( std::size_t i = 0; i < m_binds.size( ); )
			{
				config_bind_t& b = m_binds[i];
				if ( b.removing )
				{
					b.remove_t += ( 1.f - b.remove_t ) * ( 1.f - std::exp( -16.f * dt ) );
					b.anim += ( 0.f - b.anim ) * ( 1.f - std::exp( -16.f * dt ) );
					if ( b.remove_t > 0.97f )
					{
						if ( b.applied )
							restore_bind( b );
						if ( m_selected == static_cast<int>( i ) )
							m_selected = -1;
						else if ( m_selected > static_cast<int>( i ) )
							--m_selected;
						m_binds.erase( m_binds.begin( ) + static_cast<std::ptrdiff_t>( i ) );
						continue;
					}
				}
				else
				{
					b.anim += ( 1.f - b.anim ) * ( 1.f - std::exp( -14.f * dt ) );
				}
				++i;
			}

			const c_vector_2d mp = menu_pos( );
			float y = mp.y + k_pad;
			float target_y = y;
			float target_h = k_bind_row - 2.f;
			bool found = false;
			int idx = 0;
			for ( const config_bind_t& b : m_binds )
			{
				const float e = c_render::smoothstep( b.anim ) * ( 1.f - c_render::smoothstep( b.remove_t ) );
				if ( e > 0.01f )
				{
					const float row_h = k_bind_row * e;
					if ( idx == m_selected )
					{
						target_y = y + 1.f;
						target_h = ( std::max )( 18.f, row_h - 2.f );
						found = true;
					}
					y += row_h;
				}
				++idx;
			}

			const float sel_t = found ? 1.f : 0.f;
			m_sel_a += ( sel_t - m_sel_a ) * ( 1.f - std::exp( -18.f * dt ) );
			if ( !m_sel_init && found )
			{
				m_sel_y = target_y;
				m_sel_h = target_h;
				m_sel_init = true;
			}
			else
			{
				m_sel_y += ( target_y - m_sel_y ) * ( 1.f - std::exp( -20.f * dt ) );
				m_sel_h += ( target_h - m_sel_h ) * ( 1.f - std::exp( -20.f * dt ) );
			}
			if ( !found && m_sel_a < 0.01f )
				m_sel_init = false;

			if ( m_selected >= 0 && m_selected < static_cast<int>( m_binds.size( ) ) && m_slot.kind == bind_kind::floating )
			{
				const float span = m_slot.max_f - m_slot.min_f;
				const float target = span > 0.f ? ImClamp( ( m_binds[m_selected].f_val - m_slot.min_f ) / span, 0.f, 1.f ) : 0.f;
				m_slider_anim += ( target - m_slider_anim ) * ( 1.f - std::exp( -16.f * dt ) );
				m_knob_anim += ( ( m_slider_drag ? 1.f : 0.f ) - m_knob_anim ) * ( 1.f - std::exp( -22.f * dt ) );
				m_tip_anim += ( ( m_slider_drag ? 1.f : 0.f ) - m_tip_anim ) * ( 1.f - std::exp( -8.f * dt ) );
			}
			else
			{
				m_slider_drag = false;
				m_knob_anim += ( 0.f - m_knob_anim ) * ( 1.f - std::exp( -22.f * dt ) );
				m_tip_anim += ( 0.f - m_tip_anim ) * ( 1.f - std::exp( -8.f * dt ) );
			}
		}

		float binds_block_h( ) const
		{
			float h = 0.f;
			for ( const config_bind_t& b : m_binds )
			{
				const float e = c_render::smoothstep( b.anim ) * ( 1.f - c_render::smoothstep( b.remove_t ) );
				h += k_bind_row * e;
			}
			return h;
		}

		c_vector_2d menu_size( ) const
		{
			const float actions = k_action_row * 2.f + k_sep_gap + 1.f + k_action_row;
			const float binds_h = binds_block_h( );
			const float sep = binds_h > 0.5f ? ( k_sep_gap + 1.f ) : 0.f;
			return c_vector_2d( k_menu_w, k_pad + binds_h + sep + actions + k_pad );
		}

		c_vector_2d editor_size( ) const
		{
			float h = k_pad;
			h += 30.f + k_sep_gap + 1.f; // key
			h += 34.f + k_sep_gap + 1.f; // mode
			if ( needs_value( ) )
				h += 34.f + k_sep_gap + 1.f;
			h += 30.f; // delete
			h += k_pad;
			return c_vector_2d( k_editor_w, h );
		}

		void apply_bind( config_bind_t& b )
		{
			switch ( m_slot.kind )
			{
			case bind_kind::boolean:
				b.b_restore = m_slot.read_bool( );
				m_slot.write_bool( b.b_val );
				break;
			case bind_kind::integer:
				b.i_restore = m_slot.read_int( );
				m_slot.write_int( b.i_val );
				break;
			case bind_kind::floating:
				b.f_restore = m_slot.read_float( );
				m_slot.write_float( b.f_val );
				break;
			}
			b.applied = true;
		}

		void restore_bind( config_bind_t& b )
		{
			switch ( m_slot.kind )
			{
			case bind_kind::boolean: m_slot.write_bool( b.b_restore ); break;
			case bind_kind::integer: m_slot.write_int( b.i_restore ); break;
			case bind_kind::floating: m_slot.write_float( b.f_restore ); break;
			}
			b.applied = false;
		}

		int visible_bind_count( ) const
		{
			int n = 0;
			for ( const config_bind_t& b : m_binds )
			{
				if ( !b.removing || b.anim > 0.02f )
					++n;
			}
			return n;
		}

		c_vector_2d menu_pos( ) const
		{
			const c_vector_2d sz = menu_size( );
			float x = std::floor( m_dots.x + dots_width( ) - sz.x );
			float y = std::floor( m_dots.y + dots_height( ) + 8.f );
			const ImVec2 display = ImGui::GetIO( ).DisplaySize;
			if ( x < 8.f )
				x = 8.f;
			if ( x + sz.x > display.x - 8.f )
				x = display.x - sz.x - 8.f;
			if ( y + sz.y > display.y - 8.f )
				y = std::floor( m_dots.y - sz.y - 8.f );
			return c_vector_2d( x, y );
		}

		bool needs_value( ) const
		{
			return m_slot.kind != bind_kind::boolean;
		}

		c_vector_2d editor_pos( ) const
		{
			const c_vector_2d mp = menu_pos( );
			const c_vector_2d es = editor_size( );
			return c_vector_2d( std::floor( mp.x - 8.f - es.x ), mp.y );
		}

		c_vector_2d hotkeys_size( ) const
		{
			int n = 0;
			if ( g_bind_hub )
			{
				for ( c_bind_popup* p : g_bind_hub->popups( ) )
				{
					if ( !p )
						continue;
					n += static_cast<int>( p->binds( ).size( ) );
				}
			}
			n = ( std::max )( 1, n );
			return c_vector_2d( k_hotkeys_w, k_pad + 28.f + static_cast<float>( n ) * 26.f + k_pad );
		}

		c_vector_2d hotkeys_pos( ) const
		{
			const c_vector_2d mp = menu_pos( );
			const c_vector_2d hs = hotkeys_size( );
			float x = std::floor( mp.x - 8.f - hs.x );
			if ( m_editor_anim > 0.2f )
				x = std::floor( editor_pos( ).x - 8.f - hs.x );
			return c_vector_2d( x, mp.y );
		}

		const char* mode_name( key_mode_t m ) const
		{
			if ( m == key_mode_t::toggle )
				return "Toggle";
			if ( m == key_mode_t::always )
				return "Always";
			return "Hold";
		}

		void format_bind_label( const config_bind_t& b, char* buf, int n ) const
		{
			if ( b.key.key <= 0 && b.key.mode != key_mode_t::always )
			{
				std::snprintf( buf, n, "New bind" );
				return;
			}
			const char* key = ( b.key.key <= 0 ) ? "-" : b.key.name( );
			std::snprintf( buf, n, "%s \"%s\"", mode_name( b.key.mode ), key );
		}

		void draw_sep( float x, float y, float w )
		{
			g_render->line( c_vector_2d( x + 10.f, y + 0.5f ), c_vector_2d( x + w - 10.f, y + 0.5f ), c_color( 48, 52, 64, 150 ) );
		}

		void draw_menu_body( c_vector_2d mp )
		{
			float y = mp.y + k_pad;

			if ( m_sel_a > 0.01f )
			{
				const float e = c_render::smoothstep( m_sel_a );
				g_render->rect_filled_f(
					mp.x + 5.f,
					m_sel_y,
					k_menu_w - 10.f,
					m_sel_h,
					c_color( 30, 34, 48, static_cast<int>( 210.f * e ) ),
					5.f );
				g_render->rect_f(
					mp.x + 5.f,
					m_sel_y,
					k_menu_w - 10.f,
					m_sel_h,
					g_style->accent.with_alpha( static_cast<int>( 90.f * e ) ),
					5.f,
					1.f );
			}

			int idx = 0;
			for ( config_bind_t& b : m_binds )
			{
				const float e = c_render::smoothstep( b.anim ) * ( 1.f - c_render::smoothstep( b.remove_t ) );
				if ( e <= 0.01f )
				{
					++idx;
					continue;
				}

				const float row_h = k_bind_row * e;
				const bool sel = idx == m_selected;
				const bool unset = ( b.key.key <= 0 && b.key.mode != key_mode_t::always );
				char label[64] {};
				format_bind_label( b, label, sizeof( label ) );

				const char* icon = unset ? "\xef\x81\x95" : "\xef\x84\x9c";
				const float icon_y = y + std::floor( ( row_h - 12.f ) * 0.5f );
				const float text_y = y + std::floor( ( row_h - g_style->text_control ) * 0.5f );
				const c_color col = ( sel ? g_style->text_bright : g_style->text_control_col ).with_alpha( static_cast<int>( 255.f * e ) );

				g_render->text( c_fonts::k_icons_key, c_vector_2d( mp.x + 12.f, icon_y ), col.lerp( g_style->accent, unset ? 0.35f : 0.f ), icon, 11.f );
				g_render->text( c_fonts::k_default_key, c_vector_2d( mp.x + 32.f, text_y ), col, label, g_style->text_control );

				y += row_h;
				++idx;
			}

			if ( binds_block_h( ) > 0.5f )
			{
				y += k_sep_gap * 0.5f;
				draw_sep( mp.x, y, k_menu_w );
				y += k_sep_gap * 0.5f + 1.f;
			}

			draw_action_row( mp.x, y, "\xef\x81\x95", "New bind", 0, false );
			y += k_action_row;
			draw_action_row( mp.x, y, "\xef\x80\xba", "Hotkeys", 1, m_hotkeys_open );
			y += k_action_row;

			y += k_sep_gap * 0.5f;
			draw_sep( mp.x, y, k_menu_w );
			y += k_sep_gap * 0.5f + 1.f;
			draw_action_row( mp.x, y, "\xef\x83\xad", "Reset", 2, false, true );
		}

		void draw_action_row( float x, float y, const char* icon, const char* text, int id, bool active, bool danger = false )
		{
			const float h = m_action_hover[id];
			const c_color base = danger ? c_color( 220, 90, 100 ) : g_style->text_control_col;
			const c_color col = base.lerp( danger ? c_color( 255, 120, 130 ) : g_style->text_bright, h ).lerp( g_style->accent, active ? 0.55f : 0.f );
			const float icon_y = y + std::floor( ( k_action_row - 12.f ) * 0.5f );
			const float text_y = y + std::floor( ( k_action_row - g_style->text_control ) * 0.5f );
			g_render->text( c_fonts::k_icons_key, c_vector_2d( x + 12.f, icon_y ), col, icon, 12.f );
			g_render->text( c_fonts::k_default_key, c_vector_2d( x + 34.f, text_y ), col, text, g_style->text_control );
		}

		void draw_editor_body( c_vector_2d ep, config_bind_t& b )
		{
			const float ctrl_x = ep.x + k_label_col;
			const float ctrl_w = k_editor_w - k_label_col - 12.f;
			float y = ep.y + k_pad;

			// Key
			{
				const float row_h = 30.f;
				g_render->text( c_fonts::k_default_key, c_vector_2d( ep.x + 12.f, y + std::floor( ( row_h - g_style->text_control ) * 0.5f ) ), g_style->text_dim, "Key", g_style->text_control );
				const char* key_txt = m_listening ? "..." : ( ( b.key.key <= 0 ) ? "-" : b.key.name( ) );
				const float box_h = 24.f;
				const float box_y = y + std::floor( ( row_h - box_h ) * 0.5f );
				g_render->rect_filled_f( ctrl_x, box_y, ctrl_w, box_h, g_style->frame.lerp( g_style->accent.with_alpha( 40 ), m_listening ? 1.f : 0.f ), 5.f );
				g_render->rect_f( ctrl_x, box_y, ctrl_w, box_h, g_style->frame_border.lerp( g_style->accent, m_listening ? 1.f : 0.f ), 5.f );
				const c_vector_2d kts = g_render->measure_text( c_fonts::k_default_key, key_txt, g_style->text_control );
				g_render->text(
					c_fonts::k_default_key,
					c_vector_2d( ctrl_x + std::floor( ( ctrl_w - kts.x ) * 0.5f ), box_y + std::floor( ( box_h - kts.y ) * 0.5f ) ),
					m_listening ? g_style->accent : g_style->text_bright,
					key_txt,
					g_style->text_control );
				y += row_h;
			}

			y += k_sep_gap * 0.5f;
			draw_sep( ep.x, y, k_editor_w );
			y += k_sep_gap * 0.5f + 1.f;

			// Mode
			{
				const float row_h = 34.f;
				g_render->text( c_fonts::k_default_key, c_vector_2d( ep.x + 12.f, y + std::floor( ( row_h - g_style->text_control ) * 0.5f ) ), g_style->text_dim, "Mode", g_style->text_control );
				const char* modes[3] = { "Hold", "Toggle", "Always" };
				const key_mode_t mode_vals[3] = { key_mode_t::hold, key_mode_t::toggle, key_mode_t::always };
				const float gap = 4.f;
				const float pill_w = std::floor( ( ctrl_w - gap * 2.f ) / 3.f );
				const float pill_h = 26.f;
				const float pill_y = y + std::floor( ( row_h - pill_h ) * 0.5f );
				float mx = ctrl_x;
				for ( int i = 0; i < 3; ++i )
				{
					const bool on = b.key.mode == mode_vals[i];
					const float hv = m_mode_hover[i];
					g_render->rect_filled_f( mx, pill_y, pill_w, pill_h, on ? g_style->accent.with_alpha( 70 ) : g_style->frame.lerp( c_color( 36, 40, 54 ), hv * 0.5f ), 5.f );
					g_render->rect_f( mx, pill_y, pill_w, pill_h, on ? g_style->accent.with_alpha( 160 ) : g_style->frame_border, 5.f );
					const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, modes[i], g_style->text_control );
					g_render->text(
						c_fonts::k_default_key,
						c_vector_2d( mx + std::floor( ( pill_w - ts.x ) * 0.5f ), pill_y + std::floor( ( pill_h - ts.y ) * 0.5f ) ),
						on ? g_style->text_bright : g_style->text_control_col.lerp( g_style->text_bright, hv ),
						modes[i],
						g_style->text_control );
					mx += pill_w + gap;
				}
				y += row_h;
			}

			if ( needs_value( ) )
			{
				y += k_sep_gap * 0.5f;
				draw_sep( ep.x, y, k_editor_w );
				y += k_sep_gap * 0.5f + 1.f;

				const float row_h = 34.f;
				g_render->text( c_fonts::k_default_key, c_vector_2d( ep.x + 12.f, y + std::floor( ( row_h - g_style->text_control ) * 0.5f ) ), g_style->text_dim, "Value", g_style->text_control );
				if ( m_slot.kind == bind_kind::floating )
					draw_edit_slider( ctrl_x, y, ctrl_w, row_h, b );
				else if ( m_slot.kind == bind_kind::integer )
					draw_mini_combo( ctrl_x, y + std::floor( ( row_h - 24.f ) * 0.5f ), ctrl_w, b.i_val );
				y += row_h;
			}

			y += k_sep_gap * 0.5f;
			draw_sep( ep.x, y, k_editor_w );
			y += k_sep_gap * 0.5f + 1.f;

			{
				const float row_h = 30.f;
				const float icon_y = y + std::floor( ( row_h - 12.f ) * 0.5f );
				const float text_y = y + std::floor( ( row_h - g_style->text_control ) * 0.5f );
				g_render->text( c_fonts::k_icons_key, c_vector_2d( ep.x + 12.f, icon_y ), c_color( 220, 90, 100 ), "\xef\x83\xad", 12.f );
				g_render->text( c_fonts::k_default_key, c_vector_2d( ep.x + 34.f, text_y ), c_color( 220, 90, 100 ), "Delete", g_style->text_control );
			}
		}

		void draw_edit_slider( float x, float y, float w, float row_h, config_bind_t& b )
		{
			const float th = g_style->slider_track_h;
			const float track_y = y + std::floor( ( row_h - th ) * 0.5f );
			const float round = th * 0.5f;
			const float fill_w = w * m_slider_anim;

			g_render->rect_filled_f( x, track_y, w, th, g_style->slider_track, round );
			if ( fill_w > 0.5f )
			{
				ImDrawList* dl = g_render->draw_list( );
				if ( dl )
				{
					dl->PushClipRect( ImVec2( x, track_y ), ImVec2( x + fill_w, track_y + th ), true );
					g_render->rect_filled_f( x, track_y, w, th, g_style->accent, round );
					dl->PopClipRect( );
				}
			}

			const float radius = ImLerp( 5.5f, 7.f, m_knob_anim );
			const float knob_x = x + fill_w;
			const float knob_y = track_y + th * 0.5f;
			g_render->circle_shadow( c_vector_2d( knob_x, knob_y ), radius, c_color( 0, 0, 0, 90 ), 10.f );
			g_render->circle_filled( c_vector_2d( knob_x, knob_y ), radius, c_color( 247, 248, 252 ), 48 );

			if ( m_tip_anim > 0.01f )
			{
				const float tip_x = knob_x;
				const float tip_y = knob_y;
				const float tip_anim = m_tip_anim;
				const float tip_val = b.f_val;
				g_render->defer( [tip_x, tip_y, tip_anim, tip_val]( )
				{
					char buf[32] {};
					std::snprintf( buf, sizeof( buf ), "%d", static_cast<int>( std::lround( tip_val ) ) );
					const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, buf, g_style->text_control );
					const float pad_x = 10.f;
					const float pad_y = 6.f;
					const float bw = std::floor( ts.x + pad_x * 2.f );
					const float bh = std::floor( ts.y + pad_y * 2.f );
					const float bx = std::floor( tip_x - bw * 0.5f );
					const float by = std::floor( tip_y - bh - 14.f );
					const float fr = g_style->frame_rounding + 1.f;
					const int vtx = g_render->vtx_count( );
					g_render->frosted_panel( bx, by, bw, bh, fr, true );
					g_render->text(
						c_fonts::k_default_key,
						c_vector_2d( bx + std::floor( ( bw - ts.x ) * 0.5f ), by + std::floor( ( bh - ts.y ) * 0.5f ) ),
						c_color( 236, 238, 245 ),
						buf,
						g_style->text_control );
					g_render->apply_open( vtx, c_vector_2d( tip_x, by + bh ), tip_anim, 0.88f, 12.f );
				} );
			}
		}

		void draw_mini_combo( float x, float y, float w, int& value )
		{
			const char* text = "-";
			if ( !m_slot.items.empty( ) )
			{
				value = ( std::max )( 0, ( std::min )( value, static_cast<int>( m_slot.items.size( ) ) - 1 ) );
				text = m_slot.items[value].c_str( );
			}
			g_render->rect_filled_f( x, y, w, 24.f, g_style->frame, 5.f );
			g_render->rect_f( x, y, w, 24.f, g_style->frame_border, 5.f );
			const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, text, g_style->text_control );
			g_render->text( c_fonts::k_default_key, c_vector_2d( x + std::floor( ( w - ts.x ) * 0.5f ), y + std::floor( ( 24.f - ts.y ) * 0.5f ) ), g_style->text_control_col, text, g_style->text_control );
		}

		void add_bind( )
		{
			config_bind_t b {};
			b.key.mode = key_mode_t::hold;
			b.b_val = true;
			b.i_val = m_seed_i;
			b.f_val = m_seed_f;
			b.anim = 0.f;
			m_binds.push_back( b );
			m_selected = static_cast<int>( m_binds.size( ) ) - 1;
			m_hotkeys_open = false;
		}

		void request_remove( int index )
		{
			if ( index < 0 || index >= static_cast<int>( m_binds.size( ) ) )
				return;
			m_binds[index].removing = true;
			if ( m_selected == index )
				m_selected = -1;
		}

		void reset_all( )
		{
			for ( config_bind_t& b : m_binds )
			{
				if ( b.applied )
					restore_bind( b );
				b.removing = true;
			}
			m_selected = -1;
			m_slot.reset_default( );
			seed_override_from_slot( );
		}

		void draw_panels( )
		{
			const c_vector_2d mp = menu_pos( );
			const c_vector_2d ms = menu_size( );
			const int vtx = g_render->vtx_count( );
			const float fr = 8.f;

			g_render->frosted_panel( mp.x, mp.y, ms.x, ms.y, fr, true );
			draw_menu_body( mp );

			g_render->apply_open( vtx, c_vector_2d( m_dots.x + dots_width( ) * 0.5f, m_dots.y + dots_height( ) * 0.5f ), m_open_anim, 0.86f, 18.f );

			if ( m_editor_anim > 0.01f && m_selected >= 0 && m_selected < static_cast<int>( m_binds.size( ) ) )
			{
				const c_vector_2d ep = editor_pos( );
				const c_vector_2d es = editor_size( );
				const int ev = g_render->vtx_count( );
				g_render->frosted_panel( ep.x, ep.y, es.x, es.y, fr, true );
				draw_editor_body( ep, m_binds[m_selected] );
				g_render->apply_open( ev, c_vector_2d( ep.x + es.x, ep.y + 20.f ), m_editor_anim, 0.88f, 16.f );
			}

			if ( m_hotkeys_anim > 0.01f )
			{
				const c_vector_2d hp = hotkeys_pos( );
				const c_vector_2d hs = hotkeys_size( );
				const int hv = g_render->vtx_count( );
				g_render->frosted_panel( hp.x, hp.y, hs.x, hs.y, fr, true );
				draw_hotkeys_body( hp );
				g_render->apply_open( hv, c_vector_2d( hp.x + hs.x, hp.y + 20.f ), m_hotkeys_anim, 0.88f, 16.f );
			}
		}

		void draw_hotkeys_body( c_vector_2d hp )
		{
			g_render->text( c_fonts::k_default_key, c_vector_2d( hp.x + 12.f, hp.y + 10.f ), g_style->text_bright, "Hotkeys", g_style->text_body );
			float y = hp.y + 34.f;
			bool any = false;
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
						any = true;
						char line[96] {};
						char bl[48] {};
						p->format_bind_label( b, bl, sizeof( bl ) );
						std::snprintf( line, sizeof( line ), "%s  ·  %s", p->slot( ).label.c_str( ), bl );
						g_render->text( c_fonts::k_default_key, c_vector_2d( hp.x + 12.f, y ), g_style->text_control_col, line, g_style->text_control );
						y += 26.f;
					}
				}
			}
			if ( !any )
				g_render->text( c_fonts::k_default_key, c_vector_2d( hp.x + 12.f, y ), g_style->text_dim, "No binds yet", g_style->text_control );
		}

		void handle_menu_input( )
		{
			const c_vector_2d mp = menu_pos( );
			float y = mp.y + k_pad;
			int idx = 0;
			for ( config_bind_t& b : m_binds )
			{
				const float e = c_render::smoothstep( b.anim ) * ( 1.f - c_render::smoothstep( b.remove_t ) );
				if ( e <= 0.01f )
				{
					++idx;
					continue;
				}
				const float row_h = k_bind_row * e;
				if ( g_input->mouse_in_region( c_vector_2d( mp.x, y ), c_vector_2d( k_menu_w, row_h ) ) && g_input->clicked( mouse_buttons::left ) )
				{
					m_selected = idx;
					m_hotkeys_open = false;
					return;
				}
				y += row_h;
				++idx;
			}

			if ( binds_block_h( ) > 0.5f )
				y += k_sep_gap + 1.f;

			const float dt = ImGui::GetIO( ).DeltaTime;
			for ( int i = 0; i < 3; ++i )
			{
				float ay = y;
				if ( i == 1 )
					ay = y + k_action_row;
				else if ( i == 2 )
					ay = y + k_action_row * 2.f + k_sep_gap + 1.f;

				const bool hovered = g_input->mouse_in_region( c_vector_2d( mp.x, ay ), c_vector_2d( k_menu_w, k_action_row ) );
				m_action_hover[i] += ( ( hovered ? 1.f : 0.f ) - m_action_hover[i] ) * ( 1.f - std::exp( -20.f * dt ) );
				if ( hovered && g_input->clicked( mouse_buttons::left ) )
				{
					if ( i == 0 )
						add_bind( );
					else if ( i == 1 )
						m_hotkeys_open = !m_hotkeys_open;
					else
						reset_all( );
					return;
				}
			}
		}

		void handle_editor_input( )
		{
			config_bind_t& b = m_binds[m_selected];
			const c_vector_2d ep = editor_pos( );
			const float ctrl_x = ep.x + k_label_col;
			const float ctrl_w = k_editor_w - k_label_col - 12.f;
			float y = ep.y + k_pad;
			const float dt = ImGui::GetIO( ).DeltaTime;

			const float key_row = 30.f;
			const float box_h = 24.f;
			const float box_y = y + std::floor( ( key_row - box_h ) * 0.5f );
			if ( g_input->mouse_in_region( c_vector_2d( ctrl_x, box_y ), c_vector_2d( ctrl_w, box_h ) ) && g_input->clicked( mouse_buttons::left ) )
			{
				m_listening = true;
				return;
			}

			if ( m_listening )
			{
				if ( g_input->key_pressed( VK_ESCAPE ) )
				{
					m_listening = false;
					return;
				}
				for ( int vk = 1; vk < 255; ++vk )
				{
					if ( vk == VK_INSERT || vk == VK_LBUTTON )
						continue;
					if ( g_input->key_pressed( vk ) )
					{
						b.key.key = vk;
						m_listening = false;
						return;
					}
				}
				return;
			}

			y += key_row + k_sep_gap + 1.f;

			const float mode_row = 34.f;
			const key_mode_t mode_vals[3] = { key_mode_t::hold, key_mode_t::toggle, key_mode_t::always };
			const float gap = 4.f;
			const float pill_w = std::floor( ( ctrl_w - gap * 2.f ) / 3.f );
			const float pill_h = 26.f;
			const float pill_y = y + std::floor( ( mode_row - pill_h ) * 0.5f );
			float mx = ctrl_x;
			for ( int i = 0; i < 3; ++i )
			{
				const bool hovered = g_input->mouse_in_region( c_vector_2d( mx, pill_y ), c_vector_2d( pill_w, pill_h ) );
				m_mode_hover[i] += ( ( hovered ? 1.f : 0.f ) - m_mode_hover[i] ) * ( 1.f - std::exp( -20.f * dt ) );
				if ( hovered && g_input->clicked( mouse_buttons::left ) )
				{
					b.key.mode = mode_vals[i];
					return;
				}
				mx += pill_w + gap;
			}
			y += mode_row;

			if ( needs_value( ) )
			{
				y += k_sep_gap + 1.f;
				const float value_row = 34.f;
				if ( m_slot.kind == bind_kind::floating )
				{
					const float th = g_style->slider_track_h;
					const float track_y = y + std::floor( ( value_row - th ) * 0.5f );
					const c_vector_2d hit( ctrl_x - 6.f, track_y - 10.f );
					const c_vector_2d hit_size( ctrl_w + 12.f, th + 20.f );
					if ( g_input->mouse_in_region( hit, hit_size ) && g_input->clicked( mouse_buttons::left ) )
						m_slider_drag = true;
					if ( m_slider_drag )
					{
						if ( g_input->click_down( mouse_buttons::left ) )
						{
							const float t = ImClamp( ( g_input->get_mouse_position( ).x - ctrl_x ) / ctrl_w, 0.f, 1.f );
							b.f_val = m_slot.min_f + ( m_slot.max_f - m_slot.min_f ) * t;
						}
						else
						{
							m_slider_drag = false;
						}
					}
				}
				else if ( m_slot.kind == bind_kind::integer && !m_slot.items.empty( ) )
				{
					const float box_y2 = y + std::floor( ( value_row - 24.f ) * 0.5f );
					if ( g_input->mouse_in_region( c_vector_2d( ctrl_x, box_y2 ), c_vector_2d( ctrl_w, 24.f ) ) && g_input->clicked( mouse_buttons::left ) )
					{
						b.i_val = ( b.i_val + 1 ) % static_cast<int>( m_slot.items.size( ) );
						return;
					}
				}
				y += value_row;
			}

			y += k_sep_gap + 1.f;
			if ( g_input->mouse_in_region( c_vector_2d( ep.x, y ), c_vector_2d( k_editor_w, 30.f ) ) && g_input->clicked( mouse_buttons::left ) )
				request_remove( m_selected );
		}

		void handle_hotkeys_input( )
		{
		}
	};

	inline void c_bind_hub::update( )
	{
		for ( c_bind_popup* p : m_popups )
		{
			if ( p )
				p->apply_runtime( );
		}
	}
}
