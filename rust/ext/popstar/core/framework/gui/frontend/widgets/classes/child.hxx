#pragma once

#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <core/framework/gui/frontend/widgets/classes/base_element.hxx>
#include <core/framework/gui/frontend/widgets/classes/checkbox.hxx>
#include <core/framework/gui/frontend/widgets/classes/button.hxx>
#include <core/framework/gui/frontend/widgets/classes/slider.hxx>
#include <core/framework/gui/frontend/widgets/classes/dropdown.hxx>
#include <core/framework/gui/frontend/widgets/classes/multidropdown.hxx>
#include <core/framework/gui/frontend/widgets/classes/colorpicker.hxx>
#include <core/framework/gui/frontend/widgets/classes/text_input.hxx>
#include <core/framework/gui/frontend/widgets/classes/listbox.hxx>
#include <core/framework/gui/frontend/widgets/classes/popup.hxx>
#include <core/framework/gui/frontend/widgets/classes/options.hxx>
#include <core/framework/gui/frontend/widgets/classes/bind_popup.hxx>
#include <core/framework/gui/backend/manager/keybinds/keybinds.hxx>
#include <core/framework/gui/frontend/widgets/settings.hxx>
#include <core/framework/gui/frontend/widgets/classes/context.hxx>
#include <core/framework/gui/backend/render/render.hxx>
#include <popstar_glue.hpp>

namespace core::gui
{
	struct child_data_t
	{
		std::string m_tab_name {};
		std::string m_subtab_name {};
		std::string m_side_name {};
	};

	enum class child_width : int
	{
		full,
		half
	};

	class c_window;

	class c_child : public std::enable_shared_from_this<c_child>
	{
		friend class c_window;

	public:
		c_child( std::string name, child_width width, float y )
			: m_name( std::move( name ) ), m_width_type( width )
		{
			m_relative_pos = c_vector_2d( 0.f, y );
			m_size = c_vector_2d( g_style ? g_style->child_w : 279.f, 221.f );
			m_visible = true;
		}

