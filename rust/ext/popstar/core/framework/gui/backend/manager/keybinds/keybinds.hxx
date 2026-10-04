#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>

#include <core/framework/gui/backend/render/render.hxx>
#include <core/framework/gui/frontend/widgets/classes/base_element.hxx>

namespace core::gui
{
	inline const char* const key_names[] =
	{
		"none",
		"m1",
		"m2",
		"break",
		"m3",
		"m4",
		"m5",
		"unk",
		"back",
		"tab",
		"unk",
		"unk",
		"clear",
		"enter",
		"unk",
		"unk",
		"shift",
		"control",
		"menu",
		"pause",
		"capital",
		"kana",
		"unk",
		"junja",
		"final",
		"kanji",
		"unk",
		"escape",
		"convert",
		"nonconvert",
		"accept",
		"modechange",
		"space",
		"prior",
		"next",
		"end",
		"home",
		"left",
		"up",
		"right",
		"down",
		"select",
		"print",
		"exec",
		"snap",
		"insert",
		"delete",
		"help",
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
		"unk", "unk", "unk", "unk", "unk", "unk", "unk",
		"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
		"n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
		"lwin",
		"rwin",
		"apps",
		"unk",
		"sleep",
		"num0", "num1", "num2", "num3", "num4", "num5", "num6", "num7", "num8", "num9",
		"multiply",
		"add",
		"separator",
		"subtract",
		"dec",
		"divide",
		"f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12",
		"f13", "f14", "f15", "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23", "f24",
		"unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk",
		"numlock",
		"scroll",
		"oem_nec_equal",
		"oem_fj_masshou",
		"oem_fj_touroku",
		"oem_fj_loya",
		"oem_fj_roya",
		"unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk", "unk",
		"lshift",
		"rshift",
		"lcontrol",
		"rcontrol",
		"lmenu",
		"rmenu"
	};

	enum key_mode_t : int
	{
		always = 0,
		hold = 1,
		toggle = 2
	};

	struct key_var_t
	{
		int key { 0 };
		key_mode_t mode { key_mode_t::hold };
		bool m_toggled { false };
		bool m_prev_down { false };

		bool active( bool bound_to_box = false ) const
		{
			if ( ( key <= 0 || key > 255 ) && mode != key_mode_t::always )
				return bound_to_box;

			if ( mode == key_mode_t::always )
				return true;
			if ( mode == key_mode_t::toggle )
				return m_toggled;
			if ( mode == key_mode_t::hold )
			{
				if ( g_input )
					return g_input->key_down( key );
				return ( GetAsyncKeyState( key ) & 0x8000 ) != 0;
			}
			return false;
		}

		void update( )
		{
			if ( mode != key_mode_t::toggle || key <= 0 || key > 255 )
			{
				m_prev_down = false;
				return;
			}

			const bool down = g_input ? g_input->key_down( key ) : ( ( GetAsyncKeyState( key ) & 0x8000 ) != 0 );
			if ( down && !m_prev_down )
				m_toggled = !m_toggled;
			m_prev_down = down;
		}

		const char* name( ) const
		{
			if ( key <= 0 || key >= static_cast<int>( sizeof( key_names ) / sizeof( key_names[0] ) ) )
				return key_names[0];
			return key_names[key];
		}
	};

	class c_keybind_system
	{
	public:
		void register_bind( key_var_t* bind )
		{
			if ( !bind )
				return;
			for ( key_var_t* existing : m_binds )
			{
				if ( existing == bind )
					return;
			}
			m_binds.push_back( bind );
		}

		void unregister_bind( key_var_t* bind )
		{
			m_binds.erase( std::remove( m_binds.begin( ), m_binds.end( ), bind ), m_binds.end( ) );
		}

		void update_all( )
		{
			for ( key_var_t* bind : m_binds )
			{
				if ( bind )
					bind->update( );
			}
		}

		void clear( )
		{
			m_binds.clear( );
		}

