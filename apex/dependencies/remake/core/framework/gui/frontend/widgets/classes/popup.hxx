#pragma once

#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <core/framework/gui/frontend/widgets/classes/base_element.hxx>
#include <core/framework/gui/frontend/widgets/classes/checkbox.hxx>
#include <core/framework/gui/frontend/widgets/classes/slider.hxx>
#include <core/framework/gui/frontend/widgets/classes/dropdown.hxx>
#include <core/framework/gui/frontend/widgets/classes/colorpicker.hxx>
#include <core/framework/gui/frontend/widgets/classes/button.hxx>
#include <core/framework/gui/frontend/widgets/settings.hxx>
#include <core/framework/gui/backend/render/render.hxx>

namespace core::gui
{
	class c_popup_section
	{
		friend class c_popup;
		friend class c_options;

	public:
		std::shared_ptr<c_checkbox> add_checkbox( std::string label, bool* val )
		{
			auto el = std::make_shared<c_checkbox>( std::move( label ), val );
			m_controls.push_back( el );
			return el;
		}

		template <typename T>
		std::shared_ptr<c_slider<T>> add_slider( std::string label, T* val, T min_v, T max_v, bool hide_label = false )
		{
			auto el = std::make_shared<c_slider<T>>( std::move( label ), val, min_v, max_v, hide_label );
			m_controls.push_back( el );
			return el;
		}

		std::shared_ptr<c_dropdown> add_dropdown( std::string label, int* val, std::vector<std::string> items, bool hide_label = false )
		{
			auto el = std::make_shared<c_dropdown>( std::move( label ), val, std::move( items ), hide_label );
			m_controls.push_back( el );
			return el;
		}

		std::shared_ptr<c_colorpicker> add_colorpicker( std::string label, c_color* val, bool hide_label = false, bool* enabled = nullptr )
		{
			auto el = std::make_shared<c_colorpicker>( std::move( label ), val, hide_label, enabled );
			m_controls.push_back( el );
			return el;
		}

		std::shared_ptr<c_button> add_button( std::string label, std::function<void( )> callback )
		{
			auto el = std::make_shared<c_button>( std::move( label ), std::move( callback ) );
			m_controls.push_back( el );
			return el;
		}

		float content_height( ) const
		{
			const float row_h = g_style ? g_style->row_height : 37.f;
			int n = 0;
			for ( const auto& c : m_controls )
			{
				if ( c && c->is_visible( ) )
					++n;
			}
			return ( std::max )( 1, n ) * row_h;
		}

	private:
		std::vector<std::shared_ptr<c_base_element>> m_controls {};
	};

	class c_popup : public c_base_element
	{
	public:
		static constexpr float dots_width( ) { return 28.f; }
		static constexpr float dots_height( ) { return 22.f; }

		c_popup( std::string label )
		{
			m_label = std::move( label );
			m_type = element_type::popup;
			m_visible = true;
			m_layer = render_layer::modal;
			m_focus_priority = focus_priority::modal;
			m_size = c_vector_2d( 200.f, g_style ? g_style->row_height : 37.f );
		}

		void set_companion( bool v ) { m_companion = v; }
		void set_dots_anchor( c_vector_2d p ) { m_dots = p; m_anchored = true; }

		bool hosts( c_base_element* element ) const override
		{
			for ( const auto& sec : m_sections )
			{
				for ( const auto& c : sec->m_controls )
				{
					if ( c.get( ) == element )
						return true;
					if ( c && c->hosts( element ) )
						return true;
				}
			}
			return false;
		}

		c_popup_section* section( std::function<void( c_popup_section* )> build )
		{
			auto sec = std::make_shared<c_popup_section>( );
			if ( build )
				build( sec.get( ) );
			m_sections.push_back( sec );
			return sec.get( );
		}

		void play_intro( ) override
		{
			m_reveal = 0.f;
			play_children_intro( );
		}

		void play_children_intro( )
		{
			int i = 0;
			for ( const auto& sec : m_sections )
			{
				if ( !sec )
					continue;
				for ( const auto& c : sec->m_controls )
				{
					if ( !c || !c->is_visible( ) )
						continue;
					c->play_intro( );
					c->set_intro_hold( 0.08f + static_cast<float>( i ) * 0.04f );
					++i;
				}
			}
		}

