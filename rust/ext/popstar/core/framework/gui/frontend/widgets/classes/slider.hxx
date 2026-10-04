#pragma once

#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <type_traits>

#include <core/framework/gui/frontend/widgets/classes/base_element.hxx>
#include <core/framework/gui/frontend/widgets/classes/bind_popup.hxx>
#include <core/framework/gui/frontend/widgets/settings.hxx>
#include <core/framework/gui/backend/render/render.hxx>
#include <core/framework/gui/backend/blur/blur.hxx>

namespace core::gui
{
	template <typename T>
	class c_slider : public c_base_element
	{
	public:
		c_slider( std::string label, T* value, T min_v, T max_v, bool hide_label = false )
			: m_value( value ), m_min( min_v ), m_max( max_v )
		{
			m_label = std::move( label );
			m_type = element_type::slider;
			m_visible = true;
			m_focus_priority = focus_priority::interactive;
			m_size = c_vector_2d( 200.f, g_style ? g_style->row_height : 37.f );
			if ( hide_label )
				this->hide_label( );
			if ( m_value )
				m_anim = normalize( *m_value );
		}

		void play_intro( ) override
		{
			m_reveal = 0.f;
			m_anim = 0.f;
		}

		void close_overlay( ) override
		{
			if ( m_binds )
				m_binds->close_overlay( );
		}

		bool hosts( c_base_element* element ) const override
		{
			return m_binds && ( m_binds.get( ) == element || m_binds->hosts( element ) );
		}

		c_slider& attach_binds( )
		{
			bind_slot_t slot {};
			slot.label = m_label;
			slot.ptr = m_value;
			if constexpr ( std::is_integral_v<T> )
			{
				slot.kind = bind_kind::integer;
				slot.min_i = static_cast<int>( m_min );
				slot.max_i = static_cast<int>( m_max );
			}
			else
			{
				slot.kind = bind_kind::floating;
				slot.min_f = static_cast<float>( m_min );
				slot.max_f = static_cast<float>( m_max );
			}
			m_binds = std::make_shared<c_bind_popup>( std::move( slot ) );
			return *this;
		}

		void draw( ) override
		{
			if ( !is_visible( ) || !g_render || !g_style || !m_value )
				return;

			const float row_h = g_style->row_height;
			m_size.y = row_h;
			const float width = m_parent_width > 0.f ? m_parent_width : m_size.x;
			m_size.x = width;

			const float dt = ImGui::GetIO( ).DeltaTime;
			if ( tick_intro( dt ) )
			{
				const float target = normalize( *m_value );
				m_anim += ( target - m_anim ) * ( 1.f - std::exp( -9.5f * dt ) );
				if ( std::fabs( m_anim - target ) < 0.0005f )
					m_anim = target;
			}

			const float knob_target = m_active ? 1.f : 0.f;
			m_knob_anim += ( knob_target - m_knob_anim ) * ( 1.f - std::exp( -22.f * dt ) );
			m_tip_anim += ( knob_target - m_tip_anim ) * ( 1.f - std::exp( -8.f * dt ) );

			if ( !m_hide_label )
			{
				const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, m_label.c_str( ), g_style->text_body );
				g_render->text(
					c_fonts::k_default_key,
					c_vector_2d( m_pos.x + g_style->label_pad, m_pos.y + std::floor( ( row_h - ts.y ) * 0.5f ) ),
					g_style->text,
					m_label.c_str( ),
					g_style->text_body );
			}

			const float th = g_style->slider_track_h;
			const float round = th * 0.5f;
			m_track_w = g_style->control_w_for( width );
			m_track = c_vector_2d(
				std::floor( g_style->control_x( m_pos.x, width ) ),
				std::floor( m_pos.y + ( row_h - th ) * 0.5f ) );

			if ( m_binds )
			{
				m_binds->m_pos = m_pos;
				m_binds->m_parent_width = width;
				m_binds->set_dots_anchor( c_vector_2d( std::floor( m_track.x - 8.f - c_bind_popup::dots_width( ) ), std::floor( m_pos.y + ( row_h - c_bind_popup::dots_height( ) ) * 0.5f ) ) );
				m_binds->draw( );
			}

			g_render->rect_filled_f( m_track.x, m_track.y, m_track_w, th, g_style->slider_track, round );

			const float fill_w = m_track_w * m_anim;
			if ( fill_w > 0.5f )
			{
				ImDrawList* dl = g_render->draw_list( );
				if ( dl )
				{
					dl->PushClipRect( ImVec2( m_track.x, m_track.y ), ImVec2( m_track.x + fill_w, m_track.y + th ), true );
					g_render->rect_filled_f( m_track.x, m_track.y, m_track_w, th, g_style->accent, round );
					dl->PopClipRect( );
				}
			}