		void draw( )
		{
			if ( !m_visible || !g_render || !g_style )
				return;

			recompute_height( );

			const float label_pad = g_style->label_pad;
			const c_vector_2d ls = g_render->measure_text( c_fonts::k_caption_key, m_name.c_str( ), g_style->text_caption );
			g_render->text(
				c_fonts::k_caption_key,
				c_vector_2d( m_pos.x + label_pad, m_pos.y - g_style->child_label_gap + std::floor( ( g_style->child_label_gap - ls.y ) * 0.5f ) ),
				c_color( 89, 94, 106 ),
				m_name.c_str( ),
				g_style->text_caption );

			g_render->rect_filled(
				static_cast<int>( m_pos.x ),
				static_cast<int>( m_pos.y ),
				static_cast<int>( m_size.x ),
				static_cast<int>( m_size.y ),
				g_style->card_bg,
				g_style->card_rounding );

			g_render->rect(
				static_cast<int>( m_pos.x ),
				static_cast<int>( m_pos.y ),
				static_cast<int>( m_size.x ),
				static_cast<int>( m_size.y ),
				g_style->outline,
				g_style->card_rounding );

			const float row_h = g_style->row_height;
			float cursor_y = m_pos.y - m_scroll_offset;
			int row = 0;
			const float dt = ImGui::GetIO( ).DeltaTime;
			const int total_rows = visible_count( );
			const float round = g_style->card_rounding;

			g_render->push_clip(
				static_cast<int>( m_pos.x ),
				static_cast<int>( m_pos.y ),
				static_cast<int>( m_size.x ),
				static_cast<int>( m_size.y ) );

			for ( const std::shared_ptr<c_base_element>& control : m_controls )
			{
				if ( !control || !control->is_visible( ) )
					continue;

				while ( static_cast<int>( m_row_hover.size( ) ) <= row )
					m_row_hover.push_back( 0.f );

				const bool row_hovered = g_input && !g_ctx->m_modal_owner && g_input->mouse_in_region( c_vector_2d( m_pos.x, cursor_y ), c_vector_2d( m_size.x, row_h ) );
				m_row_hover[row] += ( ( row_hovered ? 1.f : 0.f ) - m_row_hover[row] ) * ( 1.f - std::exp( -18.f * dt ) );

				if ( m_row_hover[row] > 0.01f )
				{
					draw_flags flags = draw_flags_round_corners_none;
					float hl_round = 0.f;
					if ( total_rows == 1 )
					{
						flags = draw_flags_none;
						hl_round = round;
					}
					else if ( row == 0 )
					{
						flags = static_cast<draw_flags>( draw_flags_round_corners_top_left | draw_flags_round_corners_top_right );
						hl_round = round;
					}
					else if ( row == total_rows - 1 )
					{
						flags = static_cast<draw_flags>( draw_flags_round_corners_bottom_left | draw_flags_round_corners_bottom_right );
						hl_round = round;
					}

					g_render->rect_filled(
						static_cast<int>( m_pos.x + 1.f ),
						static_cast<int>( cursor_y ),
						static_cast<int>( m_size.x - 2.f ),
						static_cast<int>( row_h ),
						g_style->row_line.with_alpha( static_cast<int>( 200.f * m_row_hover[row] ) ),
						hl_round,
						flags );
				}

				if ( row > 0 && m_row_hover[row] < 0.85f && ( row > 0 && m_row_hover[row - 1] < 0.85f ) )
				{
					g_render->line(
						c_vector_2d( m_pos.x + 12.f, cursor_y ),
						c_vector_2d( m_pos.x + m_size.x - 12.f, cursor_y ),
						g_style->row_line );
				}

				control->m_pos = c_vector_2d( m_pos.x, cursor_y );
				control->m_size = c_vector_2d( m_size.x, row_h );
				control->m_parent_width = m_size.x;
				control->advance_reveal( dt );
				const int control_vtx = g_render->vtx_count( );
				control->draw( );
				g_render->apply_fade( control_vtx, control->reveal_t( ) );
				cursor_y += row_h;
				++row;
			}

			g_render->restore_clip( );

			m_content_height = static_cast<float>( visible_count( ) ) * row_h;
			m_max_scroll = ( std::max )( 0.f, m_content_height - m_size.y );
			if ( m_max_scroll <= 0.f )
			{
				m_max_scroll = 0.f;
				m_scroll_offset = 0.f;
				m_scroll_target = 0.f;
			}
		}

		void input( )
		{
			if ( !m_visible || !g_input )
				return;

			recompute_height( );
			m_content_height = static_cast<float>( visible_count( ) ) * ( g_style ? g_style->row_height : 37.f );
			m_max_scroll = ( std::max )( 0.f, m_content_height - m_size.y );

			if ( m_max_scroll > 0.f && g_input->mouse_in_region( m_pos, m_size ) )
			{
				const float wheel = g_input->get_wheel_value( );
				if ( wheel != 0.f )
				{
					m_scroll_target -= wheel * 28.f;
					m_scroll_target = ImClamp( m_scroll_target, 0.f, m_max_scroll );
				}
			}
			else if ( m_max_scroll <= 0.f )
			{
				m_scroll_target = 0.f;
				m_scroll_offset = 0.f;
			}

			m_scroll_offset += ( m_scroll_target - m_scroll_offset ) * ImClamp( ( g_style ? g_style->scroll_speed : 18.f ) * ImGui::GetIO( ).DeltaTime, 0.f, 1.f );
			m_scroll_offset = ImClamp( m_scroll_offset, 0.f, m_max_scroll );

			for ( const std::shared_ptr<c_base_element>& control : m_controls )
			{
				if ( control && control->is_visible( ) )
					control->input( );
			}
		}

		void attach_child( std::string tab_name, std::string subtab_name, std::string side_name = {} )
		{
			m_child_attach_data.m_tab_name = std::move( tab_name );
			m_child_attach_data.m_subtab_name = std::move( subtab_name );
			m_child_attach_data.m_side_name = std::move( side_name );
		}

		const child_data_t& attach_data( ) const
		{
			return m_child_attach_data;
		}

		void set_visible( bool data )
		{
			m_visible = data;
		}

