#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include <imgui.h>

#include <core/framework/gui/frontend/widgets/classes/base_element.hxx>
#include <core/framework/gui/frontend/widgets/settings.hxx>
#include <core/framework/gui/backend/render/render.hxx>
#include <core/framework/gui/backend/inputs/inputs.hxx>

namespace core::gui
{
	struct text_char_exit_t
	{
		char ch {};
		float x {};
		float anim { 1.f }; // 1 = seated, 0 = fully out
		float drift { 0.f }; // sideways leave so deletes don't stack/stick
	};

	struct text_anim_state_t
	{
		std::vector<float> enter {};
		std::vector<text_char_exit_t> exit {};
		float back_hold { 0.f };
		float del_hold { 0.f };
		bool back_repeat { false };
		bool del_repeat { false };
		bool back_burst { false };
		bool del_burst { false };

		void clear( )
		{
			enter.clear( );
			exit.clear( );
			back_hold = 0.f;
			del_hold = 0.f;
			back_repeat = false;
			del_repeat = false;
			back_burst = false;
			del_burst = false;
		}
	};

	class c_text_input : public c_base_element
	{
	public:
		static constexpr float k_travel = 10.f;
		static constexpr float k_enter_speed = 34.f;
		static constexpr float k_exit_speed = 38.f;
		static constexpr std::size_t k_max_exits = 28;

		c_text_input( std::string label, std::string* value, bool hide_label = false )
			: m_value( value )
		{
			m_label = std::move( label );
			m_type = element_type::text_input;
			m_visible = true;
			m_focus_priority = focus_priority::interactive;
			m_size = c_vector_2d( 200.f, g_style ? g_style->row_height : 37.f );
			if ( hide_label )
				this->hide_label( );
			if ( m_value )
				m_anim.enter.assign( m_value->size( ), 1.f );
		}

		void set_placeholder( std::string text ) { m_placeholder = std::move( text ); }
		void set_leading_icon( const char* icon ) { m_icon = icon; }
		void set_max_length( int n ) { m_max_len = n; }
		void set_box( c_vector_2d pos, float w, float h )
		{
			m_box = pos;
			m_box_w = w;
			m_box_h = h;
			m_manual_box = true;
		}

		bool focused( ) const { return m_focused; }
		void focus( )
		{
			m_focused = true;
			m_caret = m_value ? static_cast<int>( m_value->size( ) ) : 0;
			m_caret_t = 0.f;
		}
		void blur( ) { m_focused = false; }

		void play_intro( ) override
		{
			m_reveal = 0.f;
			m_focus_anim = 0.f;
			m_hover = 0.f;
			m_ph_anim = 1.f;
		}

		void draw( ) override
		{
			if ( !is_visible( ) || !g_render || !g_style )
				return;

			const float row_h = g_style->row_height;
			m_size.y = row_h;
			const float width = m_parent_width > 0.f ? m_parent_width : m_size.x;
			m_size.x = width;

			if ( !m_manual_box )
			{
				const float cw = g_style->control_w_for( width );
				const float bh = g_style->combo_h;
				m_box = c_vector_2d( g_style->control_x( m_pos.x, width ), m_pos.y + std::floor( ( row_h - bh ) * 0.5f ) );
				m_box_w = cw;
				m_box_h = bh;
			}

			const float dt = ImGui::GetIO( ).DeltaTime;
			const bool hovered = g_input && g_input->mouse_in_region( m_box, c_vector_2d( m_box_w, m_box_h ) );
			m_hover += ( ( hovered || m_focused ? 1.f : 0.f ) - m_hover ) * ( 1.f - std::exp( -20.f * dt ) );
			m_focus_anim += ( ( m_focused ? 1.f : 0.f ) - m_focus_anim ) * ( 1.f - std::exp( -16.f * dt ) );
			const bool show_ph = ( !m_value || m_value->empty( ) ) && !m_focused && m_anim.exit.empty( );
			m_ph_anim += ( ( show_ph ? 1.f : 0.f ) - m_ph_anim ) * ( 1.f - std::exp( -18.f * dt ) );
			advance_reveal( dt );
			tick_text_anims( m_anim, dt );

			if ( !m_hide_label && !m_manual_box )
			{
				const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, m_label.c_str( ), g_style->text_body );
				g_render->text(
					c_fonts::k_default_key,
					c_vector_2d( m_pos.x + g_style->label_pad, m_pos.y + std::floor( ( row_h - ts.y ) * 0.5f ) ),
					g_style->text,
					m_label.c_str( ),
					g_style->text_body );
			}