		void close_overlay( ) override
		{
			m_open = false;
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

			if ( !m_companion && !m_hide_label )
			{
				const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, m_label.c_str( ), g_style->text_body );
				g_render->text( c_fonts::k_default_key, c_vector_2d( m_pos.x + g_style->label_pad, m_pos.y + std::floor( ( row_h - ts.y ) * 0.5f ) ), g_style->text, m_label.c_str( ), g_style->text_body );
			}

			if ( !m_anchored )
				layout_dots( width );

			const float dt = ImGui::GetIO( ).DeltaTime;
			if ( g_ctx && !g_ctx->m_open )
			{
				m_open = false;
				m_open_anim = 0.f;
			}
			else
			{
				const bool hovered = g_input && g_input->mouse_in_region( m_dots, c_vector_2d( dots_width( ), dots_height( ) ) );
				m_hover += ( ( hovered || m_open ? 1.f : 0.f ) - m_hover ) * ( 1.f - std::exp( -18.f * dt ) );
				m_open_anim += ( ( m_open ? 1.f : 0.f ) - m_open_anim ) * ( 1.f - std::exp( -3.4f * dt ) );
				if ( std::fabs( m_open_anim - ( m_open ? 1.f : 0.f ) ) < 0.0008f )
					m_open_anim = m_open ? 1.f : 0.f;
			}

			const float fr = 5.f;
			const float dw = dots_width( );
			const float dh = dots_height( );

			const float cy = m_dots.y + dh * 0.5f;
			const float cx = m_dots.x + dw * 0.5f;
			const c_color dot = c_color( 170, 174, 186 ).lerp( g_style->text_bright, m_hover ).lerp( g_style->accent, m_open_anim * 0.65f );
			for ( int i = -1; i <= 1; ++i )
				g_render->circle_filled( c_vector_2d( cx + static_cast<float>( i ) * 5.f, cy ), 1.7f, dot, 12 );

			if ( m_open_anim > 0.01f && g_ctx && g_ctx->m_open && g_ctx->m_open_anim > 0.05f )
				g_render->defer( [this]( ) { draw_popup( ); } );

			m_anchored = false;
		}

