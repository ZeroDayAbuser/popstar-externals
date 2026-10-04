#pragma once
#include <dependencies/includes.h>

auto classes::c_entity::get( int ix ) -> classes::c_entity
{
	auto list = hypervisor->read<classes::c_entity>( hypervisor->m_base_address + offsets::entity_list + ( ix * 0x20 ) );
	return classes::c_entity( list );
}

auto classes::c_entity::localplayer( ) -> classes::c_entity
{
	return hypervisor->read<classes::c_entity>( hypervisor->m_base_address + offsets::local_player );
}

auto classes::c_entity::team( ) -> int
{
	return hypervisor->read<int>( this->m_address + offsets::m_i_team_num );
}

auto classes::c_entity::pos( ) -> data::_vector2
{
	return hypervisor->read<data::_vector2>( this->m_address + offsets::m_vec_abs_origin );
}

auto classes::c_entity::bone_pos( data::e_bone_type bone_id ) -> data::_vector3
{
	auto model = hypervisor->read<std::uint64_t>( this->m_address + offsets::studio_hdr );
	if ( !model ) return { };

	auto studio_hdr = hypervisor->read<std::uint64_t>( model + 0x8 );
	if ( !studio_hdr ) return { };

	auto hitbox_cache = hypervisor->read<std::uint16_t>( studio_hdr + 0x34 );
	auto hitbox_array = studio_hdr + ( ( std::uint16_t )( hitbox_cache & 0xFFFE ) << ( 4 * ( hitbox_cache & 1 ) ) );

	auto index_cache = hypervisor->read<std::uint16_t>( hitbox_array + 0x4 );
	auto hitbox_index = ( ( std::uint16_t )( index_cache & 0xFFFE ) << ( 4 * ( index_cache & 1 ) ) );

	const auto hb = static_cast<std::uint64_t>( bone_id );

	auto bone = hypervisor->read<std::int32_t>( hitbox_array + hitbox_index + ( hb * 0x20 ) );
	if ( bone < 0 || bone > 255 )
		bone = hypervisor->read<std::uint16_t>( hitbox_array + hitbox_index + ( hb * 0x20 ) );
	if ( bone < 0 || bone > 255 ) return { };

	auto bone_array = hypervisor->read<std::uintptr_t>( this->m_address + offsets::off_bone_array );
	if ( !bone_array ) return { };

	auto origin = hypervisor->read<data::_vector3>( this->m_address + offsets::m_vec_abs_origin );

	const auto base = bone_array + static_cast<std::uint64_t>( bone ) * 0x30;
	float bx = hypervisor->read<float>( base + 0x0C );
	float by = hypervisor->read<float>( base + 0x1C );
	float bz = hypervisor->read<float>( base + 0x2C );

	if ( ( bx * bx + by * by + bz * bz ) < 1.f )
	{
		bx = hypervisor->read<float>( base + 0xCC );
		by = hypervisor->read<float>( base + 0xDC );
		bz = hypervisor->read<float>( base + 0xEC );
	}

	return { bx + origin.x , by + origin.y , bz + origin.z };
}

auto classes::c_entity::player_names( ) -> std::string
{
	if ( !this->m_address ) return "";

	auto read_cstr = [ & ]( std::uint64_t ptr ) -> std::string
	{
		if ( !ptr || !hypervisor->is_valid( ptr ) )
			return "";

		auto buf = hypervisor->read<std::array<char , 64>>( ptr );
		buf.back( ) = '\0';
		if ( !buf[ 0 ] )
			return "";

		for ( int i = 0; i < static_cast<int>( buf.size( ) ) - 1 && buf[ i ]; i++ )
		{
			const auto c = static_cast<unsigned char>( buf[ i ] );
			if ( c < 32 || c == 127 )
			{
				buf[ i ] = '\0';
				break;
			}
		}

		return buf[ 0 ] ? std::string( buf.data( ) ) : "";
	};

	const auto sig = read_cstr( hypervisor->read<std::uint64_t>( this->m_address + offsets::m_i_signifier_name ) );
	if ( !sig.empty( ) )
	{
		auto lower = sig;
		for ( auto& c : lower )
			c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );

		if ( lower.find( "dummie" ) != std::string::npos )
			return "dummy";
	}

	const auto index = hypervisor->read<int>( this->m_address + offsets::name_index );
	if ( index >= 1 )
	{
		const auto list = hypervisor->m_base_address + offsets::name_list;

		if ( auto s = read_cstr( hypervisor->read<std::uint64_t>( list + ( static_cast<std::uint64_t>( index - 1 ) * 0x18 ) ) ); !s.empty( ) )
			return s;

		if ( auto s = read_cstr( hypervisor->read<std::uint64_t>( list + ( static_cast<std::uint64_t>( index ) * 0x10 ) ) ); !s.empty( ) )
			return s;

		if ( auto s = read_cstr( hypervisor->read<std::uint64_t>( list + ( static_cast<std::uint64_t>( index - 1 ) * 0x10 ) ) ); !s.empty( ) )
			return s;
	}

	if ( !sig.empty( ) && sig != "player" )
		return sig;

	return "";
}

