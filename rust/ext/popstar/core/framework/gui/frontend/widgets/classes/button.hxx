#pragma once

#include <cmath>
#include <functional>
#include <string>

#include <core/framework/gui/frontend/widgets/classes/base_element.hxx>
#include <core/framework/gui/frontend/widgets/settings.hxx>
#include <core/framework/gui/backend/render/render.hxx>

namespace core::gui
{
	class c_button : public c_base_element
	{
	public:
		c_button( std::string label, std::function<void( )> callback )
			: m_callback( std::move( callback ) )
		{
			m_label = std::move( label );
			m_type = element_type::button;
			m_visible = true;
			m_focus_priority = focus_priority::interactive;
			m_size = c_vector_2d( 200.f, g_style ? g_style->row_height : 37.f );
		}

		void play_intro( ) override
		{
			m_reveal = 0.f;
			m_hover = 0.f;
			m_press = 0.f;
			m_intro_anim = 0.f;
		}

		void draw( ) override
		{
			if ( !is_visible( ) || !g_render || !g_style )
				return;

			const float row_h = g_style->row_height;
			m_size.y = row_h;
			const float width = m_parent_width > 0.f ? m_parent_width : m_size.x;
			m_size.x = width;

			const float bw = g_style->control_w_for( width );
			const float bh = g_style->combo_h + 2.f;
			m_box = c_vector_2d( g_style->control_x( m_pos.x, width ), m_pos.y + std::floor( ( row_h - bh ) * 0.5f ) );
			m_box_w = bw;
			m_box_h = bh;

			const float dt = ImGui::GetIO( ).DeltaTime;
			const bool hovered = g_input && g_input->mouse_in_region( m_box, c_vector_2d( bw, bh ) );
			m_hover += ( ( hovered ? 1.f : 0.f ) - m_hover ) * ( 1.f - std::exp( -18.f * dt ) );
			m_press += ( ( m_pressed ? 1.f : 0.f ) - m_press ) * ( 1.f - std::exp( -28.f * dt ) );
			m_flash += ( 0.f - m_flash ) * ( 1.f - std::exp( -8.f * dt ) );
			if ( tick_intro( dt ) )
			{
				m_intro_anim += ( 1.f - m_intro_anim ) * ( 1.f - std::exp( -3.6f * dt ) );
				if ( std::fabs( 1.f - m_intro_anim ) < 0.001f )
					m_intro_anim = 1.f;
			}

			if ( !m_hide_label )
			{
				const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, m_label.c_str( ), g_style->text_body );
				g_render->text( c_fonts::k_default_key, c_vector_2d( m_pos.x + g_style->label_pad, m_pos.y + std::floor( ( row_h - ts.y ) * 0.5f ) ), g_style->text, m_label.c_str( ), g_style->text_body );
			}

			const float reveal = c_render::ease_in_out_quint( m_intro_anim );
			const float fr = g_style->frame_rounding + 1.f;
			const float scale = ImLerp( 0.92f, 1.f, reveal ) * ImLerp( 1.f, 0.96f, m_press );
			const float dw = bw * scale;
			const float dh = bh * scale;
			const float bx = m_box.x + ( bw - dw ) * 0.5f;
			const float by = m_box.y + ( bh - dh ) * 0.5f;

			const c_color fill = g_style->frame
				.lerp( c_color( 36, 40, 56 ), m_hover * 0.7f )
				.lerp( g_style->accent.with_alpha( 90 ), m_hover * 0.35f + m_flash * 0.55f )
				.with_alpha( static_cast<int>( 255.f * ( 0.4f + 0.6f * reveal ) ) );
			const c_color border = g_style->frame_border.lerp( g_style->accent, m_hover * 0.55f + m_flash * 0.45f ).with_alpha( static_cast<int>( 255.f * reveal ) );

			g_render->rect_filled_f( bx, by, dw, dh, fill, fr );
			g_render->rect_f( bx, by, dw, dh, border, fr, 1.f );

			const char* text = m_button_text.empty( ) ? "Apply" : m_button_text.c_str( );
			const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, text, g_style->text_control );
			g_render->text(
				c_fonts::k_default_key,
				c_vector_2d( bx + std::floor( ( dw - ts.x ) * 0.5f ), by + std::floor( ( dh - ts.y ) * 0.5f ) ),
				g_style->text_control_col.lerp( g_style->text_bright, m_hover ).with_alpha( static_cast<int>( 255.f * reveal ) ),
				text,
				g_style->text_control );
		}

		void input( ) override
		{
			if ( !is_visible( ) || !g_input || !g_ctx || !g_style )
				return;
			if ( !g_ctx->can_interact( this, m_focus_priority ) )
			{
				m_pressed = false;
				return;
			}

			const float width = m_parent_width > 0.f ? m_parent_width : m_size.x;
			const float bw = g_style->control_w_for( width );
			const float bh = g_style->combo_h + 2.f;
			m_box = c_vector_2d( g_style->control_x( m_pos.x, width ), m_pos.y + std::floor( ( g_style->row_height - bh ) * 0.5f ) );
			m_box_w = bw;
			m_box_h = bh;

			const bool in = g_input->mouse_in_region( m_box, c_vector_2d( bw, bh ) );
			m_pressed = in && g_input->click_down( mouse_buttons::left );

			if ( in && g_input->clicked( mouse_buttons::left ) )
			{
				m_flash = 1.f;
				if ( m_callback )
					m_callback( );
			}
		}

		void set_button_text( std::string text )
		{
			m_button_text = std::move( text );
		}

	private:
		std::function<void( )> m_callback {};
		std::string m_button_text { "Apply" };
		float m_hover { 0.f };
		float m_press { 0.f };
		float m_flash { 0.f };
		float m_intro_anim { 1.f };
		bool m_pressed { false };
		c_vector_2d m_box {};
		float m_box_w { 0.f };
		float m_box_h { 0.f };
	};
}