		void play_intro( )
		{
			int i = 0;
			for ( const std::shared_ptr<c_base_element>& control : m_controls )
			{
				if ( !control || !control->is_visible( ) )
					continue;
				control->play_intro( );
				control->set_intro_hold( 0.28f + static_cast<float>( i ) * 0.065f );
				++i;
			}
		}

		void close_overlays( )
		{
			for ( const std::shared_ptr<c_base_element>& control : m_controls )
			{
				if ( !control )
					continue;
				control->close_overlay( );
			}
		}

		bool visible( ) const
		{
			return m_visible;
		}

		child_width get_type( ) const
		{
			return m_width_type;
		}

		void remove_titlebar( )
		{
			m_titlebar = false;
		}

		void set_size( c_vector_2d size )
		{
			m_size = size;
		}

		float measured_height( ) const
		{
			const float row_h = g_style ? g_style->row_height : 37.f;
			return static_cast<float>( visible_count( ) ) * row_h;
		}

		void recompute_height( )
		{
			if ( m_size.y < 1.f )
				m_size.y = g_style ? g_style->row_height : 37.f;
		}

		void set_max_height( float h )
		{
			m_size.y = h;
		}

		int visible_count( ) const
		{
			int count = 0;
			for ( const std::shared_ptr<c_base_element>& control : m_controls )
			{
				if ( control && control->is_visible( ) )
					++count;
			}
			return count;
		}

		c_vector_2d calculate_element_padding( ) const
		{
			return c_vector_2d( 0.f, 0.f );
		}

		c_vector_2d calculate_safe_area( ) const
		{
			return m_pos;
		}

		std::shared_ptr<c_checkbox> add_checkbox( std::string label, bool* val )
		{
			std::shared_ptr<c_checkbox> el = std::make_shared<c_checkbox>( std::move( label ), val );
			el->set_parent( m_name );
			m_controls.push_back( el );
			return el;
		}

		template <typename T>
		std::shared_ptr<c_slider<T>> add_slider( std::string label, T* val, T min_v, T max_v, bool hide_label = false )
		{
			std::shared_ptr<c_slider<T>> el = std::make_shared<c_slider<T>>( std::move( label ), val, min_v, max_v, hide_label );
			el->set_parent( m_name );
			m_controls.push_back( el );
			return el;
		}

		std::shared_ptr<c_dropdown> add_dropdown( std::string label, int* val, std::vector<std::string> items, bool hide_label = false )
		{
			std::shared_ptr<c_dropdown> el = std::make_shared<c_dropdown>( std::move( label ), val, std::move( items ), hide_label );
			el->set_parent( m_name );
			m_controls.push_back( el );
			return el;
		}

		std::shared_ptr<c_multidropdown> add_multibox( std::string label, bool hide_label, std::function<void( c_multidropdown* ptr )> callback )
		{
			std::shared_ptr<c_multidropdown> el = std::make_shared<c_multidropdown>( std::move( label ) );
			if ( hide_label )
				el->hide_label( );
			el->set_parent( m_name );
			if ( callback )
				callback( el.get( ) );
			m_controls.push_back( el );
			return el;
		}

		std::shared_ptr<c_colorpicker> add_colorpicker( std::string label, c_color* val, bool hide_label = false, bool* enabled = nullptr )
		{
			std::shared_ptr<c_colorpicker> el = std::make_shared<c_colorpicker>( std::move( label ), val, hide_label, enabled );
			el->set_parent( m_name );
			m_controls.push_back( el );
			return el;
		}

		std::shared_ptr<c_colorpicker> add_colorpicker( std::string label, Color* val, bool hide_label = false, bool* enabled = nullptr )
		{
			return add_colorpicker( std::move( label ), popstar_glue::color( val ), hide_label, enabled );
		}

		std::shared_ptr<c_keybind> add_keybind( std::string label, key_var_t* val, bool hide_label = false )
		{
			std::shared_ptr<c_keybind> el = std::make_shared<c_keybind>( std::move( label ), val, hide_label );
			el->set_parent( m_name );
			m_controls.push_back( el );
			return el;
		}