			paint_field( );
			m_manual_box = false;
		}

		void paint_field( )
		{
			const float reveal = c_render::ease_in_out_quint( m_reveal );
			const float fe = c_render::ease_out_cubic( m_focus_anim );
			const float he = c_render::ease_out_cubic( m_hover );
			const float fr = g_style->frame_rounding + 1.f;

			const c_color fill = g_style->frame
				.lerp( c_color( 30, 34, 48 ), he * 0.7f )
				.lerp( c_color( 24, 30, 48 ), fe * 0.95f )
				.with_alpha( static_cast<int>( 255.f * ( 0.45f + 0.55f * reveal ) ) );
			const c_color border = g_style->frame_border
				.lerp( c_color( 58, 64, 84 ), he * 0.5f )
				.lerp( g_style->accent, fe * 0.95f )
				.with_alpha( static_cast<int>( 255.f * reveal ) );

			g_render->rect_filled_f( m_box.x, m_box.y, m_box_w, m_box_h, fill, fr );
			g_render->rect_f( m_box.x, m_box.y, m_box_w, m_box_h, border, fr, 1.f );

			float text_x = m_box.x + 10.f;
			const float fs = g_style->text_control;

			if ( m_icon && m_icon[0] )
			{
				const c_color ic = c_color( 120, 126, 140 )
					.lerp( g_style->accent, fe )
					.with_alpha( static_cast<int>( 255.f * reveal ) );
				g_render->text( c_fonts::k_icons_key, c_vector_2d( m_box.x + 9.f, m_box.y + 7.f ), ic, m_icon, 12.f );
				text_x = m_box.x + 28.f;
			}

			const float text_y_base = m_box.y + std::floor( ( m_box_h - fs ) * 0.5f );
			ImDrawList* dl = g_render->draw_list( );
			if ( !dl )
				return;

			dl->PushClipRect( ImVec2( text_x - 1.f, m_box.y + 1.f ), ImVec2( m_box.x + m_box_w - 8.f, m_box.y + m_box_h - 1.f ), true );

			const float phe = c_render::ease_out_cubic( m_ph_anim );
			if ( phe > 0.01f )
			{
				g_render->text(
					c_fonts::k_default_key,
					c_vector_2d( text_x, text_y_base + ( 1.f - phe ) * k_travel ),
					c_color( 96, 102, 116, static_cast<int>( 255.f * reveal * phe ) ),
					m_placeholder.c_str( ),
					fs );
			}

			paint_text_chars( m_value ? *m_value : m_empty, m_anim, text_x, text_y_base, fs, g_style->text_control_col.with_alpha( static_cast<int>( 255.f * reveal ) ) );

			if ( m_focused && m_value )
			{
				m_caret_t += ImGui::GetIO( ).DeltaTime;
				if ( std::fmod( m_caret_t, 1.05f ) < 0.55f )
				{
					const int caret = ImClamp( m_caret, 0, static_cast<int>( m_value->size( ) ) );
					const std::string prefix = m_value->substr( 0, static_cast<std::size_t>( caret ) );
					const c_vector_2d ps = g_render->measure_text( c_fonts::k_default_key, prefix.c_str( ), fs );
					const float cx = text_x + ps.x;
					g_render->line(
						c_vector_2d( cx, m_box.y + 6.f ),
						c_vector_2d( cx, m_box.y + m_box_h - 6.f ),
						g_style->accent.with_alpha( static_cast<int>( 220.f * fe ) ),
						1.2f );
				}
			}

			dl->PopClipRect( );
		}

