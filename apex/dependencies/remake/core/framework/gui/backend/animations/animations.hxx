#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

#include <imgui.h>
#include <core/framework/gui/backend/math/math.hxx>

namespace core::gui
{
	enum class easing_type : int
	{
		linear = 0,
		ease_in,
		ease_out,
		ease_in_out,
		ease_in_cubic,
		ease_out_cubic,
		ease_in_out_cubic,
		ease_in_quad,
		ease_out_quad,
		ease_in_out_quad,
		ease_in_quart,
		ease_out_quart,
		ease_in_out_quart,
		ease_in_expo,
		ease_out_expo,
		ease_in_out_expo,
		ease_in_back,
		ease_out_back,
		ease_in_out_back,
		bounce,
		elastic,
		spring
	};

	inline float apply_easing( easing_type type, float t )
	{
		t = ImClamp( t, 0.f, 1.f );
		constexpr float c1 = 1.70158f;
		constexpr float c2 = c1 * 1.525f;
		constexpr float c3 = c1 + 1.f;

		switch ( type )
		{
		case easing_type::linear:
			return t;
		case easing_type::ease_in:
		case easing_type::ease_in_quad:
			return t * t;
		case easing_type::ease_out:
		case easing_type::ease_out_quad:
			return 1.f - ( 1.f - t ) * ( 1.f - t );
		case easing_type::ease_in_out:
		case easing_type::ease_in_out_quad:
			return t < 0.5f ? 2.f * t * t : 1.f - std::pow( -2.f * t + 2.f, 2.f ) / 2.f;
		case easing_type::ease_in_cubic:
			return t * t * t;
		case easing_type::ease_out_cubic:
			return 1.f - std::pow( 1.f - t, 3.f );
		case easing_type::ease_in_out_cubic:
			return t < 0.5f ? 4.f * t * t * t : 1.f - std::pow( -2.f * t + 2.f, 3.f ) / 2.f;
		case easing_type::ease_in_quart:
			return t * t * t * t;
		case easing_type::ease_out_quart:
			return 1.f - std::pow( 1.f - t, 4.f );
		case easing_type::ease_in_out_quart:
			return t < 0.5f ? 8.f * t * t * t * t : 1.f - std::pow( -2.f * t + 2.f, 4.f ) / 2.f;
		case easing_type::ease_in_expo:
			return t <= 0.f ? 0.f : std::pow( 2.f, 10.f * t - 10.f );
		case easing_type::ease_out_expo:
			return t >= 1.f ? 1.f : 1.f - std::pow( 2.f, -10.f * t );
		case easing_type::ease_in_out_expo:
			if ( t <= 0.f ) return 0.f;
			if ( t >= 1.f ) return 1.f;
			return t < 0.5f
				? std::pow( 2.f, 20.f * t - 10.f ) / 2.f
				: ( 2.f - std::pow( 2.f, -20.f * t + 10.f ) ) / 2.f;
		case easing_type::ease_in_back:
			return c3 * t * t * t - c1 * t * t;
		case easing_type::ease_out_back:
			return 1.f + c3 * std::pow( t - 1.f, 3.f ) + c1 * std::pow( t - 1.f, 2.f );
		case easing_type::ease_in_out_back:
			return t < 0.5f
				? ( std::pow( 2.f * t, 2.f ) * ( ( c2 + 1.f ) * 2.f * t - c2 ) ) / 2.f
				: ( std::pow( 2.f * t - 2.f, 2.f ) * ( ( c2 + 1.f ) * ( t * 2.f - 2.f ) + c2 ) + 2.f ) / 2.f;
		case easing_type::bounce:
		{
			const float n1 = 7.5625f;
			const float d1 = 2.75f;
			if ( t < 1.f / d1 ) return n1 * t * t;
			if ( t < 2.f / d1 ) { t -= 1.5f / d1; return n1 * t * t + 0.75f; }
			if ( t < 2.5f / d1 ) { t -= 2.25f / d1; return n1 * t * t + 0.9375f; }
			t -= 2.625f / d1;
			return n1 * t * t + 0.984375f;
		}
		case easing_type::elastic:
		{
			if ( t <= 0.f ) return 0.f;
			if ( t >= 1.f ) return 1.f;
			return std::pow( 2.f, -10.f * t ) * std::sin( ( t * 10.f - 0.75f ) * ( 2.f * 3.14159265f / 3.f ) ) + 1.f;
		}
		case easing_type::spring:
		{
			const float damp = 0.5f;
			return 1.f - std::exp( -6.f * t ) * std::cos( 12.f * t * damp );
		}
		default:
			return t;
		}
	}

