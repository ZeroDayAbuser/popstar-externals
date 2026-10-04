#include <dependencies/includes.h>
#include "aimbot.cuh"
#include <src/utility/global/global.cuh>

namespace
{
	struct store_t
	{
		classes::c_entity* enemy = nullptr;
		data::_vector3 aim { };
		data::_vector2 screen { };
	};

	store_t store { };
}

auto aimbot::get_enemy( ) -> classes::c_entity*
{
	classes::c_entity* closest = nullptr;
	auto min_distance = FLT_MAX;

	const auto center = math::screen_center( );

	for ( auto& entity : cache->m_players )
	{
		if ( !entity.m_address ) continue;
		if ( entity.m_health <= 0 ) continue;
		if ( entity.m_team == cache->m_local.m_team ) continue;
		if ( global::aimbot::visible_check && !entity.m_visible ) continue;

		auto screen_pos = data::world_to_screen( entity.m_origin );
		if ( screen_pos.is_zero( ) ) continue;

		auto dist = math::length_2d( screen_pos.x - center.x , screen_pos.y - center.y );

		if ( dist < min_distance && dist <= global::aimbot::fov )
		{
			min_distance = dist;
			closest = &entity;
		}
	}

	return closest;
}

auto aimbot::get_bone( classes::c_entity* enemy ) -> data::_vector3
{
	using b = data::e_bone_type;
	static constexpr b k_all[] = {
		b::head , b::neck , b::upper_chest , b::lower_chest , b::stomach , b::pelvis ,
		b::left_shoulder , b::left_elbow , b::left_hand ,
		b::right_shoulder , b::right_elbow , b::right_hand ,
		b::left_hip , b::left_knee , b::left_foot ,
		b::right_hip , b::right_knee , b::right_foot ,
	};
	static constexpr b k_single[] = { b::head , b::neck , b::upper_chest };

	if ( !enemy ) return { };

	const auto mode = global::aimbot::bone_mode;
	if ( mode != 3 )
		return enemy->bone_pos( k_single[ mode > 2 ? 0 : mode ] );

	const auto center = math::screen_center( );
	auto best = FLT_MAX;
	data::_vector3 aim { };

	for ( auto bone : k_all )
	{
		auto aim_pos = enemy->bone_pos( bone );
		if ( aim_pos.is_zero( ) ) continue;

		auto screen_pos = data::world_to_screen( aim_pos );
		if ( screen_pos.is_zero( ) ) continue;

		auto dist = math::length_2d( screen_pos.x - center.x , screen_pos.y - center.y );
		if ( dist < best )
		{
			best = dist;
			aim = aim_pos;
		}
	}

	return aim;
}

auto aimbot::view_angles( data::_vector3 world , data::_vector3 origin ) -> void
{
	auto want = math::calc_angle( origin , world );
	if ( !want.x && !want.y )
		return;

	const auto local = cache->m_local.m_address;
	const auto current = cache->m_local.view_angles( );
	const auto punch = hypervisor->read<data::_vector3>( local + offsets::m_vec_punch_base_angle ) + hypervisor->read<data::_vector3>( local + offsets::m_vec_punch_weapon_angle );

	const auto t = 1.f / ( std::max )( 1.f , global::aimbot::smoothness );

	data::_vector3 out {
		math::lerp_angle( current.x , want.x - punch.x , t ) ,
		math::lerp_angle( current.y , want.y - punch.y , t )
	};
	out.clamp( );

	hypervisor->write<data::_vector3>( local + offsets::off_view_angles , out );
}

auto aimbot::draw( ) -> void
{
	auto draw_bg = ImGui::GetBackgroundDrawList( );
	const int flags = draw_bg->Flags;
	draw_bg->Flags &= ~( ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedLinesUseTex | ImDrawListFlags_AntiAliasedFill );

	const auto col = ImGui::ColorConvertFloat4ToU32( ImVec4(
		global::esp::color[ 0 ] , global::esp::color[ 1 ] ,
		global::esp::color[ 2 ] , global::esp::color[ 3 ] ) );

	const auto& io = ImGui::GetIO( );
	const ImVec2 center( io.DisplaySize.x * 0.5f , io.DisplaySize.y * 0.5f );

	if ( global::aimbot::fov_circle )
	{
		draw_bg->AddCircle( center , global::aimbot::fov , IM_COL32( 0 , 0 , 0 , 255 ) , 48 , 3.0f );
		draw_bg->AddCircle( center , global::aimbot::fov , col , 48 , 1.0f );
	}

	if ( global::aimbot::enabled && global::aimbot::snapline && !store.screen.is_zero( ) )
	{
		const ImVec2 target( store.screen.x , store.screen.y );

		draw_bg->AddLine( center , target , IM_COL32( 0 , 0 , 0 , 255 ) , 3.0f );
		draw_bg->AddLine( center , target , col , 1.0f );
	}

	draw_bg->Flags = flags;
}

auto aimbot::tick( ) -> void
{
	while ( true )
	{
		const bool held = global::aimbot::enabled
			&& ( GetAsyncKeyState( global::aimbot::key ) & 0x8000 );

		if ( !held )
		{
			store.enemy = nullptr;
			store.screen = { };
			std::this_thread::sleep_for( std::chrono::milliseconds( global::aimbot::enabled ? 4 : 8 ) );
			continue;
		}

		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

		std::lock_guard<std::mutex> lock( cache->m_mutex );
		if ( !cache->m_local.m_address )
		{
			store.enemy = nullptr;
			store.screen = { };
			continue;
		}

		store.enemy = aimbot::get_enemy( );
		if ( !store.enemy )
		{
			store.screen = { };
			continue;
		}

		store.aim = aimbot::get_bone( store.enemy );
		if ( store.aim.is_zero( ) )
		{
			store.screen = { };
			continue;
		}

		auto cam = cache->m_local.camera_origin( );
		auto origin = cam.is_zero( ) ? cache->m_local.m_origin : cam;

		if ( global::aimbot::prediction )
		{
			auto proj = cache->m_local.projectile_info( );
			if ( proj.bullet_speed > 1.f )
				store.aim = data::bullet_trajectory( origin , store.aim , store.enemy->m_vel , proj.bullet_speed , proj.bullet_scale );
		}

		store.screen = data::world_to_screen( store.aim );

		aimbot::view_angles( store.aim , origin );
	}
}