		void input( ) override
		{
			if ( !is_visible( ) || !g_input || !g_ctx || !g_style )
				return;

			if ( !m_manual_box )
			{
				const float width = m_parent_width > 0.f ? m_parent_width : m_size.x;
				const float cw = g_style->control_w_for( width );
				const float bh = g_style->combo_h;
				m_box = c_vector_2d( g_style->control_x( m_pos.x, width ), m_pos.y + std::floor( ( g_style->row_height - bh ) * 0.5f ) );
				m_box_w = cw;
				m_box_h = bh;
			}

			const bool can = g_ctx->can_interact( this, m_focus_priority ) || m_focused;
			if ( !can )
			{
				m_focused = false;
				m_manual_box = false;
				return;
			}

			if ( g_input->clicked( mouse_buttons::left ) )
			{
				const bool hit = g_input->mouse_in_region( m_box, c_vector_2d( m_box_w, m_box_h ) );
				if ( hit )
					focus( );
				else if ( m_focused )
					blur( );
			}

			if ( m_focused && m_value )
			{
				consume_typing(
					*m_value,
					&m_caret,
					m_max_len,
					&m_anim,
					g_style->text_control,
					ImGui::GetIO( ).DeltaTime );
			}

			m_manual_box = false;
		}

		static float char_motion_ease( float t )
		{
			t = ImClamp( t, 0.f, 1.f );
			// smoothstep milky settle
			return t * t * ( 3.f - 2.f * t );
		}

		static float char_spawn_ease( float t ) { return char_motion_ease( t ); }

		static void ensure_enter_size( text_anim_state_t& anim, std::size_t len )
		{
			if ( anim.enter.size( ) < len )
				anim.enter.insert( anim.enter.end( ), len - anim.enter.size( ), 1.f );
			else if ( anim.enter.size( ) > len )
				anim.enter.resize( len );
		}

		static void push_exit( text_anim_state_t& anim, char ch, float x )
		{
			if ( anim.exit.size( ) >= k_max_exits )
				anim.exit.erase( anim.exit.begin( ) );
			const float drift = ( ( static_cast<int>( ch ) & 1 ) ? 1.f : -1.f ) * 5.f;
			anim.exit.push_back( { ch, x, 1.f, drift } );
		}

		static void tick_text_anims( text_anim_state_t& anim, float dt )
		{
			for ( float& a : anim.enter )
			{
				a += ( 1.f - a ) * ( 1.f - std::exp( -k_enter_speed * dt ) );
				if ( a > 0.997f )
					a = 1.f;
			}

			for ( text_char_exit_t& g : anim.exit )
			{
				g.anim += ( 0.f - g.anim ) * ( 1.f - std::exp( -k_exit_speed * dt ) );
				if ( g.anim < 0.004f )
					g.anim = 0.f;
			}

			anim.exit.erase(
				std::remove_if( anim.exit.begin( ), anim.exit.end( ),
					[]( const text_char_exit_t& g ) { return g.anim <= 0.004f; } ),
				anim.exit.end( ) );
		}

		static void paint_text_chars(
			const std::string& value,
			const text_anim_state_t& anim,
			float text_x,
			float text_y,
			float fs,
			c_color col )
		{
			if ( !g_render )
				return;

			float cx = text_x;
			for ( std::size_t i = 0; i < value.size( ); ++i )
			{
				const float a = i < anim.enter.size( ) ? anim.enter[i] : 1.f;
				const float ae = char_motion_ease( a );
				char buf[2] { value[i], 0 };
				const c_vector_2d cs = g_render->measure_text( c_fonts::k_default_key, buf, fs );
				g_render->text(
					c_fonts::k_default_key,
					c_vector_2d( cx, text_y + ( 1.f - ae ) * k_travel ),
					col,
					buf,
					fs );
				cx += cs.x;
			}

			for ( const text_char_exit_t& g : anim.exit )
			{
				const float ae = char_motion_ease( g.anim );
				const float leave = 1.f - ae;
				char buf[2] { g.ch, 0 };
				g_render->text(
					c_fonts::k_default_key,
					c_vector_2d( text_x + g.x + g.drift * leave, text_y + leave * k_travel ),
					col,
					buf,
					fs );
			}
		}

