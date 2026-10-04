#pragma once

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include <core/framework/gui/frontend/widgets/classes/base_element.hxx>
#include <core/framework/gui/frontend/widgets/classes/bind_popup.hxx>
#include <core/framework/gui/frontend/widgets/settings.hxx>
#include <core/framework/gui/backend/render/render.hxx>
#include <core/framework/gui/backend/blur/blur.hxx>

namespace core::gui
{
	class c_dropdown : public c_base_element
	{
	public:
		c_dropdown( std::string label, int* value, std::vector<std::string> items, bool hide_label = false )
			: m_value( value ), m_items( std::move( items ) )
		{
			m_label = std::move( label );
			m_type = element_type::dropdown;
			m_visible = true;
			m_focus_priority = focus_priority::modal;
			m_layer = render_layer::modal;
			m_size = c_vector_2d( 200.f, g_style ? g_style->row_height : 37.f );
			if ( hide_label )
				this->hide_label( );
		}

		void play_intro( ) override
		{
			m_reveal = 0.f;
			m_value_anim = 0.f;
			m_hover = 0.f;
		}

		void close_overlay( ) override
		{
			m_open = false;
			m_ignore_click = false;
			if ( g_ctx )
				g_ctx->clear_modal( this );
			if ( m_binds )
				m_binds->close_overlay( );
		}

		bool hosts( c_base_element* element ) const override
		{
			if ( element == this )
				return true;
			return m_binds && ( m_binds.get( ) == element || m_binds->hosts( element ) );
		}

		c_dropdown& attach_binds( )
		{
			bind_slot_t slot {};
			slot.label = m_label;
			slot.kind = bind_kind::integer;
			slot.ptr = m_value;
			slot.min_i = 0;
			slot.max_i = m_items.empty( ) ? 0 : static_cast<int>( m_items.size( ) ) - 1;
			slot.items = m_items;
			m_binds = std::make_shared<c_bind_popup>( std::move( slot ) );
			return *this;
		}

		void draw( ) override
		{
			if ( !is_visible( ) || !g_render || !g_style )
				return;

			const float row_h = g_style->row_height;
			const float box_h = g_style->combo_h;
			const float item_h = g_style->combo_item_h;
			const float pad = g_style->combo_pad;
			m_size.y = row_h;
			const float width = m_parent_width > 0.f ? m_parent_width : m_size.x;
			m_size.x = width;

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

			const float cw = g_style->control_w_for( width );
			m_box = c_vector_2d( g_style->control_x( m_pos.x, width ), m_pos.y + std::floor( ( row_h - box_h ) * 0.5f ) );
			m_box_w = cw;
			m_box_h = box_h;

			if ( m_binds )
			{
				m_binds->m_pos = m_pos;
				m_binds->m_parent_width = width;
				m_binds->set_dots_anchor( c_vector_2d( std::floor( m_box.x - 8.f - c_bind_popup::dots_width( ) ), std::floor( m_pos.y + ( row_h - c_bind_popup::dots_height( ) ) * 0.5f ) ) );
				m_binds->draw( );
			}

			const bool hovered = g_input && g_input->mouse_in_region( m_box, c_vector_2d( cw, box_h ) );
			const float dt = ImGui::GetIO( ).DeltaTime;
			m_hover += ( ( hovered || m_open ? 1.f : 0.f ) - m_hover ) * ( 1.f - std::exp( -20.f * dt ) );
			if ( g_ctx && !g_ctx->m_open )
			{
				m_open = false;
				m_open_anim = 0.f;
			}
			else
			{
				m_open_anim += ( ( m_open ? 1.f : 0.f ) - m_open_anim ) * ( 1.f - std::exp( -3.4f * dt ) );
			}
			if ( tick_intro( dt ) )
			{
				m_value_anim += ( 1.f - m_value_anim ) * ( 1.f - std::exp( -3.6f * dt ) );
				if ( std::fabs( 1.f - m_value_anim ) < 0.001f )
					m_value_anim = 1.f;
			}

			while ( static_cast<int>( m_item_hover.size( ) ) < static_cast<int>( m_items.size( ) ) )
				m_item_hover.push_back( 0.f );

			const c_vector_2d pp = popup_pos( );
			for ( int i = 0; i < static_cast<int>( m_items.size( ) ); ++i )
			{
				const c_vector_2d rp( pp.x, pp.y + pad + i * item_h );
				const bool ih = m_open && g_input && g_input->mouse_in_region( rp, c_vector_2d( cw, item_h ) );
				m_item_hover[i] += ( ( ih ? 1.f : 0.f ) - m_item_hover[i] ) * ( 1.f - std::exp( -26.f * dt ) );
			}

			const float fr = g_style->frame_rounding;
			const float reveal = c_render::smoothstep( m_value_anim );
			const c_color fill = g_style->frame.lerp( c_color( 30, 34, 46 ), m_hover * 0.55f );
			g_render->rect_filled( static_cast<int>( m_box.x ), static_cast<int>( m_box.y ), static_cast<int>( cw ), static_cast<int>( box_h ), fill.with_alpha( static_cast<int>( 255.f * ( 0.55f + 0.45f * reveal ) ) ), fr );
			g_render->rect( static_cast<int>( m_box.x ), static_cast<int>( m_box.y ), static_cast<int>( cw ), static_cast<int>( box_h ), g_style->frame_border.with_alpha( static_cast<int>( 255.f * reveal ) ), fr );

			const char* text = "Select";
			if ( m_value && *m_value >= 0 && *m_value < static_cast<int>( m_items.size( ) ) )
				text = m_items[*m_value].c_str( );

			const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, text, g_style->text_control );
			g_render->text(
				c_fonts::k_default_key,
				c_vector_2d( m_box.x + 9.f, m_box.y + std::floor( ( box_h - ts.y ) * 0.5f ) ),
				g_style->text_control_col.with_alpha( static_cast<int>( 255.f * reveal ) ),
				text,
				g_style->text_control );

