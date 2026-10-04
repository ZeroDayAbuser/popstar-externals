#pragma once


#pragma once

#include <algorithm>
#include "../sdk/sdk.h"
#include "../settings/settings.h"
#include "../memory/offsets.h"
#include <thread>
#include <iostream>
#include <mutex>
#include <cmath>
#include <array>
#include <unordered_map>
#include <TlHelp32.h>
#include "../memory/decryption/decryption.h"
#include "../actor/exploits/chams/chams.h"
#include "../../../../dependencies/oxorany/oxorany.h"

bool g_debug = false;

struct Players
{
	uintptr_t player_state;
	uintptr_t pawn_private;
	uintptr_t mesh;
	bool isteamate;
	uintptr_t rank;
	int level;
	int rarity;
	int ammo;

	int kills;
	std::vector<Vector3> skeleton_3d_positions;
	uintptr_t actorRootComponent;
	Vector3 actorRelativeLocation;
	uintptr_t actor;
	uintptr_t dead;
	uintptr_t dbno;

};

std::vector<Players> buffer1;
std::vector<Players> buffer2;
std::atomic<std::vector<Players>*> active_buffer ( &buffer1 );
std::atomic<bool> player_cache_running { true };
std::vector<Players> player;


enum efortrarity
{
	common = 0 ,
	uncommon = 1 ,
	rare = 2 ,
	epic = 3 ,
	legendary = 4 ,
	mythic = 5 ,
	trancendent = 6 ,
	unattanable = 7 ,
	dwq = 8 ,
	qdwq = 9
};

struct CachedPlayerData {
	uintptr_t pawn_private = 0;
	Vector3 head_3d;
	Vector3 bottom_3d;
	Vector3 aim_neck_3d;
	bool is_partner;
	int kills;
	int level;
	int rarity;
	int ammo;
	Vector3 aim_chest_3d;
	Vector3 aim_pelvis_3d;
	Vector3 aim_feet_3d;
	Vector2 head_2d;
	Vector2 head_projected;
	Vector2 bottom_2d;
	std::vector<Vector2> skeleton_screen_positions;
	std::array<ImVec2 , 34> skeleton_line_points {};
	int skeleton_line_point_count = 0;
	bool is_visible;
	std::vector<Vector3> skeleton_3d_positions;
	std::string username;
	std::string platform_text;
	std::string weapon_name;
	Vector3 velocity;
	float distance;
	bool in_screen;
	uintptr_t actor;
	int rank;
	bool dbno;
	uintptr_t mesh;
	player_bounds m_bounds {};
	uint32_t player_id = 0;
	Vector3 location_3d;

	int total_kills = 0;
	int total_wins = 0;
	double kd = 0.0;
	double winrate = 0.0;

};

std::vector<CachedPlayerData> render_cache_buf1;
std::vector<CachedPlayerData> render_cache_buf2;
std::atomic<std::vector<CachedPlayerData>*> active_render_cache ( &render_cache_buf1 );
std::atomic<bool> cache_reading_thread_running { true };

struct _keybinds {
	void update ( )
	{
		for ( ;;)
		{
			if ( g_aimbot::controller_support ) {
				CustomWidgets::UpdateControllerKeybindState ( &g_aimbot::aimbot_key );
				CustomWidgets::UpdateControllerKeybindState ( &g_aimbot::secondary_aimbot_key );
			}
			else {
				CustomWidgets::UpdateKeybindState ( &g_aimbot::aimbot_key );
				CustomWidgets::UpdateKeybindState ( &g_aimbot::secondary_aimbot_key );
			}
			std::this_thread::sleep_for ( std::chrono::milliseconds ( 1 ) );
		}
	}
};

std::unique_ptr<_keybinds> g_keybinds = std::make_unique<_keybinds> ( );


struct _gworld {

	void update_camera ( ) {

		while ( true ) {

			try {

				ViewPoint::update_camera ( );

			}

			catch ( ... )
			{
			}

			std::this_thread::sleep_for ( std::chrono::milliseconds ( 1 ) );
		}

	}