	struct anim_context_t
	{
		float current { 0.f };
		float target { 0.f };
		float speed { 12.f };

		void set( float value )
		{
			current = value;
			target = value;
		}

		void go( float value )
		{
			target = value;
		}

		void update( float dt )
		{
			if ( dt <= 0.f )
				dt = ImGui::GetIO( ).DeltaTime;
			current += ( target - current ) * ImClamp( speed * dt, 0.f, 1.f );
		}

		float value( ) const
		{
			return current;
		}

		bool finished( float epsilon = 0.001f ) const
		{
			return std::fabs( current - target ) <= epsilon;
		}
	};

	namespace detail
	{
		template <typename T>
		struct anim_traits;

		template <>
		struct anim_traits<float>
		{
			static float lerp( float a, float b, float t ) { return a + ( b - a ) * t; }
			static float distance( float a, float b ) { return std::fabs( b - a ); }
		};

		template <>
		struct anim_traits<int>
		{
			static int lerp( int a, int b, float t ) { return static_cast<int>( std::lround( a + ( b - a ) * t ) ); }
			static float distance( int a, int b ) { return static_cast<float>( std::abs( b - a ) ); }
		};

		template <>
		struct anim_traits<bool>
		{
			static bool lerp( bool a, bool b, float t ) { return t >= 0.5f ? b : a; }
			static float distance( bool a, bool b ) { return a == b ? 0.f : 1.f; }
		};

		template <>
		struct anim_traits<c_vector_2d>
		{
			static c_vector_2d lerp( const c_vector_2d& a, const c_vector_2d& b, float t )
			{
				return c_vector_2d( a.x + ( b.x - a.x ) * t, a.y + ( b.y - a.y ) * t );
			}
			static float distance( const c_vector_2d& a, const c_vector_2d& b )
			{
				return ( b - a ).length_2d( );
			}
		};

		template <>
		struct anim_traits<c_color>
		{
			static c_color lerp( const c_color& a, const c_color& b, float t )
			{
				return a.lerp( b, t );
			}
			static float distance( const c_color& a, const c_color& b )
			{
				return static_cast<float>( std::abs( a.r - b.r ) + std::abs( a.g - b.g ) + std::abs( a.b - b.b ) + std::abs( a.a - b.a ) );
			}
		};
	}

	template <typename T>
	class c_anim
	{
	public:
		c_anim( ) = default;

		c_anim& from( const T& value )
		{
			m_from = value;
			m_value = value;
			m_has_from = true;
			return *this;
		}

		c_anim& to( const T& value )
		{
			m_to = value;
			m_has_to = true;
			return *this;
		}

		c_anim& speed( float value )
		{
			m_speed = value;
			m_use_duration = false;
			return *this;
		}

		c_anim& duration( float seconds )
		{
			m_duration = ( std::max )( seconds, 0.0001f );
			m_use_duration = true;
			return *this;
		}

		c_anim& easing( easing_type type )
		{
			m_easing = type;
			return *this;
		}

		c_anim& ping_pong( bool enabled = true )
		{
			m_ping_pong = enabled;
			return *this;
		}

		c_anim& loop( bool enabled = true )
		{
			m_loop = enabled;
			return *this;
		}

		c_anim& delay( float seconds )
		{
			m_delay = seconds;
			m_delay_left = seconds;
			return *this;
		}

