#pragma once

#include <dependencies/includes.h>
#include <src/sdk/classes/classes.cuh>
#include <unordered_map>
#include <cfloat>
#include <cmath>
#include <algorithm>

namespace helper
{
	struct sides_t
	{
		enum Enum
		{
			LEFT = 0 ,
			TOP ,
			RIGHT ,
			BOTTOM
		} side;

		sides_t( Enum s ) : side( s ) { }

		ImVec2 get_position( const ImVec2& box_pos , const ImVec2& box_size , const ImVec2& text_size , float gap = 3.0f , float offset = 0.0f ) const
		{
			switch ( side )
			{
			case LEFT:
				return ImVec2(
					std::round( box_pos.x - gap - text_size.x ) ,
					std::round( box_pos.y + ( box_size.y * 0.5f ) - ( text_size.y * 0.5f ) )
				);
			case RIGHT:
				return ImVec2(
					std::round( box_pos.x + box_size.x + gap ) ,
					std::round( box_pos.y + offset )
				);
			case TOP:
				return ImVec2(
					std::round( box_pos.x + ( box_size.x * 0.5f ) - ( text_size.x * 0.5f ) ) ,
					std::round( box_pos.y - text_size.y - gap )
				);
			case BOTTOM:
				return ImVec2(
					std::round( box_pos.x + ( box_size.x * 0.5f ) - ( text_size.x * 0.5f ) ) ,
					std::round( box_pos.y + box_size.y + gap + offset )
				);
			}
			return box_pos;
		}
	};

	inline std::vector<sides_t> element_sides = {
		sides_t( sides_t::LEFT ) ,
		sides_t( sides_t::TOP ) ,
		sides_t( sides_t::RIGHT ) ,
		sides_t( sides_t::BOTTOM ) ,
		sides_t( sides_t::LEFT )
	};

	namespace colors
	{
		inline ImU32 ConvertFloatImVec4( const float color[ 4 ] )
		{
			return ImGui::ColorConvertFloat4ToU32( ImVec4( color[ 0 ] , color[ 1 ] , color[ 2 ] , color[ 3 ] ) );
		}
	}

	struct bar_rect_t
	{
		ImVec2 m_min = { };
		ImVec2 m_max = { };
	};

	struct health_delta_t
	{
		float m_value = { 0.f };
		float m_time = { 0.f };
		float m_current_y = { 0.f };
		bool m_has_y = { false };
	};

	struct health_track_t
	{
		std::unordered_map<std::uint64_t , float> m_last;
		std::unordered_map<std::uint64_t , float> m_shown;
		std::unordered_map<std::uint64_t , health_delta_t> m_delta;

		auto smooth( std::uint64_t address , float target ) -> float
		{
			const float clamped = ( std::max )( 0.f , target );
			const float dt = ImGui::GetIO( ).DeltaTime;
			constexpr float response_speed = 22.f;

			auto it = m_shown.find( address );
			if ( it == m_shown.end( ) )
			{
				m_shown[ address ] = clamped;
				return clamped;
			}

			float& shown = it->second;
			const float t = 1.f - std::exp( -response_speed * dt );
			shown += ( clamped - shown ) * t;
			return shown;
		}

		auto track( std::uint64_t address , float current ) -> void
		{
			float previous = 0.f;
			bool has_previous = false;
			if ( auto it = m_last.find( address ); it != m_last.end( ) )
			{
				previous = it->second;
				has_previous = true;
			}

			if ( has_previous )
			{
				const float delta = current - previous;
				if ( std::fabs( delta ) >= 1.f )
				{
					auto it = m_delta.find( address );
					if ( it != m_delta.end( ) )
					{
						const bool same_sign =
							( it->second.m_value >= 0.f && delta >= 0.f ) ||
							( it->second.m_value < 0.f && delta < 0.f );

						if ( same_sign )
							it->second.m_value += delta;
						else
						{
							it->second.m_value = delta;
							it->second.m_time = 0.f;
							it->second.m_has_y = false;
						}

						constexpr float fade_in = 0.2f;
						if ( same_sign && it->second.m_time >= fade_in )
							it->second.m_time = fade_in;
					}
					else
					{
						m_delta[ address ] = { delta , 0.f , 0.f , false };
					}
				}
			}

			m_last[ address ] = current;
		}