			const float radius = ImLerp( 5.5f, 7.f, m_knob_anim );
			const float knob_x = m_track.x + fill_w;
			const float knob_y = m_track.y + th * 0.5f;
			g_render->circle_shadow( c_vector_2d( knob_x, knob_y ), radius, c_color( 0, 0, 0, 90 ), 10.f );
			g_render->circle_filled( c_vector_2d( knob_x, knob_y ), radius, c_color( 247, 248, 252 ), 48 );

			if ( m_tip_anim > 0.01f )
			{
				const float tip_x = knob_x;
				const float tip_y = knob_y;
				g_render->defer( [this, tip_x, tip_y]( ) { draw_value_tip( tip_x, tip_y ); } );
			}
		}

		void input( ) override
		{
			if ( !is_visible( ) || !m_value || !g_input || !g_ctx || !g_style )
				return;

			const float width = m_parent_width > 0.f ? m_parent_width : m_size.x;
			const float th = g_style->slider_track_h;
			m_track_w = g_style->control_w_for( width );
			m_track = c_vector_2d(
				std::floor( g_style->control_x( m_pos.x, width ) ),
				std::floor( m_pos.y + ( g_style->row_height - th ) * 0.5f ) );

			if ( m_binds )
			{
				m_binds->m_pos = m_pos;
				m_binds->m_parent_width = width;
				m_binds->set_dots_anchor( c_vector_2d( std::floor( m_track.x - 8.f - c_bind_popup::dots_width( ) ), std::floor( m_pos.y + ( g_style->row_height - c_bind_popup::dots_height( ) ) * 0.5f ) ) );
				m_binds->input( );
			}

			if ( !g_ctx->can_interact( this, m_focus_priority ) )
			{
				m_active = false;
				return;
			}

			const c_vector_2d hit( m_track.x - 6.f, m_track.y - 10.f );
			const c_vector_2d hit_size( m_track_w + 12.f, th + 20.f );

			if ( g_input->mouse_in_region( hit, hit_size ) && g_input->clicked( mouse_buttons::left ) )
				m_active = true;

			if ( m_active )
			{
				if ( g_input->click_down( mouse_buttons::left ) )
				{
					const float mx = g_input->get_mouse_position( ).x;
					const float t = ImClamp( ( mx - m_track.x ) / m_track_w, 0.f, 1.f );
					*m_value = denormalize( t );
				}
				else
				{
					m_active = false;
				}
			}
		}

	private:
		T* m_value { nullptr };
		T m_min {};
		T m_max {};
		float m_anim { 0.f };
		float m_knob_anim { 0.f };
		float m_tip_anim { 0.f };
		bool m_active { false };
		c_vector_2d m_track {};
		float m_track_w { 0.f };
		std::shared_ptr<c_bind_popup> m_binds {};

		float normalize( T v ) const
		{
			const float span = static_cast<float>( m_max - m_min );
			if ( span <= 0.f )
				return 0.f;
			return ImClamp( ( static_cast<float>( v ) - static_cast<float>( m_min ) ) / span, 0.f, 1.f );
		}

		T denormalize( float t ) const
		{
			if constexpr ( std::is_integral_v<T> )
				return m_min + static_cast<T>( std::lround( static_cast<float>( m_max - m_min ) * t ) );
			else
				return static_cast<T>( static_cast<float>( m_min ) + static_cast<float>( m_max - m_min ) * t );
		}

		void draw_value_tip( float knob_x, float knob_y )
		{
			char buf[32] {};
			if constexpr ( std::is_integral_v<T> )
				std::snprintf( buf, sizeof( buf ), "%d", static_cast<int>( *m_value ) );
			else
				std::snprintf( buf, sizeof( buf ), "%d", static_cast<int>( std::lround( static_cast<double>( *m_value ) ) ) );

			const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, buf, g_style->text_control );
			const float pad_x = 10.f;
			const float pad_y = 6.f;
			const float bw = std::floor( ts.x + pad_x * 2.f );
			const float bh = std::floor( ts.y + pad_y * 2.f );
			const float bx = std::floor( knob_x - bw * 0.5f );
			const float by = std::floor( knob_y - bh - 14.f );
			const float fr = g_style->frame_rounding + 1.f;
			const float text_x = bx + std::floor( ( bw - ts.x ) * 0.5f );
			const float text_y = by + std::floor( ( bh - ts.y ) * 0.5f );

			const int vtx = g_render->vtx_count( );
			const c_vector_2d pivot( knob_x, by + bh );
			g_render->frosted_panel( bx, by, bw, bh, fr, true );
			g_render->text( c_fonts::k_default_key, c_vector_2d( text_x, text_y ), c_color( 236, 238, 245 ), buf, g_style->text_control );
			g_render->apply_open( vtx, pivot, m_tip_anim, 0.88f, 12.f );
		}
	};
}