		c_anim& on_complete( std::function<void( )> callback )
		{
			m_on_complete = std::move( callback );
			return *this;
		}

		void reset( )
		{
			m_elapsed = 0.f;
			m_delay_left = m_delay;
			m_finished = false;
			m_forward = true;
			if ( m_has_from )
				m_value = m_from;
		}

		void update( float dt = -1.f )
		{
			if ( m_finished || !m_has_to )
				return;

			if ( dt < 0.f )
				dt = ImGui::GetIO( ).DeltaTime;

			if ( m_delay_left > 0.f )
			{
				m_delay_left -= dt;
				if ( m_delay_left > 0.f )
					return;
				dt += m_delay_left;
				m_delay_left = 0.f;
			}

			if ( !m_has_from )
			{
				m_from = m_value;
				m_has_from = true;
			}

			if ( m_use_duration )
			{
				m_elapsed += dt;
				float t = ImClamp( m_elapsed / m_duration, 0.f, 1.f );
				const float e = apply_easing( m_easing, m_forward ? t : 1.f - t );
				m_value = detail::anim_traits<T>::lerp( m_from, m_to, e );

				if ( t >= 1.f )
				{
					if ( m_ping_pong )
					{
						m_forward = !m_forward;
						m_elapsed = 0.f;
						if ( m_forward && !m_loop )
						{
							m_finished = true;
							if ( m_on_complete )
								m_on_complete( );
						}
					}
					else if ( m_loop )
					{
						m_elapsed = 0.f;
					}
					else
					{
						m_value = m_to;
						m_finished = true;
						if ( m_on_complete )
							m_on_complete( );
					}
				}
			}
			else
			{
				const float alpha = ImClamp( m_speed * dt, 0.f, 1.f );
				m_value = detail::anim_traits<T>::lerp( m_value, m_to, apply_easing( m_easing, alpha ) );
				if ( detail::anim_traits<T>::distance( m_value, m_to ) <= 0.001f )
				{
					m_value = m_to;
					if ( m_ping_pong )
					{
						const T old_to = m_to;
						m_to = m_from;
						m_from = old_to;
						m_finished = false;
					}
					else if ( m_loop )
					{
						m_value = m_from;
					}
					else
					{
						m_finished = true;
						if ( m_on_complete )
							m_on_complete( );
					}
				}
			}
		}

		const T& value( ) const
		{
			return m_value;
		}

		T& value( )
		{
			return m_value;
		}

		bool finished( ) const
		{
			return m_finished;
		}

	private:
		T m_value {};
		T m_from {};
		T m_to {};
		bool m_has_from { false };
		bool m_has_to { false };
		bool m_use_duration { false };
		bool m_ping_pong { false };
		bool m_loop { false };
		bool m_forward { true };
		bool m_finished { false };
		float m_speed { 12.f };
		float m_duration { 0.25f };
		float m_elapsed { 0.f };
		float m_delay { 0.f };
		float m_delay_left { 0.f };
		easing_type m_easing { easing_type::ease_out_cubic };
		std::function<void( )> m_on_complete {};
	};

	class c_animator
	{
	public:
		using tick_fn = std::function<void( float )>;

		std::uint64_t register_anim( tick_fn fn )
		{
			const std::uint64_t id = ++m_next_id;
			m_entries.push_back( { id, std::move( fn ), true } );
			return id;
		}

		void unregister_anim( std::uint64_t id )
		{
			m_entries.erase(
				std::remove_if( m_entries.begin( ), m_entries.end( ),
					[id]( const entry_t& e ) { return e.id == id; } ),
				m_entries.end( ) );
		}

		void update( float dt = -1.f )
		{
			if ( dt < 0.f )
				dt = ImGui::GetIO( ).DeltaTime;

			for ( entry_t& entry : m_entries )
			{
				if ( entry.active && entry.fn )
					entry.fn( dt );
			}
		}

		void clear( )
		{
			m_entries.clear( );
		}