	void update_engine ( ) {
		// Debug flag - you can set this as a global variable or pass as parameter
		static bool debug = false; // Set to false to disable debug output
		// Or make it a global: extern bool g_debug;

		try {
			while ( true ) {
				uintptr_t encrypted_uworld = bypass::read<uintptr_t> ( bypass::target::m_base_address + Offsets::Uworld );
				uintptr_t decrypted_uworld = g_decrypt->decrypt ( );

				if ( debug ) {
					printf ( "[DEBUG] encrypted_uworld: 0x%llX\n" , encrypted_uworld );
					printf ( "[DEBUG] decrypted_uworld: 0x%llX\n" , decrypted_uworld );
				}

				if ( bypass::target::is_valid ( decrypted_uworld ) )
					g_pointers->uworld = decrypted_uworld;

				if ( debug && bypass::target::is_valid ( decrypted_uworld ) ) {
					printf ( "[DEBUG] Setting uworld to: 0x%llX\n" , g_pointers->uworld );
				}

				if ( bypass::target::is_valid ( g_pointers->uworld ) ) {
					g_pointers->game_instance = bypass::read<uintptr_t> ( g_pointers->uworld + Offsets::GameInstance );
					if ( debug ) printf ( "[DEBUG] game_instance: 0x%llX\n" , g_pointers->game_instance );
				}

				if ( bypass::target::is_valid ( g_pointers->game_instance ) ) {
					g_pointers->local_players = bypass::read<uintptr_t> ( bypass::read<uintptr_t> ( g_pointers->game_instance + Offsets::LocalPlayers ) );
					if ( debug ) printf ( "[DEBUG] local_players: 0x%llX\n" , g_pointers->local_players );
				}

				if ( bypass::target::is_valid ( g_pointers->local_players ) ) {
					g_pointers->player_controller = bypass::read<uintptr_t> ( g_pointers->local_players + Offsets::PlayerController );
					if ( debug ) printf ( "[DEBUG] player_controller: 0x%llX\n" , g_pointers->player_controller );
				}

				if ( bypass::target::is_valid ( g_pointers->player_controller ) ) {
					g_pointers->local_pawn = bypass::read<uintptr_t> ( g_pointers->player_controller + Offsets::AcknowledgedPawn );
					if ( debug ) printf ( "[DEBUG] local_pawn: 0x%llX\n" , g_pointers->local_pawn );
				}

				if ( bypass::target::is_valid ( g_pointers->local_pawn ) ) {
					g_pointers->root_component = bypass::read<uintptr_t> ( g_pointers->local_pawn + Offsets::RootComponent );
					g_pointers->player_state = bypass::read<uintptr_t> ( g_pointers->local_pawn + Offsets::PlayerState );
					g_pointers->current_weapon = bypass::read<uintptr_t> ( g_pointers->local_pawn + Offsets::CurrentWeapon );

					if ( debug ) {
						printf ( "[DEBUG] root_component: 0x%llX\n" , g_pointers->root_component );
						printf ( "[DEBUG] player_state: 0x%llX\n" , g_pointers->player_state );
						printf ( "[DEBUG] current_weapon: 0x%llX\n" , g_pointers->current_weapon );
					}
				}

				if ( bypass::target::is_valid ( g_pointers->root_component ) ) {
					g_pointers->relative_location = bypass::read<Vector3> ( g_pointers->root_component + Offsets::RelativeLocation );
					if ( debug ) {
						printf ( "[DEBUG] relative_location: (%.2f, %.2f, %.2f)\n" ,
							g_pointers->relative_location.x ,
							g_pointers->relative_location.y ,
							g_pointers->relative_location.z );
					}
				}

				if ( bypass::target::is_valid ( g_pointers->player_state ) ) {
					g_pointers->my_team_id = bypass::read<int> ( g_pointers->player_state + Offsets::TeamIndex );
					if ( debug ) printf ( "[DEBUG] my_team_id: %d\n" , g_pointers->my_team_id );
				}

				if ( bypass::target::is_valid ( g_pointers->uworld ) ) {
					g_pointers->game_state = bypass::read<uintptr_t> ( g_pointers->uworld + Offsets::GameState );
					if ( debug ) printf ( "[DEBUG] game_state: 0x%llX\n" , g_pointers->game_state );
				}

				if ( bypass::target::is_valid ( g_pointers->game_state ) ) {
					g_pointers->player_array = bypass::read<uintptr_t> ( g_pointers->game_state + Offsets::PlayerArray );
					g_pointers->player_count = bypass::read<int> ( g_pointers->game_state + Offsets::PlayerArray + sizeof ( uintptr_t ) );
					g_pointers->server_time = bypass::read<float> ( g_pointers->game_state + 0x2e8 );

					if ( debug ) {
						printf ( "[DEBUG] player_array: 0x%llX\n" , g_pointers->player_array );
						printf ( "[DEBUG] player_count: %d\n" , g_pointers->player_count );
						printf ( "[DEBUG] server_time: %.2f\n" , g_pointers->server_time );
					}
				}

				if ( bypass::target::is_valid ( g_pointers->player_controller ) ) {
					g_pointers->targeted_fort_pawn = bypass::read<uintptr_t> ( g_pointers->player_controller + Offsets::TargetedFortPawn );
					if ( debug ) printf ( "[DEBUG] targeted_fort_pawn: 0x%llX\n" , g_pointers->targeted_fort_pawn );
				}

				if ( !ViewPoint::view_state ) {
					ViewPoint::setup_camera ( );
					if ( debug ) printf ( "[DEBUG] Camera setup completed\n" );
				}


				std::this_thread::sleep_for ( std::chrono::milliseconds ( 100 ) );
			}
		}
		catch ( ... ) {
			if ( debug ) printf ( "[DEBUG] Exception caught in update_engine\n" );
		}
	}

