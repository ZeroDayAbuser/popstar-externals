#pragma once

#include <cmath>
#include <functional>
#include <string>

#include <core/framework/gui/backend/math/math.hxx>
#include <core/framework/gui/backend/render/render.hxx>
#include <core/framework/gui/frontend/widgets/classes/context.hxx>

namespace core::gui
{
	enum class element_type : int
	{
		checkbox,
		slider,
		dropdown,
		button,
		colorpicker,
		keybind,
		text_input,
		listbox,
		popup,
		options,
		tab,
		child,
		window
	};

	class c_base_element
	{
	public:
		c_vector_2d m_pos {}, m_size {};
		std::string m_label {}, m_parent {};

		c_base_element* m_parent_control { nullptr };

		bool m_visible { true };
		bool m_inlined { false };
		bool m_callback_visibility { false };
		bool* m_visible_by_callback { nullptr };

		focus_priority m_focus_priority { focus_priority::interactive };

		float m_parent_width { 0.f };
		float m_child_size { 0.f };

		bool m_hide_label { false };

		render_layer m_layer { render_layer::normal };

		element_type m_type {};

		virtual void draw( ) = 0;
		virtual void input( ) = 0;
		virtual void play_intro( )
		{
			m_reveal = 0.f;
		}
		virtual void close_overlay( )
		{
		}
		virtual bool hosts( c_base_element* /*element*/ ) const { return false; }
		virtual ~c_base_element( ) = default;

		void set_intro_hold( float delay )
		{
			m_intro_hold = delay;
		}

		bool tick_intro( float dt )
		{
			if ( m_intro_hold <= 0.f )
				return true;
			m_intro_hold -= dt;
			return false;
		}

		void advance_reveal( float dt )
		{
			if ( !tick_intro( dt ) )
				return;
			m_reveal += ( 1.f - m_reveal ) * ( 1.f - std::exp( -3.6f * dt ) );
			if ( std::fabs( 1.f - m_reveal ) < 0.001f )
				m_reveal = 1.f;
		}

		float reveal_t( ) const
		{
			return m_reveal;
		}

		float m_intro_hold { 0.f };
		float m_reveal { 1.f };

		void set_parent( std::string parent )
		{
			m_parent = std::move( parent );
		}

		void set_visibility( bool visible )
		{
			m_visible = visible;
		}

		void set_layer( render_layer layer )
		{
			m_layer = layer;
		}

		void hide_label( )
		{
			m_hide_label = true;
		}

		void set_callback_visibility( bool* val )
		{
			m_callback_visibility = true;
			m_visible_by_callback = val;
		}

		void set_inlined( )
		{
			m_inlined = true;
		}

		bool is_visible( ) const
		{
			if ( m_callback_visibility && m_visible_by_callback )
				return m_visible && *m_visible_by_callback;
			return m_visible;
		}
	};
}