		// Swap a displayed string with enter/exit char motion (config chip, etc).
		static void morph_string( std::string& current, text_anim_state_t& anim, const std::string& next, float font_size )
		{
			if ( current == next )
			{
				ensure_enter_size( anim, current.size( ) );
				return;
			}

			if ( g_render && !current.empty( ) )
			{
				float x = 0.f;
				for ( char ch : current )
				{
					push_exit( anim, ch, x );
					char buf[2] { ch, 0 };
					x += g_render->measure_text( c_fonts::k_default_key, buf, font_size ).x;
				}
			}

			current = next;
			anim.enter.assign( next.size( ), 0.f );
		}

		static void consume_typing(
			std::string& value,
			int* caret = nullptr,
			int max_len = 64,
			text_anim_state_t* anim = nullptr,
			float font_size = 13.f,
			float dt = 0.f )
		{
			ImGuiIO& io = ImGui::GetIO( );
			int cur = caret ? *caret : static_cast<int>( value.size( ) );
			cur = ImClamp( cur, 0, static_cast<int>( value.size( ) ) );

			if ( anim )
				ensure_enter_size( *anim, value.size( ) );

			auto measure_x = [&]( std::size_t count ) -> float
			{
				if ( count == 0 || !g_render )
					return 0.f;
				const std::string prefix = value.substr( 0, count );
				return g_render->measure_text( c_fonts::k_default_key, prefix.c_str( ), font_size ).x;
			};

			auto do_backspace = [&]( ) -> bool
			{
				if ( cur <= 0 || value.empty( ) )
					return false;
				const int idx = cur - 1;
				if ( anim )
				{
					push_exit( *anim, value[static_cast<std::size_t>( idx )], measure_x( static_cast<std::size_t>( idx ) ) );
					if ( idx < static_cast<int>( anim->enter.size( ) ) )
						anim->enter.erase( anim->enter.begin( ) + idx );
				}
				value.erase( static_cast<std::size_t>( idx ), 1 );
				--cur;
				if ( anim )
					ensure_enter_size( *anim, value.size( ) );
				return true;
			};

			auto do_delete = [&]( ) -> bool
			{
				if ( cur >= static_cast<int>( value.size( ) ) )
					return false;
				if ( anim )
				{
					push_exit( *anim, value[static_cast<std::size_t>( cur )], measure_x( static_cast<std::size_t>( cur ) ) );
					if ( cur < static_cast<int>( anim->enter.size( ) ) )
						anim->enter.erase( anim->enter.begin( ) + cur );
				}
				value.erase( static_cast<std::size_t>( cur ), 1 );
				if ( anim )
					ensure_enter_size( *anim, value.size( ) );
				return true;
			};

			auto tick_hold = [&]( bool pressed, bool down, bool& repeat, bool& burst, float& hold, auto&& erase_fn )
			{
				if ( pressed )
				{
					erase_fn( );
					hold = 0.f;
					repeat = true;
					burst = false;
					return;
				}
				if ( !down )
				{
					repeat = false;
					burst = false;
					hold = 0.f;
					return;
				}
				if ( !repeat )
					return;

				hold += dt;
				const float rate = burst ? 0.028f : 0.26f;
				while ( hold >= rate )
				{
					if ( !erase_fn( ) )
					{
						repeat = false;
						burst = false;
						hold = 0.f;
						return;
					}
					hold -= rate;
					burst = true;
				}
			};

			if ( g_input && g_input->key_down( VK_CONTROL ) )
			{
				if ( g_input->key_pressed( 'V' ) )
				{
					const std::string clip = g_input->get_clipboard( );
					for ( char ch : clip )
					{
						if ( static_cast<int>( value.size( ) ) >= max_len )
							break;
						if ( ch >= 32 && ch != 127 )
						{
							value.insert( value.begin( ) + cur, ch );
							if ( anim )
							{
								ensure_enter_size( *anim, value.size( ) - 1 );
								anim->enter.insert( anim->enter.begin( ) + cur, 0.f );
							}
							++cur;
						}
					}
				}
				if ( g_input->key_pressed( 'A' ) )
					cur = static_cast<int>( value.size( ) );
				if ( caret )
					*caret = cur;
				return;
			}

			if ( g_input )
			{
				if ( anim )
				{
					tick_hold( g_input->key_pressed( VK_BACK ), g_input->key_down( VK_BACK ), anim->back_repeat, anim->back_burst, anim->back_hold, do_backspace );
					tick_hold( g_input->key_pressed( VK_DELETE ), g_input->key_down( VK_DELETE ), anim->del_repeat, anim->del_burst, anim->del_hold, do_delete );
				}
				else
				{
					if ( g_input->key_pressed( VK_BACK ) )
						do_backspace( );
					if ( g_input->key_pressed( VK_DELETE ) )
						do_delete( );
				}

				if ( g_input->key_pressed( VK_LEFT ) )
					cur = ( std::max )( 0, cur - 1 );
				if ( g_input->key_pressed( VK_RIGHT ) )
					cur = ( std::min )( static_cast<int>( value.size( ) ), cur + 1 );
				if ( g_input->key_pressed( VK_HOME ) )
					cur = 0;
				if ( g_input->key_pressed( VK_END ) )
					cur = static_cast<int>( value.size( ) );
			}

			for ( int n = 0; n < io.InputQueueCharacters.Size; ++n )
			{
				const unsigned int c = io.InputQueueCharacters[n];
				if ( c < 32 || c == 127 || c > 255 )
					continue;
				if ( static_cast<int>( value.size( ) ) >= max_len )
					break;
				value.insert( value.begin( ) + cur, static_cast<char>( c ) );
				if ( anim )
				{
					ensure_enter_size( *anim, value.size( ) - 1 );
					anim->enter.insert( anim->enter.begin( ) + cur, 0.f );
				}
				++cur;
			}

			if ( anim )
				ensure_enter_size( *anim, value.size( ) );

			if ( caret )
				*caret = cur;
		}