	void update_actors ( ) {

		while ( true ) {
			try {
				std::vector<Players>* write_buffer = ( active_buffer.load ( ) == &buffer1 ) ? &buffer2 : &buffer1;
				write_buffer->clear ( );

				int count = g_pointers->player_count;

				if ( count <= 0 || count > 200 || !bypass::target::is_valid ( g_pointers->player_array ) ) {
					active_buffer.store ( write_buffer );
					std::this_thread::sleep_for ( std::chrono::milliseconds ( 50 ) );
					continue;
				}

				write_buffer->reserve ( count );
				uintptr_t local_pawn = g_pointers->local_pawn;
				uintptr_t local_player_state = g_pointers->player_state;
				uintptr_t local_mesh = 0;
				if ( bypass::target::is_valid ( local_pawn ) ) {
					local_mesh = bypass::read<uintptr_t> ( local_pawn + Offsets::Mesh );
				}

				for ( int i = 0; i < count; i++ ) {
					uintptr_t player_state = bypass::read<uintptr_t> ( g_pointers->player_array + i * sizeof ( uintptr_t ) );
					if ( !player_state ) continue;
					if ( local_player_state && player_state == local_player_state ) continue;

					uintptr_t pawn_private = bypass::read<uintptr_t> ( player_state + Offsets::PawnPrivate );
					if ( !pawn_private || ( local_pawn && pawn_private == local_pawn ) ) continue;

					uintptr_t mesh = bypass::read<uintptr_t> ( pawn_private + Offsets::Mesh );
					if ( !mesh ) continue;
					if ( local_mesh && mesh == local_mesh ) continue;

					char dying_flag = bypass::read<char> ( pawn_private + Offsets::bIsDying );
					if ( ( dying_flag >> 5 ) & 1 ) continue;

					uintptr_t root = bypass::read<uintptr_t> ( pawn_private + Offsets::RootComponent );
					if ( !root ) continue;

					int team_id = bypass::read<int> ( player_state + Offsets::TeamIndex );
					char downed_flag = bypass::read<char> ( pawn_private + Offsets::bIsDBNO );

					int rank = 0;
					uintptr_t hab = bypass::read<uintptr_t> ( player_state + Offsets::HabaneroComponent );
					if ( hab ) rank = bypass::read<int> ( hab + Offsets::RankedProgress + 0x10 );

					int level = bypass::read<int> ( player_state + Offsets::SeasonLevelUIDisplay );
					int kills = bypass::read<int> ( player_state + Offsets::KillScore );
					int ammo = 0;
					int rarity = 0;

					uintptr_t current_weapon = bypass::read<uintptr_t> ( pawn_private + Offsets::CurrentWeapon );
					if ( current_weapon ) {
						ammo = bypass::read<int> ( current_weapon + Offsets::AmmoCount );
						uintptr_t weapon_data = bypass::read<uintptr_t> ( current_weapon + Offsets::WeaponData );
						if ( weapon_data ) {
							rarity = bypass::read<unsigned char> ( weapon_data + Offsets::Rarity );
						}
					}

					Players actor {};
					actor.actorRelativeLocation = bypass::read<Vector3> ( root + Offsets::RelativeLocation );
					actor.actorRootComponent = root;
					actor.mesh = mesh;
					actor.dbno = ( downed_flag >> 7 ) & 1;
					actor.rank = rank;
					actor.isteamate = ( count > 1 ) && ( team_id == g_pointers->my_team_id );
					actor.pawn_private = pawn_private;
					actor.player_state = player_state;
					actor.level = level;
					actor.kills = kills;
					actor.ammo = ammo;
					actor.rarity = rarity;
					actor.actor = pawn_private;
					write_buffer->push_back ( actor );
				}

				active_buffer.store ( write_buffer );
			}
			catch ( ... ) { }

			std::this_thread::sleep_for ( std::chrono::milliseconds ( 14 ) );
		}

	}