		const std::vector<key_var_t*>& binds( ) const
		{
			return m_binds;
		}

	private:
		std::vector<key_var_t*> m_binds {};
	};

	inline std::shared_ptr<c_keybind_system> g_keybinds = std::make_shared<c_keybind_system>( );

	class c_keybind : public c_base_element
	{
	public:
		c_keybind( std::string label, key_var_t* val, bool hide_label = false )
			: m_val( val )
		{
			m_label = std::move( label );
			m_type = element_type::keybind;
			m_visible = true;
			m_focus_priority = focus_priority::modal;
			m_layer = render_layer::modal;
			m_size = c_vector_2d( 200.f, g_style ? g_style->row_height : 37.f );
			if ( hide_label )
				this->hide_label( );
			if ( m_val && g_keybinds )
				g_keybinds->register_bind( m_val );
		}

		void play_intro( ) override
		{
			m_hover = 0.f;
			m_mode_anim = 0.f;
			m_intro_anim = 0.f;
		}

		void close_overlay( ) override
		{
			if ( m_listening )
				stop_listen( );
			m_listen_anim = 0.f;
			m_ignore_click = false;
		}

		void draw( ) override
		{
			if ( !is_visible( ) || !g_render || !g_style || !m_val )
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

			layout_box( width );

			const float dt = ImGui::GetIO( ).DeltaTime;
			const bool hovered = g_input && g_input->mouse_in_region( m_box, c_vector_2d( m_box_w, m_box_h ) );
			m_hover += ( ( hovered || m_listening ? 1.f : 0.f ) - m_hover ) * ( 1.f - std::exp( -18.f * dt ) );
			m_listen_anim += ( ( m_listening ? 1.f : 0.f ) - m_listen_anim ) * ( 1.f - std::exp( -16.f * dt ) );
			m_pulse += dt * 3.2f;
			if ( tick_intro( dt ) )
			{
				m_mode_anim += ( static_cast<float>( m_val->mode ) / 2.f - m_mode_anim ) * ( 1.f - std::exp( -14.f * dt ) );
				m_intro_anim += ( 1.f - m_intro_anim ) * ( 1.f - std::exp( -3.6f * dt ) );
				if ( std::fabs( 1.f - m_intro_anim ) < 0.001f )
					m_intro_anim = 1.f;
			}

			const float reveal = c_render::ease_in_out_quint( m_intro_anim );
			const float fr = g_style->frame_rounding + 1.f;
			const float listen_e = c_render::smoothstep( m_listen_anim );
			const float pulse = 0.5f + 0.5f * std::sin( m_pulse );
			const c_color fill = g_style->frame.lerp( c_color( 34, 38, 52 ), m_hover * 0.6f ).lerp( g_style->accent.with_alpha( 55 ), listen_e * ( 0.55f + pulse * 0.45f ) );
			const c_color border = g_style->frame_border.lerp( g_style->accent, m_hover * 0.35f + listen_e * ( 0.55f + pulse * 0.45f ) );

			const float scale = ImLerp( 0.92f, 1.f, reveal ) * ImLerp( 1.f, 1.04f, listen_e * pulse );
			const float bw = m_box_w * scale;
			const float bh = m_box_h * scale;
			const float bx = m_box.x + ( m_box_w - bw ) * 0.5f;
			const float by = m_box.y + ( m_box_h - bh ) * 0.5f;

			g_render->rect_filled_f( bx, by, bw, bh, fill.with_alpha( static_cast<int>( 255.f * ( 0.45f + 0.55f * reveal ) ) ), fr );
			g_render->rect_f( bx, by, bw, bh, border.with_alpha( static_cast<int>( 255.f * reveal ) ), fr, 1.f + listen_e );

			const char* mode_name = "hold";
			if ( m_val->mode == key_mode_t::toggle )
				mode_name = "toggle";
			else if ( m_val->mode == key_mode_t::always )
				mode_name = "always";

			char label[64] {};
			if ( m_listening )
				std::snprintf( label, sizeof( label ), "..." );
			else if ( m_val->mode == key_mode_t::always )
				std::snprintf( label, sizeof( label ), "always" );
			else
				std::snprintf( label, sizeof( label ), "%s", m_val->name( ) );

			const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, label, g_style->text_control );
			const c_color text_col = ( m_listening ? g_style->accent : g_style->text_control_col ).lerp( g_style->text_bright, m_hover * 0.4f );
			g_render->text(
				c_fonts::k_default_key,
				c_vector_2d( bx + std::floor( ( bw - ts.x ) * 0.5f ), by + std::floor( ( bh - ts.y ) * 0.5f ) ),
				text_col.with_alpha( static_cast<int>( 255.f * reveal ) ),
				label,
				g_style->text_control );

