#include <dependencies/includes.h>
#include "esp.cuh"
#include <src/utility/global/global.cuh>
#include <src/cheat/helper/helper.cuh>
#include "helpers.cuh"
#include <cctype>
#include <string>

static constexpr float k_bar_width = 2.f;
static constexpr float k_bar_gap = 3.f;

static auto size( ) -> ImVec2
{
	return ImVec2( esp::store.box_max.x - esp::store.box_min.x , esp::store.box_max.y - esp::store.box_min.y );
}

auto esp::tick( ) -> void
{
	if ( !global::esp::draw ) return;
	std::lock_guard<std::mutex> lock( cache->m_mutex );

	store.draw = ImGui::GetBackgroundDrawList( );
	const int flags = store.draw->Flags;
	store.draw->Flags &= ~( ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedLinesUseTex | ImDrawListFlags_AntiAliasedFill );

	const auto col = helper::colors::ConvertFloatImVec4( global::esp::color );
	const auto hp_col = helper::colors::ConvertFloatImVec4( global::esp::health_color );
	const auto sh_col = helper::colors::ConvertFloatImVec4( global::esp::shield_color );

	for ( auto& player : cache->m_players )
	{
		if ( !player.m_address ) continue;
		if ( player.m_health <= 0 ) continue;
		if ( player.m_origin.is_zero( ) ) continue;
		if ( !esp::bounds( player ) ) continue;

		const auto visible = !global::esp::visible_check || player.m_visible;
		store.alpha = visible ? 1.f : global::esp::occluded_alpha;
		store.col = helper::with_alpha( col , store.alpha );
		store.hp_col = helper::with_alpha( hp_col , store.alpha );
		store.sh_col = helper::with_alpha( sh_col , store.alpha );

		if ( global::esp::box )
		{
			if ( global::esp::box_type == 1 )
				esp::corner_box( store.draw , store.box_min , store.box_max , store.col , global::esp::box_thickness , global::esp::box_outline );
			else
				esp::box( store.draw , store.box_min , store.box_max , store.col , global::esp::box_thickness , global::esp::box_outline , false , IM_COL32_BLACK );
		}

		if ( global::esp::health_bar )
		{
	
			const float hp = static_cast<float>( player.m_health );
			const float max_hp = static_cast<float>( player.m_max_health );
			const float shown = helper::health.smooth( player.m_address , hp );
	
			helper::health.track( player.m_address , hp );
	
			const ImU32 top = IM_COL32(
				( int )( ( ( store.hp_col >> IM_COL32_R_SHIFT ) & 0xFF ) * 0.35f ) ,
				( int )( ( ( store.hp_col >> IM_COL32_G_SHIFT ) & 0xFF ) * 0.35f ) ,
				( int )( ( ( store.hp_col >> IM_COL32_B_SHIFT ) & 0xFF ) * 0.35f ) ,
				( store.hp_col >> IM_COL32_A_SHIFT ) & 0xFF );
	
			esp::health_bar( store.draw , store.box_min , store.box_max , shown , max_hp ,
				global::esp::box_thickness , global::esp::box_outline , k_bar_width , 0 , k_bar_gap , 1 , store.hp_col , top );
	
			const auto bar = helper::health.bar_rect( store.box_min , store.box_max , global::esp::box_thickness , global::esp::box_outline , k_bar_width , k_bar_gap );
			helper::health.draw( store.draw , esp::flags_font , player.m_address , bar.m_min , bar.m_max , shown , max_hp , store.alpha );
		}

		if ( global::esp::shield && player.m_max_shield > 0 )
		{
	
			esp::health_bar( store.draw , store.box_min , store.box_max ,
				static_cast<float>( player.m_shield ) , static_cast<float>( player.m_max_shield ) ,
				global::esp::box_thickness , global::esp::box_outline , k_bar_width , 3 , k_bar_gap , 0 , store.sh_col , store.sh_col );
		}

		if ( global::esp::name && !player.m_name.empty( ) )
		{
	
			esp::label( store.draw , player.m_name , store.col , store.box_min , size( ) , 1.f , helper::sides_t::TOP , 4.f , esp::name_font );
		}

		{
			auto* font = esp::distance_font;
			const float fs = ( font && font->LegacySize > 0.f ) ? font->LegacySize : 13.f;
	
			float y = ( global::esp::shield && player.m_max_shield > 0 ) ? k_bar_width + k_bar_gap + 1.f : 0.f;
			y += 2.f;
	
			if ( global::esp::distance )
			{
				char buf[ 32 ];
				std::snprintf( buf , sizeof( buf ) , "%.0fm" , player.m_origin.distance( cache->m_local.m_origin ) / 39.37f );
				esp::label( store.draw , buf , store.col , store.box_min , size( ) , y , helper::sides_t::BOTTOM , 0.f , font );
	
				float ascent = fs;
				if ( font )
					if ( auto* baked = font->GetFontBaked( fs ) ) ascent = baked->Ascent;
				y += ascent + 1.f;
			}
	
			if ( !global::esp::weapon || player.m_weapon.empty( ) ) return;
	
			std::string wpn = player.m_weapon;
			for ( char& c : wpn )
				c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
			esp::label( store.draw , wpn , store.col , store.box_min , size( ) , y , helper::sides_t::BOTTOM , 0.f , font );
		}

		if ( global::esp::flags )
		{
	
			auto* font = esp::flags_font;
			float font_size = font ? font->LegacySize : 10.f;
			if ( font_size <= 0.f ) font_size = 10.f;
	
			auto col = store.col;
			if ( player.m_knocked )
				col = helper::with_alpha( IM_COL32( 255 , 80 , 80 , 255 ) , store.alpha );
	
			float side_y = std::round( store.box_min.y - 4.f );
			helper::side_text( store.draw , font , font_size ,
				std::round( store.box_max.x + 5.f ) , side_y ,
				helper::move_flag( player ) , col );
		}

		if ( global::esp::head_dot && store.ok[ 0 ] )
		{
	
			store.draw->Flags |= ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedLinesUseTex | ImDrawListFlags_AntiAliasedFill;
	
			ImVec2 hd( store.screen[ 0 ].x , store.screen[ 0 ].y );
			float r = size( ).x * 0.22f;
	
			if ( store.ok[ 1 ] )
			{
				const float dx = store.screen[ 0 ].x - store.screen[ 1 ].x;
				const float dy = store.screen[ 0 ].y - store.screen[ 1 ].y;
				const float hn = sqrtf( dx * dx + dy * dy );
				if ( hn > 1.f )
				{
					hd.x += dx * 0.55f;
					hd.y += dy * 0.55f;
					r = hn * 0.65f;
				}
			}
			else
			{
				hd.y -= r * 0.7f;
			}
	
			r = ( std::max )( 3.f , ( std::min )( r , 48.f ) ) * global::esp::head_dot_scale;
			r = ( std::max )( 2.f , ( std::min )( r , 64.f ) );
	
			store.draw->AddCircle( hd , r , IM_COL32( 0 , 0 , 0 , 255 ) , 0 , 3.0f );
			store.draw->AddCircle( hd , r , store.col , 0 , 1.5f );
	
			store.draw->Flags &= ~( ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedLinesUseTex | ImDrawListFlags_AntiAliasedFill );
		}

		if ( global::esp::skeleton )
		{
	
			store.draw->Flags |= ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedFill;
			store.draw->Flags &= ~ImDrawListFlags_AntiAliasedLinesUseTex;
	
			if ( store.ok[ 11 ] && store.ok[ 12 ] && store.ok[ 14 ] && store.ok[ 15 ] )
			{
				const float ll = data::dist2( store.world[ 11 ] , store.world[ 12 ] );
				const float lr = data::dist2( store.world[ 11 ] , store.world[ 15 ] );
				if ( lr < ll )
				{
					std::swap( store.screen[ 12 ] , store.screen[ 15 ] );
					std::swap( store.world[ 12 ] , store.world[ 15 ] );
				}
			}
	
			auto link = [ ]( int a , int c )
			{
				if ( store.ok[ a ] && store.ok[ c ] )
					helper::bone_line( store.draw , store.screen[ a ] , store.screen[ c ] , store.col , global::esp::skeleton_thickness );
			};
	
			link( 0 , 1 );
			link( 1 , 2 );
			link( 2 , 3 );
			link( 1 , 4 ); link( 4 , 5 ); link( 5 , 6 );
			link( 1 , 7 ); link( 7 , 8 ); link( 8 , 9 );
			link( 3 , 10 ); link( 10 , 11 ); link( 11 , 12 );
			link( 3 , 13 ); link( 13 , 14 ); link( 14 , 15 );
	
			store.draw->Flags &= ~( ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedLinesUseTex | ImDrawListFlags_AntiAliasedFill );
		}
	}

	store.draw->Flags = flags;
}