	private:
		struct entry_t
		{
			std::uint64_t id {};
			tick_fn fn {};
			bool active { true };
		};

		std::vector<entry_t> m_entries {};
		std::uint64_t m_next_id { 0 };
	};

	inline std::shared_ptr<c_animator> g_animator = std::make_shared<c_animator>( );

	namespace animations
	{
		inline anim_context_t m_window_opacity {};
		inline anim_context_t m_tab_switching {};

		inline anim_context_t m_checkbox_opacity {};
		inline anim_context_t m_checkbox_value {};
		inline anim_context_t m_checkbox_hover {};
		inline anim_context_t m_checkbox_deactivated {};

		inline anim_context_t m_slider_opacity {};
		inline anim_context_t m_slider_value {};
		inline anim_context_t m_slider_hover {};
		inline anim_context_t m_slider_deactivated {};

		inline anim_context_t m_dropdown_opacity {};
		inline anim_context_t m_dropdown_value {};
		inline anim_context_t m_dropdown_open {};
		inline anim_context_t m_dropdown_switch {};
		inline anim_context_t m_dropdown_hover {};
		inline anim_context_t m_dropdown_deactivated {};

		inline anim_context_t m_button_opacity {};
		inline anim_context_t m_button_value {};
		inline anim_context_t m_button_hover {};

		inline anim_context_t m_textinput_opacity {};
		inline anim_context_t m_textinput_value {};
		inline anim_context_t m_textinput_hover {};

		inline anim_context_t m_colorpicker_opacity {};
		inline anim_context_t m_colorpicker_value {};
		inline anim_context_t m_colorpicker_hover {};

		inline anim_context_t m_popup_opacity {};
		inline anim_context_t m_popup_value {};
		inline anim_context_t m_popup_hover {};

		inline anim_context_t m_keybind_opacity {};
		inline anim_context_t m_keybind_value {};
		inline anim_context_t m_keybind_hover {};
		inline anim_context_t m_keybind_selected_mode {};
		inline anim_context_t m_keybind_binding {};

		inline anim_context_t m_listbox_opacity {};
		inline anim_context_t m_listbox_value {};
		inline anim_context_t m_listbox_hover {};
		inline anim_context_t m_listbox_item_hover {};
		inline anim_context_t m_listbox_scroll {};

		inline void tick_presets( float dt = -1.f )
		{
			if ( dt < 0.f )
				dt = ImGui::GetIO( ).DeltaTime;

			m_window_opacity.update( dt );
			m_tab_switching.update( dt );
			m_checkbox_opacity.update( dt );
			m_checkbox_value.update( dt );
			m_checkbox_hover.update( dt );
			m_checkbox_deactivated.update( dt );
			m_slider_opacity.update( dt );
			m_slider_value.update( dt );
			m_slider_hover.update( dt );
			m_slider_deactivated.update( dt );
			m_dropdown_opacity.update( dt );
			m_dropdown_value.update( dt );
			m_dropdown_open.update( dt );
			m_dropdown_switch.update( dt );
			m_dropdown_hover.update( dt );
			m_dropdown_deactivated.update( dt );
			m_button_opacity.update( dt );
			m_button_value.update( dt );
			m_button_hover.update( dt );
			m_textinput_opacity.update( dt );
			m_textinput_value.update( dt );
			m_textinput_hover.update( dt );
			m_colorpicker_opacity.update( dt );
			m_colorpicker_value.update( dt );
			m_colorpicker_hover.update( dt );
			m_popup_opacity.update( dt );
			m_popup_value.update( dt );
			m_popup_hover.update( dt );
			m_keybind_opacity.update( dt );
			m_keybind_value.update( dt );
			m_keybind_hover.update( dt );
			m_keybind_selected_mode.update( dt );
			m_keybind_binding.update( dt );
			m_listbox_opacity.update( dt );
			m_listbox_value.update( dt );
			m_listbox_hover.update( dt );
			m_listbox_item_hover.update( dt );
			m_listbox_scroll.update( dt );
		}
	}
}
