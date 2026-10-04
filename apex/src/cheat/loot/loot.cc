#include <dependencies/includes.h>
#include "loot.cuh"
#include <src/cheat/esp/helpers.cuh>
#include <src/cheat/loot/items.cuh>
#include <src/cheat/helper/helper.cuh>
#include <cctype>

auto loot::tick( ) -> void
{
	if ( !global::loot::enabled )
		return;

	std::lock_guard<std::mutex> lock( cache->m_mutex );

	auto* draw = ImGui::GetBackgroundDrawList( );
	const float max_u = global::loot::max_distance * 39.37f;
	const float max_u2 = max_u * max_u;

	for ( const auto& item : cache->m_loot )
	{
		if ( !item.address )
			continue;
		if ( !loot::allowed( item.category ) )
			continue;
		if ( static_cast<int>( item.rarity ) < global::loot::min_rarity )
			continue;

		const float dx = item.x - cache->m_local.m_origin.x;
		const float dy = item.y - cache->m_local.m_origin.y;
		const float dz = item.z - cache->m_local.m_origin.z;
		const float d2 = dx * dx + dy * dy + dz * dz;
		if ( d2 > max_u2 )
			continue;

		const auto screen = data::world_to_screen( data::_vector3( item.x , item.y , item.z ) );
		if ( !data::on_screen( screen ) )
			continue;

		const auto col = loot::color( item.rarity );
		const float meters = sqrtf( d2 ) / 39.37f;

		char name_buf[ 64 ] { };
		const std::size_t name_len = item.name.size( );
		const std::size_t n = name_len < sizeof( name_buf ) - 1 ? name_len : sizeof( name_buf ) - 1;
		for ( std::size_t i = 0; i < n; ++i )
			name_buf[ i ] = static_cast<char>( std::tolower( static_cast<unsigned char>( item.name[ i ] ) ) );
		name_buf[ n ] = '\0';

		char label[ 96 ];
		if ( global::loot::name && global::loot::distance )
			std::snprintf( label , sizeof( label ) , "%s (%.0fm)" , name_buf , meters );
		else if ( global::loot::distance )
			std::snprintf( label , sizeof( label ) , "(%.0fm)" , meters );
		else if ( global::loot::name )
			std::snprintf( label , sizeof( label ) , "%s" , name_buf );
		else
			continue;

		const ImVec2 pos( screen.x , screen.y );
		const ImVec2 size( 2.f , 2.f );

		esp::label(
			draw , label , col ,
			ImVec2( pos.x - 1.f , pos.y ) , size ,
			0.f , helper::sides_t::BOTTOM , 0.f ,
			esp::flags_font );
	}
}