		auto bar_rect( const ImVec2& box_min , const ImVec2& box_max , float box_thickness , bool box_outline , float bar_width , float gap ) -> bar_rect_t
		{
			const float outer = box_thickness - 1.f;
			const float adjust = box_outline ? 0.f : 1.f;

			bar_rect_t out { };
			out.m_min = ImVec2(
				std::round( box_min.x - outer - gap - bar_width ) - 1.f ,
				std::round( box_min.y - outer + adjust ) );
			out.m_max = ImVec2(
				std::round( box_min.x - outer - gap ) - 1.f ,
				std::round( box_max.y + outer - adjust ) );
			return out;
		}

		auto draw( ImDrawList* draw , ImFont* font , std::uint64_t address , const ImVec2& bar_min , const ImVec2& bar_max , float displayed , float max_health , float alpha_factor ) -> void
		{
			auto it = m_delta.find( address );
			if ( it == m_delta.end( ) )
				return;

			auto& evt = it->second;
			const float dt = ImGui::GetIO( ).DeltaTime;
			evt.m_time += dt;

			constexpr float lifetime = 2.f;
			constexpr float fade_in = 0.2f;
			constexpr float fade_out = 0.2f;

			if ( evt.m_time >= lifetime )
			{
				m_delta.erase( it );
				return;
			}

			float alpha = 1.f;
			if ( evt.m_time < fade_in )
				alpha = evt.m_time / fade_in;
			else if ( evt.m_time > lifetime - fade_out )
				alpha = ( lifetime - evt.m_time ) / fade_out;

			const int delta_value = static_cast<int>( std::round( evt.m_value ) );
			if ( delta_value == 0 )
				return;

			char buf[ 16 ];
			if ( delta_value > 0 )
				std::snprintf( buf , sizeof( buf ) , "+%d" , delta_value );
			else
				std::snprintf( buf , sizeof( buf ) , "%d" , delta_value );

			if ( !font )
				font = ImGui::GetFont( );

			constexpr float font_size = 10.f;
			const ImVec2 text_size = font
				? font->CalcTextSizeA( font_size , FLT_MAX , 0.f , buf )
				: ImGui::CalcTextSize( buf );

			constexpr float pad_top = 1.f;
			constexpr float pad_bottom = 1.f;
			const float full_height = std::round( bar_max.y - bar_min.y ) + pad_top + pad_bottom;
			const float bar_top = std::round( bar_min.y - pad_top );
			const float bar_bottom = bar_top + full_height;

			const float clamped = std::clamp( displayed , 0.f , ( std::max )( max_health , 1.f ) );
			const float fill_ratio = ( max_health > 0.f ) ? ( clamped / max_health ) : 0.f;
			const float fill_top = std::round( bar_bottom - full_height * fill_ratio );

			float target_y = fill_top - text_size.y * 0.5f;
			target_y = std::clamp( target_y , bar_top , bar_bottom - text_size.y );

			if ( !evt.m_has_y )
			{
				evt.m_current_y = target_y;
				evt.m_has_y = true;
			}
			else
			{
				const float going_down = target_y > evt.m_current_y;
				const float speed = going_down ? 28.f : 36.f;
				const float t = 1.f - std::exp( -speed * dt );
				evt.m_current_y += ( target_y - evt.m_current_y ) * t;
			}

			constexpr float padding = 3.f;
			const ImVec2 text_pos(
				std::round( bar_min.x - padding - text_size.x ) ,
				std::round( evt.m_current_y ) );

			ImVec4 color = ( delta_value > 0 )
				? ImVec4( 0.35f , 1.f , 0.45f , 1.f )
				: ImVec4( 1.f , 0.3f , 0.3f , 1.f );
			color.w *= alpha_factor * alpha;
			const ImU32 text_color = ImGui::ColorConvertFloat4ToU32( color );
			const ImU32 outline_color = IM_COL32( 0 , 0 , 0 , ( int )( 255 * color.w ) );

			for ( float dx = -1.f; dx <= 1.f; ++dx )
			{
				for ( float dy = -1.f; dy <= 1.f; ++dy )
				{
					if ( dx == 0.f && dy == 0.f ) continue;
					draw->AddText( font , font_size , ImVec2( text_pos.x + dx , text_pos.y + dy ) , outline_color , buf );
				}
			}
			draw->AddText( font , font_size , text_pos , text_color , buf );
		}
	};