	void prepare_skeletal ( std::vector<CachedPlayerData>& cache ) {
		try {
			const bool need_bounds = ( g_settings::box || g_settings::rank || g_settings::platform || g_settings::player_name );
			const bool need_skeleton_lines = g_settings::skeleton;
			for ( auto& cached : cache ) {
				cached.head_2d = Custom::K2_Project ( Vector3 ( cached.head_3d.x , cached.head_3d.y , cached.head_3d.z + 20 ) );
				cached.head_projected = cached.head_2d;
				cached.bottom_2d = Custom::K2_Project ( cached.bottom_3d );
				cached.in_screen = (
					cached.head_2d.x >= 0 && cached.head_2d.x <= width_sdk &&
					cached.head_2d.y >= 0 && cached.head_2d.y <= height_sdk
					);

				cached.skeleton_screen_positions.clear ( );
				cached.skeleton_screen_positions.reserve ( cached.skeleton_3d_positions.size ( ) );
				for ( const auto& pos : cached.skeleton_3d_positions ) {
					cached.skeleton_screen_positions.push_back ( Custom::K2_Project ( pos ) );
				}

				cached.skeleton_line_point_count = 0;
				if ( need_skeleton_lines && cached.skeleton_screen_positions.size ( ) >= 20 ) {
					const auto& sp = cached.skeleton_screen_positions;
					const int segments [ ] [ 2 ] = {
						{ 17, 19 }, { 19, 18 }, { 18, 8 },
						{ 18, 2 }, { 2, 3 }, { 3, 4 },
						{ 18, 5 }, { 5, 6 }, { 6, 7 },
						{ 8, 9 }, { 9, 10 }, { 10, 11 }, { 11, 12 },
						{ 8, 13 }, { 13, 14 }, { 14, 15 }, { 15, 16 }
					};

					for ( const auto& seg : segments ) {
						if ( cached.skeleton_line_point_count + 1 >= static_cast<int> ( cached.skeleton_line_points.size ( ) ) )
							break;

						const Vector2& a = sp [ seg [ 0 ] ];
						const Vector2& b = sp [ seg [ 1 ] ];
						const bool a_valid =
							std::isfinite ( a.x ) && std::isfinite ( a.y ) &&
							a.x > 1.0 && a.x < ( static_cast<double> ( width_sdk ) - 1.0 ) &&
							a.y > 1.0 && a.y < ( static_cast<double> ( height_sdk ) - 1.0 );
						const bool b_valid =
							std::isfinite ( b.x ) && std::isfinite ( b.y ) &&
							b.x > 1.0 && b.x < ( static_cast<double> ( width_sdk ) - 1.0 ) &&
							b.y > 1.0 && b.y < ( static_cast<double> ( height_sdk ) - 1.0 );
						if ( !a_valid || !b_valid )
							continue;

						cached.skeleton_line_points [ cached.skeleton_line_point_count++ ] = ImVec2 ( static_cast<float> ( a.x ) , static_cast<float> ( a.y ) );
						cached.skeleton_line_points [ cached.skeleton_line_point_count++ ] = ImVec2 ( static_cast<float> ( b.x ) , static_cast<float> ( b.y ) );
					}
				}

				if ( need_bounds ) {
					double bmin_x = ( std::min ) ( static_cast< double >( cached.head_2d.x ) , static_cast< double >( cached.bottom_2d.x ) );
					double bmax_x = ( std::max ) ( static_cast< double >( cached.head_2d.x ) , static_cast< double >( cached.bottom_2d.x ) );
					double bmin_y = ( std::min ) ( static_cast< double >( cached.head_2d.y ) , static_cast< double >( cached.bottom_2d.y ) );
					double bmax_y = ( std::max ) ( static_cast< double >( cached.head_2d.y ) , static_cast< double >( cached.bottom_2d.y ) );

					for ( const auto& sp : cached.skeleton_screen_positions ) {
						if ( !std::isfinite ( sp.x ) || !std::isfinite ( sp.y ) )
							continue;
						if ( sp.x < bmin_x ) bmin_x = sp.x;
						if ( sp.x > bmax_x ) bmax_x = sp.x;
						if ( sp.y < bmin_y ) bmin_y = sp.y;
						if ( sp.y > bmax_y ) bmax_y = sp.y;
					}

					const double bw = bmax_x - bmin_x;
					const double bh = bmax_y - bmin_y;
					cached.m_bounds = {
						bmin_x - bw * 0.1 , bmax_x + bw * 0.1 ,
						bmin_y - bh * 0.05 , bmax_y + bh * 0.05
					};
				}
			}
		}
		catch ( ... ) { }
	}