auto classes::c_entity::camera_origin( ) const -> data::_vector3
{
	if ( !this->m_address ) return { };
	return hypervisor->read<data::_vector3>( this->m_address + offsets::off_camera_origin );
}

auto classes::c_entity::view_angles( ) const -> data::_vector3
{
	if ( !this->m_address ) return { };
	return hypervisor->read<data::_vector3>( this->m_address + offsets::off_view_angles );
}

auto classes::c_entity::control_rotation( ) const -> data::_vector3
{
	if ( !this->m_address ) return { };
	return hypervisor->read<data::_vector3>( this->m_address + offsets::m_local_angles );
}

auto classes::c_entity::active_weapon( ) -> std::uint64_t
{
	if ( !this->m_address ) return 0;

	auto handle = hypervisor->read<std::uint64_t>( this->m_address + offsets::m_latest_primary_weapons ) & 0xffff;
	if ( !handle ) return 0;

	return hypervisor->read<std::uint64_t>( hypervisor->m_base_address + offsets::entity_list + ( handle * 0x20 ) );
}

auto classes::c_entity::projectile_info( ) -> data::weapon_info_t
{
	auto weapon = this->active_weapon( );
	if ( !weapon ) return { };

	return hypervisor->read<data::weapon_info_t>( weapon + offsets::weapon_settings_meta_base + offsets::projectile_launch_speed );
}

namespace
{
	auto read_cl_fov_scale( ) -> float
	{
		const auto cvar = hypervisor->m_base_address + offsets::cl_fov_scale;
		static constexpr std::int32_t k_value_offs[] = { 0x68, 0x60, 0x64, 0x6C, 0x70 };

		auto try_obj = [ ]( std::uint64_t obj ) -> float
		{
			if ( !obj )
				return 0.f;
			for ( const auto off : k_value_offs )
			{
				const auto v = hypervisor->read<float>( obj + off );
				if ( v > 0.74f && v < 1.72f )
					return v;
			}
			return 0.f;
		};

		if ( const auto direct = try_obj( cvar ) )
			return direct;

		const auto ptr = hypervisor->read<std::uint64_t>( cvar );
		if ( const auto via_ptr = try_obj( ptr ) )
			return via_ptr;

		return 1.f;
	}

	auto read_weapon_zoom_fov( std::uint64_t weapon ) -> float
	{
		if ( !weapon )
			return 0.f;

		const auto data = weapon + offsets::m_player_data;
		const auto cur = hypervisor->read<float>( data + offsets::m_cur_zoom_fov );
		if ( cur > 1.f && cur < 170.f )
			return cur;

		const auto target = hypervisor->read<float>( data + offsets::m_target_zoom_fov );
		if ( target > 1.f && target < 170.f )
			return target;

		return 0.f;
	}
}

auto classes::c_entity::zoom_fov( ) -> float
{
	auto weapon = this->active_weapon( );
	if ( !weapon )
		return 0.f;

	if ( const auto live = read_weapon_zoom_fov( weapon ) )
		return live;

	return hypervisor->read<float>( weapon + offsets::weapon_settings_meta_base + offsets::zoom_fov );
}

auto classes::c_entity::view_fov( ) -> float
{
	const float scale = read_cl_fov_scale( );
	const float hip = 70.f * scale;

	const auto live = read_weapon_zoom_fov( this->active_weapon( ) );
	if ( live > 1.f && live < 170.f )
	{
		// ADS / lerp: live drops under the hip camera FOV
		if ( live < 69.5f || live + 0.5f < hip )
			return live;
	}

	return hip;
}

auto classes::c_entity::is_zooming( ) -> bool
{
	if ( !this->m_address )
		return false;

	const float hip = 70.f * read_cl_fov_scale( );
	const auto live = read_weapon_zoom_fov( this->active_weapon( ) );
	return live > 1.f && live + 0.5f < hip;
}

auto classes::c_entity::last_visible_time( ) -> float
{
	if ( !this->m_address ) return 0.f;
	return hypervisor->read<float>( this->m_address + offsets::last_visible_time );
}

auto classes::c_entity::is_visible( float time_base ) -> bool
{
	if ( !this->m_address )
		return false;

	const float last = this->last_visible_time( );
	if ( last <= 0.f || time_base <= 0.f )
		return false;

	const float delta = time_base - last;
	return delta >= -0.1f && delta <= 0.25f;
}
