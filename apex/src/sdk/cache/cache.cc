#include "cache.cuh"
#include <src/utility/global/global.cuh>
#include <src/cheat/loot/items.cuh>
#include <unordered_map>

auto c_cache::update( ) -> bool
{
	std::vector<classes::c_entity> temp { };
	temp.reserve( 64 );
	std::vector<loot::entity_t> loot_temp { };
	if ( global::loot::enabled )
		loot_temp.reserve( 256 );

	static std::unordered_map<std::uint64_t , data::_vector3> prev_origin;

	auto local_addr = hypervisor->read<std::uint64_t>( hypervisor->m_base_address + offsets::local_player );
	if ( !local_addr ) return false;

	m_local.m_address = local_addr;
	m_local.m_health = hypervisor->read<int>( local_addr + offsets::m_i_health );
	if ( m_local.m_health <= 0 ) return false;

	m_local.m_max_health = hypervisor->read<int>( local_addr + offsets::m_i_max_health );
	m_local.m_team = hypervisor->read<std::int8_t>( local_addr + offsets::m_i_team_num );
	m_local.m_origin = hypervisor->read<data::_vector3>( local_addr + offsets::m_vec_abs_origin );

	auto time_base = hypervisor->read<float>( local_addr + offsets::time_base );
	const bool need_name = global::esp::name;
	const bool need_weapon = global::esp::weapon;
	const bool want_loot = global::loot::enabled;

	constexpr int k_max_ents = 10000;

	for ( int i = 0; i < k_max_ents; i++ )
	{
		auto addr = hypervisor->read<std::uint64_t>( hypervisor->m_base_address + offsets::entity_list + ( i * 0x20 ) );
		if ( !addr )              continue;
		if ( addr == local_addr ) continue;

		auto class_ptr = hypervisor->read<std::uint64_t>( addr + offsets::m_i_signifier_name );
		if ( !class_ptr || !hypervisor->is_valid( class_ptr ) ) continue;

		auto buffer = hypervisor->read<std::array<char , 32>>( class_ptr );
		buffer.back( ) = '\0';
		if ( !buffer[ 0 ] )
			continue;

		const bool is_dummy = std::strstr( buffer.data( ) , "npc_dummie" ) != nullptr;
		const bool is_player = !is_dummy && std::strstr( buffer.data( ) , "player" ) != nullptr;
		const bool is_loot = want_loot && std::strstr( buffer.data( ) , "prop_survival" ) != nullptr;
		const bool is_box = want_loot && (
			std::strstr( buffer.data( ) , "prop_death_box" ) != nullptr ||
			std::strstr( buffer.data( ) , "death_box" ) != nullptr );

		if ( is_loot || is_box )
		{
			auto origin = hypervisor->read<data::_vector3>( addr + offsets::m_vec_abs_origin );
			if ( origin.is_zero( ) )
				continue;

			loot::entity_t ent { };
			ent.address = addr;
			ent.x = origin.x;
			ent.y = origin.y;
			ent.z = origin.z;

			if ( is_box )
			{
				ent.name = "Death Box";
				ent.rarity = loot::rarity_t::epic;
				ent.category = loot::category_t::deathbox;
			}
			else
			{
				const int script_id = hypervisor->read<int>( addr + offsets::m_custom_script_int );
				const int wpn_idx = hypervisor->read<int>( addr + offsets::m_loot_weapon_name_index );
				const int model_idx = hypervisor->read<int>( addr + offsets::m_n_model_index );
				const int skin = hypervisor->read<int>( addr + offsets::m_n_skin );

				auto resolved = loot::resolve( script_id , wpn_idx , model_idx , skin );
				ent.name = std::move( resolved.name );
				ent.rarity = resolved.rarity;
				ent.category = resolved.category;
			}

			loot_temp.emplace_back( std::move( ent ) );
			continue;
		}

		if ( !is_dummy && !is_player )
			continue;

		classes::c_entity entity { };
		entity.m_address = addr;
		entity.m_health = hypervisor->read<int>( addr + offsets::m_i_health );
		if ( entity.m_health <= 0 ) continue;

		entity.m_team = hypervisor->read<std::int8_t>( addr + offsets::m_i_team_num );
		if ( entity.m_team == m_local.m_team ) continue;

		entity.m_max_health = hypervisor->read<int>( addr + offsets::m_i_max_health );
		entity.m_origin = hypervisor->read<data::_vector3>( addr + offsets::m_vec_abs_origin );

		auto abs_vel = hypervisor->read<data::_vector3>( addr + offsets::m_vec_abs_velocity );
		auto net_vel = hypervisor->read<data::_vector3>( addr + offsets::m_vec_velocity );
		entity.m_vel = ( abs_vel.length( ) > net_vel.length( ) ) ? abs_vel : net_vel;

		{
			auto track = hypervisor->read<data::_vector3>( addr + offsets::m_local_origin );
			if ( track.length( ) < 1.f )
				track = entity.m_origin;

			const auto it = prev_origin.find( addr );
			if ( it != prev_origin.end( ) )
				entity.m_origin_dh = ( track - it->second ).length_2d( );
			prev_origin[ addr ] = track;
		}

		entity.m_flags = hypervisor->read<int>( addr + offsets::m_f_flags );
		entity.m_ground_ent = hypervisor->read<std::uint32_t>( addr + offsets::m_h_ground_entity );
		{
			const int bleed = hypervisor->read<int>( addr + offsets::m_bleedout_state );
			entity.m_knocked = !is_dummy && bleed > 0 && bleed < 8;
		}

		const auto local = addr + offsets::m_Local;
		entity.m_fall_velocity = hypervisor->read<float>( local + offsets::local_fl_fall_velocity );
		entity.m_fast_falling = hypervisor->read<bool>( local + offsets::local_fast_falling );
		entity.m_anim_jumping = hypervisor->read<bool>( local + offsets::local_player_anim_jumping );
		entity.m_anim_landing = hypervisor->read<bool>( local + offsets::local_player_anim_landing );
		entity.m_anim_in_air_walk = hypervisor->read<bool>( local + offsets::local_player_anim_in_air_walk );

		entity.m_sprint_tilt = hypervisor->read<float>( addr + offsets::m_sprint_tilt_frac );
		entity.m_sticky_sprint = hypervisor->read<bool>( addr + offsets::m_b_is_sticky_sprinting );
		entity.m_sliding = hypervisor->read<bool>( addr + offsets::m_sliding );
		entity.m_has_jumped = hypervisor->read<bool>( addr + offsets::m_b_has_jumped_since_touched_ground );

		{
			const auto pose = hypervisor->read<std::array<float , 2>>( addr + offsets::m_fl_pose_parameters );
			entity.m_pose_move = sqrtf( pose[ 0 ] * pose[ 0 ] + pose[ 1 ] * pose[ 1 ] );
		}

		entity.m_shield = hypervisor->read<int>( addr + offsets::m_shield_health );
		entity.m_max_shield = hypervisor->read<int>( addr + offsets::m_shield_health_max );
		entity.m_visible = entity.is_visible( time_base );

		if ( need_name )
			entity.m_name = entity.player_names( );
		else if ( is_dummy )
			entity.m_name = "dummy";

		if ( need_weapon )
		{
			const auto wpn = entity.active_weapon( );
			entity.m_weapon = loot::held_weapon( wpn );
		}

		entity.m_is_dummy = is_dummy;

		temp.emplace_back( std::move( entity ) );
	}

	{
		std::lock_guard<std::mutex> lock( m_mutex );
		m_players = std::move( temp );
		m_loot = std::move( loot_temp );
	}

	return true;
}

auto c_cache::tick( ) -> void
{
	while ( true )
	{
		this->update( );
		std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
	}
}