	void cache_reading_work ( ) {
		static const int skeleton_bones [ ] = { 66, 66, 9, 10, 11, 38, 39, 40, 2, 71, 72, 75, 76, 78, 79, 82, 83, 110, 66, 67 };
		int slow_tick = 0;

		while ( cache_reading_thread_running.load ( ) ) {
			try {
				chams::update_enabled ( g_exploits::chams );
				auto players_buf = active_buffer.load ( );
				std::vector<CachedPlayerData>* read_cache = active_render_cache.load ( );
				std::vector<CachedPlayerData>* write_cache = ( read_cache == &render_cache_buf1 ) ? &render_cache_buf2 : &render_cache_buf1;
				write_cache->clear ( );
				write_cache->reserve ( players_buf->size ( ) );

				const bool do_slow_reads = ( slow_tick % 20 == 0 );
				const bool do_visibility_read = ( slow_tick % 2 == 0 );
				std::unordered_map<uintptr_t , const CachedPlayerData*> prev_map;
				for ( const auto& prev : *read_cache )
					prev_map [ prev.actor ] = &prev;

				for ( const auto& player : *players_buf ) {
					if ( !player.mesh || !player.player_state )
						continue;

					uintptr_t bone_array = bypass::read<uintptr_t> ( player.mesh + Offsets::Bone_Array );
					if ( !bone_array ) bone_array = bypass::read<uintptr_t> ( player.mesh + Offsets::Bone_Array + 0x10 );
					if ( !bone_array ) continue;

					FTransform c2w = bypass::read<FTransform> ( player.mesh + Offsets::Component_To_World );
					D3DMATRIX c2w_mat = c2w.to_matrix_with_scale ( );

					auto read_bone = [ & ] ( int idx ) -> Vector3 {
						auto bt = bypass::read<FTransform> ( bone_array + ( idx * 0x60 ) );
						D3DMATRIX m = matrix_multiplication ( bt.to_matrix_with_scale ( ) , c2w_mat );
						return Vector3 ( m._41 , m._42 , m._43 );
						};

					CachedPlayerData cached {};
					cached.pawn_private = player.pawn_private;
					cached.mesh = player.mesh;
					cached.actor = player.actor;
					cached.head_3d = read_bone ( 110 );
					cached.bottom_3d = read_bone ( 0 );
					cached.aim_neck_3d = cached.head_3d;
					cached.aim_chest_3d = cached.head_3d;
					cached.aim_pelvis_3d = cached.bottom_3d;
					cached.aim_feet_3d = cached.bottom_3d;

					if ( g_aimbot::enable ) {
						switch ( g_aimbot::hitbox_type ) {
						case 1: cached.aim_neck_3d = read_bone ( 67 ); break;
						case 2: cached.aim_chest_3d = read_bone ( 66 ); break;
						case 3: {
							Vector3 lf = read_bone ( 76 ), rf = read_bone ( 79 );
							cached.aim_feet_3d = Vector3 ( ( lf.x + rf.x ) * 0.5f , ( lf.y + rf.y ) * 0.5f , ( lf.z + rf.z ) * 0.5f );
							break;
						}
						case 4:
						case 5: {
							cached.aim_neck_3d = read_bone ( 67 );
							cached.aim_chest_3d = read_bone ( 66 );
							cached.aim_pelvis_3d = read_bone ( 2 );
							Vector3 lf = read_bone ( 76 ), rf = read_bone ( 79 );
							cached.aim_feet_3d = Vector3 ( ( lf.x + rf.x ) * 0.5f , ( lf.y + rf.y ) * 0.5f , ( lf.z + rf.z ) * 0.5f );
							break;
						}
						default: break;
						}
					}

					if ( g_settings::skeleton ) {
						cached.skeleton_3d_positions.reserve ( 20 );
						for ( int idx : skeleton_bones ) {
							cached.skeleton_3d_positions.push_back ( read_bone ( idx ) );
						}
					}

					cached.distance = static_cast< float >( ViewPoint::Location.Distance ( cached.bottom_3d ) / 100.0 );
					if ( do_visibility_read ) {
						cached.is_visible = is_visible ( g_pointers->uworld , player.mesh );
					}
					else {
						auto it_vis = prev_map.find ( player.actor );
						cached.is_visible = ( it_vis != prev_map.end ( ) ) ? it_vis->second->is_visible : false;
					}
					cached.location_3d = player.actorRelativeLocation;
					cached.rank = static_cast<int> ( player.rank );
					cached.dbno = player.dbno;
					cached.is_partner = player.isteamate;
					cached.kills = player.kills;
					cached.level = player.level;
					cached.rarity = player.rarity;
					cached.ammo = player.ammo;
					cached.velocity = bypass::read<Vector3> ( player.actorRootComponent + Offsets::ComponentVelocity );

					if ( g_exploits::chams ) {
						chams::set ( player.pawn_private );
					}

					cached.total_kills = player.kills;
					cached.total_wins = 0;
					cached.kd = static_cast<double>( cached.total_kills );
					cached.winrate = 0.0;

					if ( do_slow_reads ) {
						cached.username = get_user ( player.player_state , g_pointers->server_time );
						cached.platform_text = get_platform_name ( player.player_state );
						cached.weapon_name = Name ( player.pawn_private );
						cached.player_id = bypass::read<uint32_t> ( player.player_state + 0x2B4 );
					}
					else {
						auto it = prev_map.find ( player.actor );
						if ( it != prev_map.end ( ) ) {
							cached.username = it->second->username;
							cached.platform_text = it->second->platform_text;
							cached.weapon_name = it->second->weapon_name;
							cached.player_id = it->second->player_id;
							cached.total_kills = it->second->total_kills;
							cached.total_wins = it->second->total_wins;
							cached.kd = it->second->kd;
							cached.winrate = it->second->winrate;
						}
					}

					write_cache->push_back ( std::move ( cached ) );
				}

				prepare_skeletal ( *write_cache );

				active_render_cache.store ( write_cache );
				slow_tick++;
			}
			catch ( ... ) { }

			Sleep ( 2 );
		}
	}