			const float cx = std::floor( m_box.x + cw - 14.f );
			const float cy = std::floor( m_box.y + box_h * 0.5f );
			const float open_e = c_render::ease_out_cubic( m_open_anim );
			const c_color chev = c_color( 150, 154, 166 ).lerp( g_style->accent, open_e * 0.55f ).with_alpha( static_cast<int>( 255.f * reveal ) );
			if ( ImDrawList* dl = g_render->draw_list( ) )
			{
				const float s = 3.25f;
				const float flip = ImLerp( 1.f, -1.f, open_e );
				const ImVec2 p0( cx, cy + 2.1f * flip );
				const ImVec2 p1( cx - s, cy - 1.6f * flip );
				const ImVec2 p2( cx + s, cy - 1.6f * flip );
				const int a = static_cast<int>( static_cast<float>( chev.a ) * g_render->alpha( ) );
				const ImU32 col = IM_COL32( chev.r, chev.g, chev.b, a );
				const ImDrawListFlags old = dl->Flags;
				dl->Flags &= ~ImDrawListFlags_AntiAliasedFill;
				dl->AddTriangleFilled( p0, p1, p2, col );
				dl->Flags = old;
			}

			if ( m_open_anim > 0.01f && !m_items.empty( ) && g_ctx && g_ctx->m_open && g_ctx->m_open_anim > 0.05f )
				g_render->defer( [this]( ) { draw_popup( ); } );
		}

		void input( ) override
		{
			if ( !is_visible( ) || !m_value || !g_input || !g_ctx || !g_style )
				return;

			const float width = m_parent_width > 0.f ? m_parent_width : m_size.x;
			const float cw = g_style->control_w_for( width );
			const float box_h = g_style->combo_h;
			const float item_h = g_style->combo_item_h;
			const float pad = g_style->combo_pad;
			m_box = c_vector_2d( g_style->control_x( m_pos.x, width ), m_pos.y + std::floor( ( g_style->row_height - box_h ) * 0.5f ) );
			m_box_w = cw;
			m_box_h = box_h;

			if ( m_binds )
			{
				m_binds->m_pos = m_pos;
				m_binds->m_parent_width = width;
				m_binds->set_dots_anchor( c_vector_2d( std::floor( m_box.x - 8.f - c_bind_popup::dots_width( ) ), std::floor( m_pos.y + ( g_style->row_height - c_bind_popup::dots_height( ) ) * 0.5f ) ) );
				m_binds->input( );
			}

			const bool box_click = g_input->mouse_in_region( m_box, c_vector_2d( cw, box_h ) ) && g_input->clicked( mouse_buttons::left );
			const bool under_parent_modal = g_ctx->m_modal_owner && g_ctx->m_modal_contains && g_ctx->m_modal_contains( this );
			const bool can_open = g_ctx->can_interact( this, m_focus_priority ) || under_parent_modal;

			if ( box_click && !m_open )
			{
				if ( can_open )
				{
					m_open = true;
					m_ignore_click = true;
					g_ctx->set_modal( this );
				}
				return;
			}

			if ( !m_open )
				return;

			if ( g_ctx->m_modal_owner != this && !g_ctx->can_interact( this, m_focus_priority ) )
				return;

			if ( m_ignore_click )
			{
				if ( !g_input->click_down( mouse_buttons::left ) )
					m_ignore_click = false;
				return;
			}

			// Wait for open anim so hitboxes match the scaled popup.
			if ( m_open_anim < 0.72f )
				return;

			const c_vector_2d pp = popup_pos( );
			const float list_h = popup_height( );
			bool hit_any = false;

			for ( int i = 0; i < static_cast<int>( m_items.size( ) ); ++i )
			{
				const c_vector_2d rp( pp.x, pp.y + pad + i * item_h );
				if ( g_input->mouse_in_region( rp, c_vector_2d( cw, item_h ) ) && g_input->clicked( mouse_buttons::left ) )
				{
					*m_value = i;
					m_open = false;
					g_ctx->clear_modal( this );
					hit_any = true;
					break;
				}
			}

			if ( !hit_any && g_input->clicked( mouse_buttons::left ) )
			{
				if ( box_click )
				{
					m_open = false;
					g_ctx->clear_modal( this );
				}
				else if ( !g_input->mouse_in_region( pp, c_vector_2d( cw, list_h ) ) )
				{
					m_open = false;
					g_ctx->clear_modal( this );
				}
			}
		}