			if ( !m_listening && m_val->mode != key_mode_t::always )
			{
				const c_vector_2d ms = g_render->measure_text( c_fonts::k_caption_key, mode_name, 10.f );
				g_render->text(
					c_fonts::k_caption_key,
					c_vector_2d( bx + bw - ms.x - 6.f, by + bh - ms.y - 3.f ),
					c_color( 110, 116, 130, static_cast<int>( 180.f * reveal * ( 0.35f + m_hover * 0.65f ) ) ),
					mode_name,
					10.f );
			}
		}

		void input( ) override
		{
			if ( !is_visible( ) || !m_val || !g_input || !g_ctx || !g_style )
				return;

			const float width = m_parent_width > 0.f ? m_parent_width : m_size.x;
			layout_box( width );

			const bool can = g_ctx->can_interact( this, m_focus_priority ) || g_ctx->m_modal_owner == this || m_listening;
			if ( !can )
				return;

			const bool in_box = g_input->mouse_in_region( m_box, c_vector_2d( m_box_w, m_box_h ) );

			if ( m_listening )
			{
				if ( m_ignore_click )
				{
					if ( !g_input->click_down( mouse_buttons::left ) )
						m_ignore_click = false;
					return;
				}

				if ( g_input->key_pressed( VK_ESCAPE ) )
				{
					m_val->key = 0;
					stop_listen( );
					return;
				}

				const int pressed = g_input->get_pressed_key( );
				if ( pressed > 0 && pressed != VK_ESCAPE )
				{
					m_val->key = pressed;
					stop_listen( );
					return;
				}

				if ( g_input->clicked( mouse_buttons::left ) && !in_box )
					stop_listen( );
				return;
			}

			if ( in_box && g_input->clicked( mouse_buttons::left ) )
			{
				m_listening = true;
				m_ignore_click = true;
				m_listen_anim = 0.f;
				g_ctx->set_modal( this );
				return;
			}

			if ( in_box && g_input->clicked( mouse_buttons::right ) )
			{
				m_val->mode = static_cast<key_mode_t>( ( static_cast<int>( m_val->mode ) + 1 ) % 3 );
				return;
			}
		}

	private:
		key_var_t* m_val { nullptr };
		bool m_listening { false };
		bool m_ignore_click { false };
		float m_hover { 0.f };
		float m_listen_anim { 0.f };
		float m_pulse { 0.f };
		float m_mode_anim { 0.f };
		float m_intro_anim { 1.f };
		c_vector_2d m_box {};
		float m_box_w { 0.f };
		float m_box_h { 0.f };

		void layout_box( float width )
		{
			const float row_h = g_style->row_height;
			m_box_w = g_style->control_w;
			m_box_h = g_style->combo_h;
			m_box = c_vector_2d( g_style->control_x( m_pos.x, width ), m_pos.y + std::floor( ( row_h - m_box_h ) * 0.5f ) );
		}

		void stop_listen( )
		{
			m_listening = false;
			if ( g_ctx )
				g_ctx->clear_modal( this );
		}
	};
}