	std::vector<CachedPlayerData>* get_render_cache ( ) {
		return active_render_cache.load ( );
	}


};

std::unique_ptr<_gworld> g_world = std::make_unique<_gworld> ( );



bool is_running ( )
{
	bool found = false;
	DWORD pid = 0;

	HANDLE snapshot = CreateToolhelp32Snapshot ( TH32CS_SNAPPROCESS , 0 );
	if ( snapshot != INVALID_HANDLE_VALUE )
	{
		PROCESSENTRY32 pe {};
		pe.dwSize = sizeof ( PROCESSENTRY32 );

		if ( Process32First ( snapshot , &pe ) )
		{
			do
			{
				if ( _wcsicmp ( pe.szExeFile , oxorany ( L"FortniteClient-Win64-Shipping.exe" ) ) == 0 )
				{
					pid = pe.th32ProcessID;
					found = true;
					break;
				}
			} while ( Process32Next ( snapshot , &pe ) );
		}
		CloseHandle ( snapshot );
	}

	return found;
}


void check_fortnite ( )
{
	while ( true )
	{
		if ( is_running ( ) )
		{
		}
		else
		{
			__fastfail ( 0x1 );
		}

		Sleep ( oxorany ( 10000 ) );
	}
}


void start_exit_handler ( ) {
	std::thread exit_handler_thread ( [ & ] ( ) { check_fortnite ( ); } );
	exit_handler_thread.detach ( );
}

void start_keybinds ( ) {
	std::thread keybinds_thread ( [ & ] ( ) { g_keybinds->update ( ); } );
	keybinds_thread.detach ( );
}

void start_camera ( ) {
	std::thread camera_thread ( [ & ] ( ) { g_world->update_camera ( ); } );
	camera_thread.detach ( );
}



void start_engine ( ) {
	std::thread entity_thread ( [ & ] ( ) { g_world->update_engine ( ); } );
	entity_thread.detach ( );
}

void start_actors ( ) {
	std::thread actors_thread ( [ & ] ( ) { g_world->update_actors ( ); } );
	actors_thread.detach ( );
}

void start_cache ( ) {
	std::thread cache_thread ( [ & ] ( ) { g_world->cache_reading_work ( ); } );
	cache_thread.detach ( );
}