		void input( ) override
		{
			if ( !is_visible( ) || !g_input || !g_ctx || !g_style )
				return;

			const float width = m_parent_width > 0.f ? m_parent_width : m_size.x;
			if ( !m_anchored )
				layout_dots( width );

			if ( m_open && g_ctx && g_ctx->m_open && !g_ctx->m_modal_owner )
				g_ctx->set_modal( this, [this]( c_base_element* el ) { return hosts( el ); } );

			const bool nested_modal = g_ctx->m_modal_owner && hosts( g_ctx->m_modal_owner );
			const bool can = g_ctx->can_interact( this, m_focus_priority ) || g_ctx->m_modal_owner == this || nested_modal;
			if ( !can )
				return;

			const bool dots_click = g_input->mouse_in_region( m_dots, c_vector_2d( dots_width( ), dots_height( ) ) ) && g_input->clicked( mouse_buttons::left );
			if ( dots_click )
			{
				if ( !m_open )
				{
					m_open = true;
					m_ignore_click = true;
					play_children_intro( );
					g_ctx->set_modal( this, [this]( c_base_element* el ) { return hosts( el ); } );
				}
				else
				{
					close( );
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
			const float card_w = k_popup_w - k_pad * 2.f;
			const float content_w = card_w - k_card_inset * 2.f;
			const float content_x = pp.x + k_pad + k_card_inset;
			const float row_h = g_style->row_height;

			float cy = pp.y + k_header;
			for ( const auto& sec : m_sections )
			{
				float row_y = cy;
				for ( const auto& c : sec->m_controls )
				{
					if ( !c || !c->is_visible( ) )
						continue;
					c->m_pos = c_vector_2d( content_x, row_y );
					c->m_size = c_vector_2d( content_w, row_h );
					c->m_parent_width = content_w;
					c->input( );
					row_y += row_h;
				}
				cy += sec->content_height( ) + k_sec_gap;
			}

			if ( g_input->clicked( mouse_buttons::left ) && !nested_modal )
			{
				if ( !g_input->mouse_in_region( pp, psz ) && !g_input->mouse_in_region( m_dots, c_vector_2d( dots_width( ), dots_height( ) ) ) )
					close( );
			}
		}

	private:
		static constexpr float k_popup_w = 240.f;
		static constexpr float k_header = 30.f;
		static constexpr float k_pad = 12.f;
		static constexpr float k_card_inset = 6.f;
		static constexpr float k_sec_gap = 8.f;

		std::vector<std::shared_ptr<c_popup_section>> m_sections {};
		bool m_open { false };
		bool m_ignore_click { false };
		bool m_companion { false };
		bool m_anchored { false };
		float m_hover { 0.f };
		float m_open_anim { 0.f };
		c_vector_2d m_dots {};

		void close( )
		{
			m_open = false;
			m_ignore_click = false;
			if ( g_ctx )
				g_ctx->clear_modal( this );
		}

		void layout_dots( float width )
		{
			const float row_h = g_style->row_height;
			const float cw = g_style->control_w_for( width );
			m_dots = c_vector_2d(
				std::floor( g_style->control_x( m_pos.x, width ) + cw - dots_width( ) ),
				std::floor( m_pos.y + ( row_h - dots_height( ) ) * 0.5f ) );
		}

		c_vector_2d popup_size( ) const
		{
			float h = k_header + k_pad;
			for ( std::size_t i = 0; i < m_sections.size( ); ++i )
			{
				h += m_sections[i]->content_height( );
				if ( i + 1 < m_sections.size( ) )
					h += k_sec_gap;
			}
			h += k_pad;
			if ( m_sections.empty( ) )
				h = k_header + k_pad * 2.f + ( g_style ? g_style->row_height : 37.f );
			return c_vector_2d( k_popup_w, h );
		}

		c_vector_2d popup_pos( ) const
		{
			const c_vector_2d psz = popup_size( );
			float x = std::floor( m_dots.x + dots_width( ) - psz.x );
			float y = std::floor( m_dots.y + dots_height( ) + 8.f );
			const ImVec2 display = ImGui::GetIO( ).DisplaySize;
			if ( x < 8.f )
				x = 8.f;
			if ( x + psz.x > display.x - 8.f )
				x = display.x - psz.x - 8.f;
			if ( y + psz.y > display.y - 8.f )
				y = std::floor( m_dots.y - psz.y - 8.f );
			return c_vector_2d( x, y );
		}

		void draw_popup( )
		{
			const c_vector_2d pp = popup_pos( );
			const c_vector_2d psz = popup_size( );
			const float e = c_render::smoothstep( m_open_anim );
			const int vtx = g_render->vtx_count( );
			const float fr = 8.f;
			const c_vector_2d pivot( m_dots.x + dots_width( ) * 0.5f, m_dots.y + dots_height( ) * 0.5f );
			const float card_w = k_popup_w - k_pad * 2.f;
			const float card_x = pp.x + k_pad;
			const float content_w = card_w - k_card_inset * 2.f;
			const float content_x = card_x + k_card_inset;
			const float card_r = g_style->card_rounding;

			g_render->frosted_panel( pp.x, pp.y, psz.x, psz.y, fr, true );

			const c_vector_2d title_ts = g_render->measure_text( c_fonts::k_default_key, m_label.c_str( ), g_style->text_body );
			g_render->text(
				c_fonts::k_default_key,
				c_vector_2d( pp.x + k_pad, pp.y + std::floor( ( k_header - title_ts.y ) * 0.5f ) ),
				g_style->text_bright.with_alpha( static_cast<int>( 255.f * e ) ),
				m_label.c_str( ),
				g_style->text_body );

			float cy = pp.y + k_header;
			const float row_h = g_style->row_height;

			for ( const auto& sec : m_sections )
			{
				const float sh = sec->content_height( );
				g_render->rect_filled_f( card_x, cy, card_w, sh, g_style->card_bg, card_r );
				g_render->rect_f( card_x, cy, card_w, sh, g_style->outline, card_r );

				float row_y = cy;
				for ( const auto& c : sec->m_controls )
				{
					if ( !c || !c->is_visible( ) )
						continue;
					c->m_pos = c_vector_2d( content_x, row_y );
					c->m_size = c_vector_2d( content_w, row_h );
					c->m_parent_width = content_w;
					c->advance_reveal( ImGui::GetIO( ).DeltaTime );
					const int cv = g_render->vtx_count( );
					c->draw( );
					g_render->apply_fade( cv, c->reveal_t( ) );
					row_y += row_h;
				}

				cy += sh + k_sec_gap;
			}

			g_render->apply_open( vtx, pivot, m_open_anim, 0.86f, 18.f );
		}
	};
}