		std::shared_ptr<c_keybind> add_keybind( std::string label, Keybind* val, bool hide_label = false )
		{
			return add_keybind( std::move( label ), popstar_glue::key( val ), hide_label );
		}

		std::shared_ptr<c_text_input> add_input_box( std::string label, std::string* val, bool hide_label = false )
		{
			std::shared_ptr<c_text_input> el = std::make_shared<c_text_input>( std::move( label ), val, hide_label );
			el->set_parent( m_name );
			m_controls.push_back( el );
			return el;
		}

		std::shared_ptr<c_listbox> add_listbox( std::string label, int* val, std::vector<std::string> items, float height, bool hide_label = false )
		{
			std::shared_ptr<c_listbox> el = std::make_shared<c_listbox>( std::move( label ), val, std::move( items ), height, hide_label );
			el->set_parent( m_name );
			m_controls.push_back( el );
			return el;
		}

		std::shared_ptr<c_popup> add_popup( std::string label, std::function<void( c_popup* ptr )> callback, bool hide_label = false )
		{
			std::shared_ptr<c_popup> el = std::make_shared<c_popup>( std::move( label ) );
			if ( hide_label )
				el->hide_label( );
			el->set_parent( m_name );
			if ( callback )
				callback( el.get( ) );
			m_controls.push_back( el );
			return el;
		}

		std::shared_ptr<c_options> add_options( std::string label, std::function<void( c_options* ptr )> callback )
		{
			std::shared_ptr<c_options> el = std::make_shared<c_options>( std::move( label ) );
			el->set_parent( m_name );
			if ( callback )
				callback( el.get( ) );
			m_controls.push_back( el );
			return el;
		}

		std::shared_ptr<c_button> add_button( std::string label, std::function<void( )> callback, std::string button_text = "Apply" )
		{
			std::shared_ptr<c_button> el = std::make_shared<c_button>( std::move( label ), std::move( callback ) );
			el->set_button_text( std::move( button_text ) );
			el->set_parent( m_name );
			m_controls.push_back( el );
			return el;
		}

	private:
		std::string m_name {};
		c_vector_2d m_pos {}, m_relative_pos {};
		c_vector_2d m_size {};
		child_width m_width_type {};
		child_data_t m_child_attach_data {};
		bool m_visible { true };
		bool m_titlebar { true };
		std::vector<std::shared_ptr<c_base_element>> m_controls {};
		float m_scroll_offset = 0.f;
		float m_scroll_target = 0.f;
		float m_max_scroll = 0.f;
		float m_content_height = 0.f;
		std::vector<float> m_row_hover {};
	};

	inline c_checkbox& c_checkbox::attach_popup( std::string title, std::function<void( c_popup* )> build )
	{
		m_popup = std::make_shared<c_popup>( std::move( title ) );
		m_popup->hide_label( );
		m_popup->set_companion( true );
		if ( build )
			build( m_popup.get( ) );
		return *this;
	}

	inline c_checkbox& c_checkbox::attach_binds( )
	{
		bind_slot_t slot {};
		slot.label = m_label;
		slot.kind = bind_kind::boolean;
		slot.ptr = m_value;
		m_binds = std::make_shared<c_bind_popup>( std::move( slot ) );
		return *this;
	}

	inline bool c_checkbox::hosts( c_base_element* element ) const
	{
		if ( m_popup )
		{
			if ( m_popup.get( ) == element || m_popup->hosts( element ) )
				return true;
		}
		if ( m_binds )
		{
			if ( m_binds.get( ) == element || m_binds->hosts( element ) )
				return true;
		}
		return false;
	}

	inline void c_checkbox::play_intro( )
	{
		m_reveal = 0.f;
		m_anim = 0.f;
		if ( m_popup )
			m_popup->play_intro( );
	}

	inline void c_checkbox::close_overlay( )
	{
		if ( m_popup )
			m_popup->close_overlay( );
		if ( m_binds )
			m_binds->close_overlay( );
	}