	inline health_track_t health;

	inline auto with_alpha( ImU32 col , float alpha ) -> ImU32
	{
		auto c = ImGui::ColorConvertU32ToFloat4( col );
		c.w *= alpha;
		return ImGui::ColorConvertFloat4ToU32( c );
	}

	inline auto bone_line( ImDrawList* draw , const data::_vector2& a , const data::_vector2& b , ImU32 col , float thickness = 1.f ) -> void
	{
		if ( !data::on_screen( a ) || !data::on_screen( b ) )
			return;

		const float dx = a.x - b.x;
		const float dy = a.y - b.y;
		const float d2 = dx * dx + dy * dy;
		if ( d2 < 2.f )
			return;

		const auto s = data::dimensions( );
		const float max_len = s.y * 0.65f;
		if ( d2 > max_len * max_len )
			return;

		// Core + outline both scale with the slider. Pad grows with thickness so the
		// black ring stays readable instead of staying stuck at a flat +2px.
		const float t = ( std::max )( 0.5f , thickness );
		const float outline = t + ( std::max )( 1.5f , t * 0.9f );
		const ImVec2 p0( a.x , a.y ) , p1( b.x , b.y );
		draw->AddLine( p0 , p1 , IM_COL32( 0 , 0 , 0 , ( col >> IM_COL32_A_SHIFT ) & 0xff ) , outline );
		draw->AddLine( p0 , p1 , col , t );
	}

	inline auto move_flag( const classes::c_entity& p ) -> const char*
	{
		constexpr int k_fl_onground = 1;
		const bool flag_ground = ( p.m_flags & k_fl_onground ) != 0;
		const bool ent_ground = ( p.m_ground_ent & 0xffff ) != 0;
		const bool grounded = flag_ground || ent_ground;

		const bool fall_sane =
			p.m_fall_velocity > -2000.f && p.m_fall_velocity < 2000.f;

		const bool planted =
			grounded ||
			!fall_sane ||
			( fabsf( p.m_fall_velocity ) < 8.f && !p.m_fast_falling );

		const float hz = p.m_vel.length_2d( );
		const bool moving_sig = p.m_is_dummy
			? ( p.m_origin_dh > 0.5f )
			: ( p.m_sliding || p.m_sticky_sprint || hz > 50.f || p.m_origin_dh > 0.45f );

		if ( p.m_knocked )
			return "knocked";
		if ( fall_sane && ( p.m_fast_falling || p.m_fall_velocity > 100.f ) )
			return "falling";
		if ( !planted && fall_sane && p.m_fall_velocity < -20.f )
			return "jumping";
		if ( !planted && !p.m_is_dummy && ( p.m_anim_jumping || p.m_has_jumped ) )
			return "jumping";
		if ( !planted && fall_sane && p.m_fall_velocity > 40.f )
			return "falling";
		if ( moving_sig )
			return "moving";
		return "idle";
	}

	inline auto side_text( ImDrawList* draw , ImFont* font , float font_size , float x , float& y , const char* text , ImU32 col ) -> void
	{
		if ( !text || !text[ 0 ] )
			return;

		const ImVec2 fp( x , y );
		const ImU32 outline = IM_COL32( 0 , 0 , 0 , ( col >> IM_COL32_A_SHIFT ) & 0xff );
		if ( font )
		{
			for ( float dx = -1.f; dx <= 1.f; dx++ )
				for ( float dy = -1.f; dy <= 1.f; dy++ )
					if ( dx != 0.f || dy != 0.f )
						draw->AddText( font , font_size , ImVec2( fp.x + dx , fp.y + dy ) , outline , text );
			draw->AddText( font , font_size , fp , col , text );
		}
		else
		{
			draw->AddText( fp , col , text );
		}

		y += font_size;
	}
}