	private:
		int* m_value { nullptr };
		std::vector<std::string> m_items {};
		std::vector<float> m_item_hover {};
		bool m_open { false };
		bool m_ignore_click { false };
		float m_hover { 0.f };
		float m_open_anim { 0.f };
		float m_value_anim { 1.f };
		c_vector_2d m_box {};
		float m_box_w { 0.f };
		float m_box_h { 0.f };
		std::shared_ptr<c_bind_popup> m_binds {};

		float popup_height( ) const
		{
			const float item_h = g_style->combo_item_h;
			const float pad = g_style->combo_pad;
			return static_cast<float>( m_items.size( ) ) * item_h + pad * 2.f;
		}

		c_vector_2d popup_pos( ) const
		{
			const float list_h = popup_height( );
			return c_vector_2d( m_box.x, m_box.y + m_box_h * 0.5f - list_h * 0.5f );
		}

		void draw_popup( )
		{
			const float cw = m_box_w;
			const float list_h = popup_height( );
			const float item_h = g_style->combo_item_h;
			const float pad = g_style->combo_pad;
			const c_vector_2d pp = popup_pos( );
			const int vtx = g_render->vtx_count( );
			ImDrawList* dl = g_render->draw_list( );
			const float fr = g_style->frame_rounding;
			const c_vector_2d pivot( m_box.x + cw * 0.5f, m_box.y + m_box_h * 0.5f );
			const int n = static_cast<int>( m_items.size( ) );

			g_render->frosted_panel( pp.x, pp.y, cw, list_h, fr, true );

			const float inset_x = 5.f;
			const float text_pad = 8.f;
			const float pill_h = item_h - 4.f;
			const float pill_r = 5.f;
			const float pill_w = cw - inset_x * 2.f;
			const float fs = g_style->text_control;

			for ( int i = 0; i < n; ++i )
			{
				const float ih = m_item_hover[i];
				if ( ih <= 0.01f )
					continue;

				const float he = ih * ih * ( 3.f - 2.f * ih );
				const float row_y = pp.y + pad + i * item_h;
				const float row_cy = row_y + item_h * 0.5f;
				const float ps = ImLerp( 0.90f, 1.f, he );
				const float pw = pill_w * ps;
				const float ph = pill_h * ps;
				const float hx = pp.x + inset_x + ( pill_w - pw ) * 0.5f;
				const float hy = row_cy - ph * 0.5f;

				c_blur* hover_blur = ( g_blur_ui && g_blur_ui->ready( ) ) ? g_blur_ui.get( ) : g_blur.get( );
				if ( hover_blur && hover_blur->ready( ) && dl )
					hover_blur->draw_region( dl, c_vector_2d( hx, hy ), c_vector_2d( hx + pw, hy + ph ), pill_r, c_color( 255, 255, 255, static_cast<int>( 30.f * he ) ) );

				g_render->rect_filled_f( hx, hy, pw, ph, c_color( 22, 25, 36, static_cast<int>( 220.f * he ) ), pill_r );
				g_render->rect_filled_f( hx, hy, pw, ph, c_color( 255, 255, 255, static_cast<int>( 12.f * he ) ), pill_r );
				g_render->stroke_rounded( hx, hy, pw, ph, pill_r, c_color( 48, 52, 66, static_cast<int>( 170.f * he ) ), 1.f );
			}

			for ( int i = 0; i < n; ++i )
			{
				const float row_y = pp.y + pad + i * item_h;
				const float row_cy = row_y + item_h * 0.5f;
				const bool sel = m_value && *m_value == i;
				const float tx = pp.x + inset_x + text_pad;
				const c_vector_2d its = g_render->measure_text( c_fonts::k_default_key, m_items[i].c_str( ), fs );
				const float ty = std::floor( row_cy - its.y * 0.5f );
				g_render->text( c_fonts::k_default_key, c_vector_2d( tx, ty ), sel ? g_style->text_bright : g_style->text_control_col, m_items[i].c_str( ), fs );
			}

			g_render->apply_open( vtx, pivot, m_open_anim, 0.86f, 18.f );
		}
	};
}