		static void sync_char_anims( std::vector<float>& anims, std::size_t len, std::size_t before_len )
		{
			if ( len > before_len )
			{
				anims.resize( before_len );
				anims.insert( anims.end( ), len - before_len, 0.f );
			}
			else if ( len < anims.size( ) )
			{
				anims.resize( len );
			}
			else if ( anims.size( ) < len )
			{
				anims.resize( len, 1.f );
			}
		}

		static void tick_char_anims( std::vector<float>& anims, float dt, float speed = k_enter_speed )
		{
			for ( float& a : anims )
			{
				a += ( 1.f - a ) * ( 1.f - std::exp( -speed * dt ) );
				if ( a > 0.997f )
					a = 1.f;
			}
		}

	private:
		std::string* m_value { nullptr };
		std::string m_empty {};
		std::string m_placeholder { "Type..." };
		const char* m_icon { nullptr };
		int m_max_len { 64 };
		bool m_focused { false };
		bool m_manual_box { false };
		float m_hover { 0.f };
		float m_focus_anim { 0.f };
		float m_ph_anim { 1.f };
		float m_caret_t { 0.f };
		int m_caret { 0 };
		c_vector_2d m_box {};
		float m_box_w { 0.f };
		float m_box_h { 0.f };
		text_anim_state_t m_anim {};
	};
}