	inline void c_checkbox::draw( )
	{
		if ( !is_visible( ) || !g_render || !g_style )
			return;

		const float row_h = g_style->row_height;
		m_size.y = row_h;
		const float width = m_parent_width > 0.f ? m_parent_width : m_size.x;
		m_size.x = width;

		const float dt = ImGui::GetIO( ).DeltaTime;
		if ( tick_intro( dt ) )
		{
			const float target = ( m_value && *m_value ) ? 1.f : 0.f;
			m_anim += ( target - m_anim ) * ( 1.f - std::exp( -9.5f * dt ) );
			if ( std::fabs( m_anim - target ) < 0.0005f )
				m_anim = target;
		}

		if ( !m_hide_label )
		{
			const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, m_label.c_str( ), g_style->text_body );
			g_render->text( c_fonts::k_default_key, c_vector_2d( m_pos.x + g_style->label_pad, m_pos.y + std::floor( ( row_h - ts.y ) * 0.5f ) ), g_style->text, m_label.c_str( ), g_style->text_body );
		}

		const float tw = g_style->toggle_w;
		const float th = g_style->toggle_h;
		const c_vector_2d toggle( g_style->toggle_x( m_pos.x, width ), m_pos.y + std::floor( ( row_h - th ) * 0.5f ) );

		float dots_x = std::floor( toggle.x - 8.f - c_popup::dots_width( ) );
		if ( m_popup )
		{
			m_popup->m_pos = m_pos;
			m_popup->m_parent_width = width;
			m_popup->set_dots_anchor( c_vector_2d( dots_x, std::floor( m_pos.y + ( row_h - c_popup::dots_height( ) ) * 0.5f ) ) );
			m_popup->draw( );
			dots_x -= ( c_bind_popup::dots_width( ) + 4.f );
		}
		if ( m_binds )
		{
			m_binds->m_pos = m_pos;
			m_binds->m_parent_width = width;
			m_binds->set_dots_anchor( c_vector_2d( dots_x, std::floor( m_pos.y + ( row_h - c_bind_popup::dots_height( ) ) * 0.5f ) ) );
			m_binds->draw( );
		}

		g_render->rect_filled( static_cast<int>( toggle.x ), static_cast<int>( toggle.y ), static_cast<int>( tw ), static_cast<int>( th ), g_style->toggle_off.lerp( g_style->accent, m_anim ), 8.f );
		g_render->circle_filled( c_vector_2d( toggle.x + ImLerp( 9.f, 20.f, m_anim ), toggle.y + th * 0.5f ), 7.f, g_style->knob_off.lerp( g_style->knob_on, m_anim ) );
	}

	inline void c_checkbox::input( )
	{
		if ( !is_visible( ) || !m_value || !g_input || !g_ctx || !g_style )
			return;

		const float width = m_parent_width > 0.f ? m_parent_width : m_size.x;
		const c_vector_2d toggle( g_style->toggle_x( m_pos.x, width ), m_pos.y + std::floor( ( g_style->row_height - g_style->toggle_h ) * 0.5f ) );
		float dots_x = std::floor( toggle.x - 8.f - c_popup::dots_width( ) );

		if ( m_popup )
		{
			m_popup->m_pos = m_pos;
			m_popup->m_parent_width = width;
			m_popup->set_dots_anchor( c_vector_2d( dots_x, std::floor( m_pos.y + ( g_style->row_height - c_popup::dots_height( ) ) * 0.5f ) ) );
			m_popup->input( );
			dots_x -= ( c_bind_popup::dots_width( ) + 4.f );
		}
		if ( m_binds )
		{
			m_binds->m_pos = m_pos;
			m_binds->m_parent_width = width;
			m_binds->set_dots_anchor( c_vector_2d( dots_x, std::floor( m_pos.y + ( g_style->row_height - c_bind_popup::dots_height( ) ) * 0.5f ) ) );
			m_binds->input( );
		}

		if ( !g_ctx->can_interact( this, m_focus_priority ) )
			return;

		if ( g_input->mouse_in_region( c_vector_2d( toggle.x - 5.f, toggle.y - 6.f ), c_vector_2d( 39.f, 30.f ) ) && g_input->clicked( mouse_buttons::left ) )
			*m_value = !*m_value;
	}
}
