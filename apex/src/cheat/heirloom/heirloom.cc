#include <dependencies/includes.h>
#include "heirloom.cuh"
#include "heirloom_data.cuh"
#include <src/cheat/loot/items.cuh>
#include <src/utility/global/global.cuh>
#include <src/sdk/offsets/offsets.cuh>
#include <array>
#include <cstring>
#include <string>
#include <unordered_map>

namespace
{
	constexpr int k_inventory_slots = 10;
	constexpr int k_offhand_base = 0x38;
	constexpr int k_melee_offhand_slot = 5;
	constexpr int k_entity_stride = 0x20;
	constexpr std::uint32_t k_bad_entity = 0xCCCCCCCCCCCCCCCCULL;
	constexpr std::uint32_t k_bad_handle = 0xCCCCCCCCu;
	constexpr int k_debug_apply_interval = 50;
	constexpr int k_melee_src_offhand = -2;
	constexpr std::int32_t k_ef_nodraw = 0x20;

	struct string_table_t
	{
		bool valid = false;
		std::uint64_t items = 0;
		int count = 0;
		int stride = 0x48;
		int str_off = 0x10;
		int valid_off = 0x28;
	};

	auto probe_string_table( std::int32_t global_rva , const char* expected_name , string_table_t& out ) -> bool;

	string_table_t g_weapon_table { };
	string_table_t g_model_table { };
	std::unordered_map<std::string , int> g_model_by_exact;
	std::unordered_map<std::string , int> g_model_by_base;
	std::unordered_map<int , bool> g_model_is_view;
	std::uint64_t g_model_map_items = 0;
	std::uint64_t g_model_array_base = 0;
	int g_model_map_count = 0;
	bool g_logged_model_sample = false;

	int g_melee_slot = -1;
	bool g_catalog_resolved = false;
	int g_vm_player_offset = 0;
	int g_alias_source = -1;
	int g_alias_target = -1;
	std::uint64_t g_studio_wrapper = 0;
	std::uint64_t g_studio_saved = 0;
	float g_anim_start_time = 0.0f;
	float g_anim_start_frozen = 0.0f;
	bool g_anim_start_captured = false;
	std::uint16_t g_pinned_sequence = 0;
	float g_pinned_rate = 0.0f;
	bool g_sequence_pinned = false;
	std::uint16_t g_held_sequence = 0;
	float g_held_start = 0.0f;
	bool g_held_valid = false;
	std::uint16_t g_watch_sequence = 0xFFFF;
	float g_watch_start = 0.0f;
	float g_watch_rate = 0.0f;
	float g_watch_cycle = 0.0f;
	float g_watch_fidget = 0.0f;
	std::uint8_t g_watch_finished = 0xFF;
	bool g_watch_valid = false;
	std::uint64_t g_alias_saved_model = 0;
	std::uint64_t g_hidden_world = 0;
	std::int32_t g_hidden_world_index = -1;
	bool g_logged_offhand_melee = false;
	int g_table_fail_logs = 0;
	bool g_was_enabled = false;
	int g_apply_count = 0;
	int g_last_logged_selection = -1;
	bool g_inventory_dumped = false;
	std::vector<std::string> g_labels;

	auto debug_on( ) -> bool
	{
		return global::heirloom::debug;
	}

	template<typename... args_t>
	auto log( const char* fmt , args_t... args ) -> void
	{
		if ( debug_on( ) )
			logger->print( fmt , args... );
	}

	auto inventory_handle( std::uint64_t player , int slot ) -> std::uint32_t
	{
		return hypervisor->read<std::uint32_t>( player + offsets::m_inventory + slot * sizeof( std::uint32_t ) );
	}

	auto entity_from_handle( std::uint32_t handle ) -> std::uint64_t
	{
		if ( !handle || handle == 0xFFFFFFFFu )
			return 0;

		const auto index = handle & 0xFFFFu;
		const auto addr = hypervisor->m_base_address + offsets::entity_list + static_cast<std::uint64_t>( index ) * k_entity_stride;
		const auto ent = hypervisor->read<std::uint64_t>( addr );
		if ( !ent || ent == k_bad_entity )
			return 0;
		return ent;
	}

	auto weapon_entity( std::uint64_t player , int slot ) -> std::uint64_t
	{
		const auto handle = inventory_handle( player , slot );
		if ( !handle || handle == k_bad_handle )
			return 0;
		return entity_from_handle( handle );
	}

	auto offhand_melee_entity( std::uint64_t player ) -> std::uint64_t
	{
		const auto handle = hypervisor->read<std::uint32_t>(
			player + offsets::m_inventory + k_offhand_base + k_melee_offhand_slot * sizeof( std::uint32_t ) );
		if ( !handle || handle == k_bad_handle || handle == 0xFFFFFFFFu )
			return 0;
		return entity_from_handle( handle );
	}

	auto is_usable( std::uint64_t ptr ) -> bool
	{
		return ptr && ptr != k_bad_entity && hypervisor->is_valid( ptr );
	}

	auto view_model_from_offset( std::uint64_t player , int byte_off ) -> std::uint64_t
	{
		const auto handle = hypervisor->read<std::uint32_t>( player + byte_off );
		if ( !handle || handle == 0xFFFFFFFFu || handle == k_bad_handle )
			return 0;
		return entity_from_handle( handle );
	}

	auto view_model_entity( std::uint64_t player , int slot ) -> std::uint64_t
	{
		if ( g_vm_player_offset > 0 && slot == 0 )
			return view_model_from_offset( player , g_vm_player_offset );

		const auto handle = hypervisor->read<std::uint32_t>( player + offsets::m_h_view_models + slot * sizeof( std::uint32_t ) );
		if ( !handle || handle == 0xFFFFFFFFu || handle == k_bad_handle )
			return 0;
		return entity_from_handle( handle );
	}

	auto model_path( int index ) -> std::string;

	auto discover_view_model( std::uint64_t player , std::uint64_t melee_weapon ) -> std::uint64_t
	{
		if ( !melee_weapon )
			return 0;

		if ( g_vm_player_offset > 0 )
		{
			if ( const auto vm = view_model_from_offset( player , g_vm_player_offset ) )
				return vm;
			g_vm_player_offset = 0;
		}

		for ( int slot = 0; slot < 8; ++slot )
		{
			const auto vm = view_model_entity( player , slot );
			if ( !vm )
				continue;
			const auto wep = entity_from_handle( hypervisor->read<std::uint32_t>( vm + offsets::vm_m_h_weapon ) );
			if ( wep != melee_weapon )
				continue;

			g_vm_player_offset = offsets::m_h_view_models + slot * static_cast<int>( sizeof( std::uint32_t ) );
			const auto render = hypervisor->read<std::int32_t>( vm + offsets::vm_render_model_index );
			const auto anim = hypervisor->read<std::int32_t>( vm + offsets::vm_anim_model_index );
			log( "heirloom -> melee viewmodel slot=%d player+0x%x vm=0x%llx render=%d '%s' anim=%d '%s'" ,
				slot , g_vm_player_offset , vm ,
				render , model_path( render ).c_str( ) ,
				anim , model_path( anim ).c_str( ) );
			return vm;
		}

		for ( int off = offsets::m_h_view_models; off <= offsets::m_h_view_models + 0x40; off += 4 )
		{
			const auto vm = view_model_from_offset( player , off );
			if ( !vm )
				continue;
			const auto wep = entity_from_handle( hypervisor->read<std::uint32_t>( vm + offsets::vm_m_h_weapon ) );
			if ( wep != melee_weapon )
				continue;

			g_vm_player_offset = off;
			log( "heirloom -> melee viewmodel player+0x%x vm=0x%llx" , off , vm );
			return vm;
		}

		return 0;
	}

	auto read_class_name( std::uint64_t weapon ) -> std::string
	{
		if ( !weapon )
			return {};

		auto buf = hypervisor->read<std::array<char , 65>>( weapon + offsets::m_weapon_class_name );
		buf.back( ) = '\0';
		return buf[ 0 ] ? std::string( buf.data( ) ) : std::string { };
	}

	auto write_class_name( std::uint64_t weapon , const char* class_name ) -> void
	{
		if ( !weapon || !class_name || !class_name[ 0 ] )
			return;

		std::array<char , 65> buf { };
		std::strncpy( buf.data( ) , class_name , buf.size( ) - 1 );
		hypervisor->write_memory( weapon + offsets::m_weapon_class_name , buf.data( ) , buf.size( ) );
	}

	auto read_cstring( std::uint64_t ptr , std::size_t max_len ) -> std::string
	{
		if ( !is_usable( ptr ) )
			return {};

		const auto len = ( max_len > 255 ) ? 255 : max_len;
		auto buf = hypervisor->read<std::array<char , 256>>( ptr );
		buf[ len ] = '\0';
		return buf[ 0 ] ? std::string( buf.data( ) ) : std::string { };
	}

	auto normalize_path( std::string path ) -> std::string
	{
		for ( auto& c : path )
		{
			if ( c == '\\' )
				c = '/';
			else if ( c >= 'A' && c <= 'Z' )
				c = static_cast<char>( c - 'A' + 'a' );
		}
		return path;
	}

	auto path_basename( const std::string& path ) -> std::string
	{
		const auto pos = path.find_last_of( '/' );
		return ( pos == std::string::npos ) ? path : path.substr( pos + 1 );
	}

	auto read_table_string( const string_table_t& layout , int index ) -> std::string;

	auto model_path( int index ) -> std::string
	{
		if ( index <= 0 || !g_model_table.valid || index >= g_model_table.count )
			return {};
		return path_basename( normalize_path( read_table_string( g_model_table , index ) ) );
	}

	auto table_entry_valid( std::uint64_t valid_flag ) -> bool
	{
		return valid_flag != 0 && valid_flag != k_bad_entity;
	}

	auto read_table_string( const string_table_t& layout , int index ) -> std::string
	{
		if ( !layout.valid || index < 0 || index >= layout.count )
			return {};

		const auto entry = layout.items + static_cast<std::uint64_t>( index ) * static_cast<std::uint64_t>( layout.stride );
		const auto valid = hypervisor->read<std::uint64_t>( entry + layout.valid_off );
		if ( !table_entry_valid( valid ) )
			return {};

		const auto str_ptr = hypervisor->read<std::uint64_t>( entry + layout.str_off );
		return read_cstring( str_ptr , 255 );
	}

	auto find_in_table( const string_table_t& layout , const char* needle ) -> int
	{
		if ( !layout.valid || !needle || !needle[ 0 ] )
			return 0;

		for ( int i = 0; i < layout.count; ++i )
		{
			const auto s = read_table_string( layout , i );
			if ( s.empty( ) )
				continue;
			if ( s == needle || _stricmp( s.c_str( ) , needle ) == 0 )
				return i;
		}
		return 0;
	}

	auto rebuild_model_maps( ) -> void
	{
		if ( !g_model_table.valid )
			return;

		if ( g_model_table.items == g_model_map_items
			&& g_model_table.count == g_model_map_count
			&& !g_model_by_exact.empty( ) )
			return;

		g_model_by_exact.clear( );
		g_model_by_base.clear( );
		g_model_is_view.clear( );

		for ( int i = 0; i < g_model_table.count; ++i )
		{
			const auto raw = read_table_string( g_model_table , i );
			if ( raw.empty( ) )
				continue;

			const auto norm = normalize_path( raw );
			g_model_by_exact[ norm ] = i;
			g_model_is_view[ i ] = norm.find( "_v.rmdl" ) != std::string::npos;

			const auto base = path_basename( norm );
			if ( !base.empty( ) && !g_model_by_base.contains( base ) )
				g_model_by_base[ base ] = i;
		}

		g_model_map_items = g_model_table.items;
		g_model_map_count = g_model_table.count;
	}

	auto model_path_variants( const char* path ) -> std::vector<std::string>
	{
		std::vector<std::string> out;
		const auto norm = normalize_path( path );
		if ( norm.empty( ) )
			return out;

		out.push_back( norm );

		auto push_swap = [ & ]( std::string s , const char* from , const char* to )
		{
			const auto pos = s.rfind( from );
			if ( pos == std::string::npos )
				return;
			s.replace( pos , std::strlen( from ) , to );
			out.push_back( std::move( s ) );
		};

		push_swap( norm , "_v.rmdl" , "_w.rmdl" );
		push_swap( norm , "_w.rmdl" , "_v.rmdl" );

		const auto base = path_basename( norm );
		if ( !base.empty( ) && base != norm )
			out.push_back( base );

		return out;
	}

	auto find_model_index( const char* path ) -> int
	{
		if ( !path || !path[ 0 ] || !g_model_table.valid )
			return 0;

		rebuild_model_maps( );

		for ( const auto& candidate : model_path_variants( path ) )
		{
			if ( const auto it = g_model_by_exact.find( candidate ); it != g_model_by_exact.end( ) )
				return it->second;

			const auto base = path_basename( candidate );
			if ( const auto it = g_model_by_base.find( base ); it != g_model_by_base.end( ) )
				return it->second;
		}

		const auto norm = normalize_path( path );
		std::string stem = path_basename( norm );
		if ( const auto dot = stem.rfind( "_v.rmdl" ); dot != std::string::npos )
			stem.resize( dot );
		else if ( const auto dot = stem.rfind( "_w.rmdl" ); dot != std::string::npos )
			stem.resize( dot );
		else if ( const auto dot = stem.rfind( ".rmdl" ); dot != std::string::npos )
			stem.resize( dot );

		int best = 0;
		std::size_t best_len = 0;
		if ( !stem.empty( ) )
		{
			for ( const auto& [ table_path , index ] : g_model_by_exact )
			{
				if ( table_path.find( stem ) == std::string::npos )
					continue;
				if ( table_path.size( ) > best_len )
				{
					best = index;
					best_len = table_path.size( );
				}
			}
		}

		return best;
	}

	auto log_model_precache_sample( ) -> void
	{
		if ( g_logged_model_sample || !debug_on( ) || !g_model_table.valid )
			return;

		int heirloom_hits = 0;
		for ( int i = 0; i < g_model_table.count && heirloom_hits < 5; ++i )
		{
			const auto s = read_table_string( g_model_table , i );
			if ( s.find( "heirloom" ) == std::string::npos && s.find( "Heirloom" ) == std::string::npos )
				continue;
			log( "  modelprecache[%d] '%s'" , i , s.c_str( ) );
			++heirloom_hits;
		}

		log( "heirloom -> modelprecache scan | entries=%d heirloom-like=%d (need in-match load)" ,
			g_model_table.count ,
			heirloom_hits );
		g_logged_model_sample = true;
	}

	auto invalidate_model_cache( ) -> void
	{
		g_model_array_base = 0;
		g_model_table = { };
		g_model_by_exact.clear( );
		g_model_by_base.clear( );
		g_model_is_view.clear( );
		g_model_map_items = 0;
		g_model_map_count = 0;
		g_logged_model_sample = false;
	}

	auto refresh_model_table_layout( ) -> void
	{
		const auto table_ptr = hypervisor->read<std::uint64_t>( hypervisor->m_base_address + offsets::model_names );
		if ( !is_usable( table_ptr ) )
		{
			invalidate_model_cache( );
			return;
		}

		const auto sub = hypervisor->read<std::uint64_t>( table_ptr + 0x48 );
		if ( !is_usable( sub ) )
			return;

		const auto items = hypervisor->read<std::uint64_t>( sub + 0x18 );
		const auto capacity = hypervisor->read<std::uint32_t>( sub + 0x20 );
		if ( g_model_table.valid && items != g_model_table.items )
		{
			g_model_table.valid = false;
			g_model_by_exact.clear( );
			g_model_by_base.clear( );
			g_model_is_view.clear( );
			g_model_map_items = 0;
			g_model_map_count = 0;
			g_logged_model_sample = false;
			g_catalog_resolved = false;
		}

		if ( !g_model_table.valid )
			( void )probe_string_table( offsets::model_names , "modelprecache" , g_model_table );
		else if ( items != g_model_table.items || static_cast<int>( capacity ) != g_model_table.count )
		{
			g_model_table.items = items;
			g_model_table.count = static_cast<int>( capacity );
			g_model_map_items = 0;
		}
	}

	auto resolve_entry_indices( heirloom::definition_t& entry ) -> void
	{
		if ( entry.weapon_name_index <= 0 && entry.weapon_class && entry.weapon_class[ 0 ] )
		{
			if ( const int idx = find_in_table( g_weapon_table , entry.weapon_class ); idx > 0 )
				entry.weapon_name_index = idx;
		}

		if ( entry.view_model_index <= 0 && entry.view_model_path && entry.view_model_path[ 0 ] )
		{
			if ( const int idx = find_model_index( entry.view_model_path ); idx > 0 )
				entry.view_model_index = idx;
		}
	}

	auto probe_string_table( std::int32_t global_rva , const char* expected_name , string_table_t& out ) -> bool
	{
		const auto table_ptr = hypervisor->read<std::uint64_t>( hypervisor->m_base_address + global_rva );
		if ( !is_usable( table_ptr ) )
			return false;

		if ( expected_name && expected_name[ 0 ] )
		{
			const auto name_ptr = hypervisor->read<std::uint64_t>( table_ptr + 0x10 );
			const auto name = read_cstring( name_ptr , 63 );
			if ( !name.empty( ) && _stricmp( name.c_str( ) , expected_name ) != 0 )
				return false;
		}

		const auto max_entries = hypervisor->read<std::int32_t>( table_ptr + 0x18 );
		if ( max_entries <= 0 || max_entries > 0x20000 )
			return false;

		const auto sub = hypervisor->read<std::uint64_t>( table_ptr + 0x48 );
		if ( !is_usable( sub ) )
			return false;

		const auto data_array = hypervisor->read<std::uint64_t>( sub + 0x18 );
		const auto capacity = hypervisor->read<std::uint32_t>( sub + 0x20 );
		if ( !is_usable( data_array ) || capacity == 0 || capacity > 0x20000 )
			return false;

		int readable = 0;
		const int sample = ( capacity < 256u ) ? static_cast<int>( capacity ) : 256;
		for ( int i = 0; i < sample; ++i )
		{
			const auto entry = data_array + static_cast<std::uint64_t>( i ) * 0x48;
			const auto valid = hypervisor->read<std::uint64_t>( entry + 0x28 );
			if ( !valid || valid == k_bad_entity )
				continue;
			const auto sp = hypervisor->read<std::uint64_t>( entry + 0x10 );
			if ( !is_usable( sp ) )
				continue;
			const auto s = read_cstring( sp , 95 );
			if ( !s.empty( ) && s[ 0 ] != '\xCC' )
				++readable;
		}
		if ( readable < 3 )
			return false;

		out.valid = true;
		out.items = data_array;
		out.count = static_cast<int>( capacity );
		out.stride = 0x48;
		out.str_off = 0x10;
		out.valid_off = 0x28;
		return true;
	}

	auto ensure_string_tables( ) -> bool
	{
		if ( !g_weapon_table.valid )
			( void )probe_string_table( offsets::weapon_names , "WeaponNames" , g_weapon_table );
		refresh_model_table_layout( );
		return g_weapon_table.valid && g_model_table.valid;
	}

	auto resolve_indices( heirloom::definition_t& selected ) -> void
	{
		if ( !ensure_string_tables( ) )
		{
			if ( debug_on( ) && ( g_table_fail_logs++ % 30 == 0 ) )
				log( "heirloom -> string tables not ready (in firing range / match)" );
			return;
		}
		g_table_fail_logs = 0;

		if ( !g_catalog_resolved )
		{
			int weapon_resolved = 0;
			int model_resolved = 0;

			std::unordered_map<std::string , int> wanted_models;
			for ( auto& entry : heirloom::catalog( ) )
			{
				if ( entry.view_model_index <= 0 && entry.view_model_path && entry.view_model_path[ 0 ] )
					wanted_models.emplace( normalize_path( entry.view_model_path ) , -1 );
			}

			rebuild_model_maps( );
			std::size_t model_remaining = wanted_models.size( );
			for ( int index = 0; index < g_model_table.count && model_remaining > 0; ++index )
			{
				const auto table_path = normalize_path( read_table_string( g_model_table , index ) );
				if ( table_path.empty( ) )
					continue;

				if ( const auto it = wanted_models.find( table_path ); it != wanted_models.end( ) && it->second < 0 )
				{
					it->second = index;
					--model_remaining;
					continue;
				}

				const auto base = path_basename( table_path );
				for ( auto& [ want_path , want_idx ] : wanted_models )
				{
					if ( want_idx >= 0 )
						continue;
					if ( path_basename( want_path ) == base )
					{
						want_idx = index;
						--model_remaining;
					}
				}
			}

			for ( auto& entry : heirloom::catalog( ) )
			{
				resolve_entry_indices( entry );

				if ( entry.view_model_index <= 0 && entry.view_model_path && entry.view_model_path[ 0 ] )
				{
					const auto it = wanted_models.find( normalize_path( entry.view_model_path ) );
					if ( it != wanted_models.end( ) && it->second > 0 )
						entry.view_model_index = it->second;
				}

				if ( entry.weapon_name_index > 0 )
					++weapon_resolved;
				if ( entry.view_model_index > 0 )
					++model_resolved;
			}

			if ( model_resolved == 0 )
				log_model_precache_sample( );

			if ( weapon_resolved > 0 || model_resolved > 0 )
			{
				g_catalog_resolved = true;
				log( "heirloom -> tables OK | weapon_names=%d model_names=%d / catalog=%zu" ,
					weapon_resolved ,
					model_resolved ,
					heirloom::catalog( ).size( ) );
			}
			else if ( debug_on( ) )
				log( "heirloom -> tables probed but 0 catalog hits (paths/class mismatch?)" );
		}

		resolve_entry_indices( selected );
		if ( debug_on( ) && ( g_apply_count == 0 || g_apply_count % k_debug_apply_interval == 0 ) )
		{
			log( "heirloom -> selected '%s' | name_idx=%d vm_idx=%d class=%s" ,
				selected.name ? selected.name : "?" ,
				selected.weapon_name_index ,
				selected.view_model_index ,
				selected.weapon_class ? selected.weapon_class : "?" );
		}
	}

	auto dump_inventory_once( std::uint64_t player ) -> void
	{
		if ( g_inventory_dumped || !debug_on( ) )
			return;
		g_inventory_dumped = true;

		log( "heirloom -> inventory dump (player=0x%llx)" , player );
		for ( int slot = 0; slot < k_inventory_slots; ++slot )
		{
			const auto handle = inventory_handle( player , slot );
			const auto ent = entity_from_handle( handle );
			if ( !handle && !ent )
				continue;

			const auto cls = ent ? read_class_name( ent ) : std::string {};
			log( "  slot[%d] handle=0x%08x ent=0x%llx cls='%s'" , slot , handle , ent , cls.c_str( ) );
		}

		const auto offhand_handle = hypervisor->read<std::uint32_t>(
			player + offsets::m_inventory + k_offhand_base + k_melee_offhand_slot * sizeof( std::uint32_t ) );
		const auto offhand_ent = entity_from_handle( offhand_handle );
		log( "  offhand melee slot[%d] handle=0x%08x ent=0x%llx cls='%s'" ,
			k_melee_offhand_slot ,
			offhand_handle ,
			offhand_ent ,
			offhand_ent ? read_class_name( offhand_ent ).c_str( ) : "" );

		for ( int vm_slot = 0; vm_slot < 8; ++vm_slot )
		{
			const auto vm_handle = hypervisor->read<std::uint32_t>( player + offsets::m_h_view_models + vm_slot * sizeof( std::uint32_t ) );
			const auto vm_ent = entity_from_handle( vm_handle );
			if ( !vm_handle && !vm_ent )
				continue;
			const auto wep = vm_ent
				? entity_from_handle( hypervisor->read<std::uint32_t>( vm_ent + offsets::vm_m_h_weapon ) )
				: 0ull;
			log( "  vm[%d] handle=0x%08x ent=0x%llx wep=0x%llx cls='%s'" ,
				vm_slot ,
				vm_handle ,
				vm_ent ,
				wep ,
				wep ? read_class_name( wep ).c_str( ) : "" );
		}

		const auto offhand = offhand_melee_entity( player );
		if ( const auto vm = discover_view_model( player , offhand ) )
		{
			const auto wh = hypervisor->read<std::uint32_t>( vm + offsets::vm_m_h_weapon );
			log( "  vm discovered ent=0x%llx player+0x%x weapon_handle=0x%08x" , vm , g_vm_player_offset , wh );
		}
		else
			log( "  vm discovered none (pull melee in first person)" );
	}

	auto find_heirloom_primary( std::uint64_t player ) -> std::uint64_t
	{
		for ( int slot = 0; slot < k_inventory_slots; ++slot )
		{
			const auto ent = weapon_entity( player , slot );
			if ( !ent )
				continue;
			const auto cls = read_class_name( ent );
			if ( cls.find( "_heirloom_" ) != std::string::npos
				|| cls.find( "_kunai_" ) != std::string::npos )
				return ent;
		}
		return 0;
	}

	auto model_array_base( ) -> std::uint64_t
	{
		if ( g_model_array_base )
			return g_model_array_base;

		const auto table = hypervisor->m_base_address + offsets::model_names;
		static bool dumped = false;
		if ( !dumped )
		{
			dumped = true;
			log( "heirloom -> model table probe base=0x%llx" , table );
			for ( int off = 0; off <= 0x80; off += 0x08 )
			{
				const auto v = hypervisor->read<std::uint64_t>( table + off );
				log( "  modeltable+0x%02x = 0x%llx" , off , v );
			}
		}

		const auto direct = hypervisor->read<std::uint64_t>( table + 0x30 );
		if ( is_usable( direct ) )
		{
			g_model_array_base = direct;
			log( "heirloom -> model array base 0x%llx (deref +0x30)" , direct );
			return direct;
		}

		g_model_array_base = table + 0x30;
		return g_model_array_base;
	}

	auto model_slot_ptr( int index ) -> std::uint64_t
	{
		if ( index <= 0 )
			return 0;
		return model_array_base( )
			+ static_cast<std::uint64_t>( index ) * offsets::model_index_stride
			+ offsets::model_index_pointer;
	}

	auto restore_world_draw( ) -> void
	{
		if ( !g_hidden_world || g_hidden_world_index < 0 )
			return;
		hypervisor->write<std::int32_t>( g_hidden_world + offsets::m_i_world_model_index , g_hidden_world_index );
		g_hidden_world = 0;
		g_hidden_world_index = -1;
	}

	auto hide_world_model( std::uint64_t melee ) -> void
	{
		if ( !melee )
			return;

		if ( g_hidden_world && g_hidden_world != melee )
			restore_world_draw( );

		const auto live = hypervisor->read<std::int32_t>( melee + offsets::m_i_world_model_index );
		if ( !g_hidden_world )
		{
			g_hidden_world_index = live;
			g_hidden_world = melee;
		}

		if ( live != 0 )
			hypervisor->write<std::int32_t>( melee + offsets::m_i_world_model_index , 0 );
	}

	auto restore_studio( ) -> void
	{
		if ( !g_studio_wrapper || !g_studio_saved )
			return;
		hypervisor->write<std::uint64_t>( g_studio_wrapper + 0x08 , g_studio_saved );
		g_studio_wrapper = 0;
		g_studio_saved = 0;
	}

	auto alias_studio( std::uint64_t vm , int target_index ) -> void
	{
		const auto wrapper = hypervisor->read<std::uint64_t>( vm + offsets::vm_studio_hdr );
		if ( !is_usable( wrapper ) )
			return;

		const auto source_model = hypervisor->read<std::uint64_t>( wrapper + 0x08 );
		const auto target_model = hypervisor->read<std::uint64_t>( model_slot_ptr( target_index ) );
		if ( !is_usable( source_model ) || !is_usable( target_model ) )
			return;

		const auto source_data = hypervisor->read<std::uint64_t>( source_model + 0x08 );
		const auto target_data = hypervisor->read<std::uint64_t>( target_model + 0x08 );
		if ( !is_usable( source_data ) || !is_usable( target_data ) || source_data == target_data )
			return;

		if ( hypervisor->read<std::uint32_t>( target_data ) != 0x54534449u )
		{
			log( "heirloom -> studio data 0x%llx has no model header, skipped" , target_data );
			return;
		}

		if ( g_studio_wrapper != source_model )
		{
			g_studio_saved = source_data;
			g_studio_wrapper = source_model;
			log( "heirloom -> studio model 0x%llx held data 0x%llx" , source_model , source_data );
		}

		hypervisor->write<std::uint64_t>( source_model + 0x08 , target_data );
		const auto back = hypervisor->read<std::uint64_t>( source_model + 0x08 );
		log( "heirloom -> studio data 0x%llx -> 0x%llx readback 0x%llx %s" ,
			source_data , target_data , back , ( back == target_data ) ? "STUCK" : "REJECTED" );
	}

	auto restore_model_alias( ) -> void
	{
		restore_studio( );
		if ( g_alias_source <= 0 || !g_alias_saved_model )
			return;
		if ( const auto slot = model_slot_ptr( g_alias_source ) )
			hypervisor->write<std::uint64_t>( slot , g_alias_saved_model );
		g_alias_source = -1;
		g_alias_target = -1;
		g_alias_saved_model = 0;
		g_anim_start_captured = false;
		g_sequence_pinned = false;
		g_pinned_sequence = 0;
		g_pinned_rate = 0.0f;
		g_held_valid = false;
		g_watch_valid = false;
	}

	auto patch_overlay_models( std::uint64_t vm , int target_index ) -> void
	{
		const auto overlay = hypervisor->read<std::uint64_t>( vm + offsets::vm_anim_overlay );
		if ( !is_usable( overlay ) )
			return;

		auto layers = hypervisor->read<std::int32_t>( vm + offsets::vm_anim_overlay_count );
		if ( layers <= 0 || layers > 15 )
			return;

		for ( int i = 0; i < layers; ++i )
		{
			const auto addr = overlay + offsets::vm_overlay_model_index
				+ static_cast<std::uint64_t>( i ) * sizeof( std::int32_t );
			if ( hypervisor->read<std::int32_t>( addr ) != target_index )
				hypervisor->write<std::int32_t>( addr , target_index );
		}
	}

	auto alias_view_model( std::uint64_t vm , int target_index ) -> void
	{
		if ( !vm || target_index <= 0 )
			return;

		const bool view_mesh = g_model_is_view.contains( target_index ) && g_model_is_view[ target_index ];
		if ( !view_mesh )
		{
			restore_model_alias( );
			return;
		}

		const auto live = hypervisor->read<std::int32_t>( vm + offsets::vm_render_model_index );
		if ( live <= 0 )
			return;

		const auto target_model = hypervisor->read<std::uint64_t>( model_slot_ptr( target_index ) );
		if ( !is_usable( target_model ) )
		{
			log( "heirloom -> alias bail target slot unusable idx=%d ptr=0x%llx" ,
				target_index , target_model );
			return;
		}

		if ( g_alias_source == live && g_alias_target == target_index )
		{
			const auto current = hypervisor->read<std::uint64_t>( model_slot_ptr( live ) );
			if ( current != target_model )
				hypervisor->write<std::uint64_t>( model_slot_ptr( live ) , target_model );
			return;
		}

		restore_model_alias( );

		const auto source_slot = model_slot_ptr( live );
		g_alias_saved_model = hypervisor->read<std::uint64_t>( source_slot );
		if ( !is_usable( g_alias_saved_model ) || g_alias_saved_model == target_model )
		{
			g_alias_saved_model = 0;
			return;
		}

		hypervisor->write<std::uint64_t>( source_slot , target_model );
		g_alias_source = live;
		g_alias_target = target_index;
		log( "heirloom -> model alias %d -> %d (model 0x%llx)" , live , target_index , target_model );
	}

	auto patch_weapon_entity( std::uint64_t ent , const heirloom::definition_t& def ) -> void
	{
		const bool view_mesh = g_model_is_view.contains( def.view_model_index ) && g_model_is_view[ def.view_model_index ];

		if ( view_mesh && def.weapon_name_index > 0 )
		{
			const auto live = hypervisor->read<std::int32_t>( ent + offsets::m_weapon_name_index );
			if ( live != def.weapon_name_index )
				hypervisor->write<std::int32_t>( ent + offsets::m_weapon_name_index , def.weapon_name_index );
		}

		if ( view_mesh && def.weapon_class )
		{
			const auto live = read_class_name( ent );
			if ( live != def.weapon_class )
				write_class_name( ent , def.weapon_class );
		}

		if ( view_mesh && def.skin_index >= 0 )
		{
			const auto live = hypervisor->read<std::int32_t>( ent + offsets::m_n_skin );
			if ( live != def.skin_index )
				hypervisor->write<std::int32_t>( ent + offsets::m_n_skin , def.skin_index );
		}
		if ( view_mesh && def.camo_index >= 0 )
		{
			const auto live = hypervisor->read<std::int32_t>( ent + offsets::m_camo_index );
			if ( live != def.camo_index )
				hypervisor->write<std::int32_t>( ent + offsets::m_camo_index , def.camo_index );
		}
	}

	auto find_melee( std::uint64_t player , const heirloom::definition_t& def ) -> std::uint64_t
	{
		( void )def;

		if ( const auto offhand = offhand_melee_entity( player ) )
		{
			if ( g_melee_slot != k_melee_src_offhand )
			{
				g_melee_slot = k_melee_src_offhand;
				g_logged_offhand_melee = true;
				log( "heirloom -> melee via offhand slot=%d ent=0x%llx cls='%s'" ,
					k_melee_offhand_slot ,
					offhand ,
					read_class_name( offhand ).c_str( ) );
			}
			return offhand;
		}

		g_melee_slot = -1;
		g_logged_offhand_melee = false;
		return 0;
	}

	auto apply( std::uint64_t player , const heirloom::definition_t& def ) -> bool
	{
		const auto melee = find_melee( player , def );
		if ( !melee )
			return false;

		patch_weapon_entity( melee , def );
		hide_world_model( melee );

		const auto skip_reset = hypervisor->m_base_address
			+ static_cast<std::uint64_t>( offsets::allow_offhand_skip_sequence_reset ) + offsets::cvar_value;
		if ( hypervisor->read<std::int32_t>( skip_reset ) != 1 )
			hypervisor->write<std::int32_t>( skip_reset , 1 );

		const bool view_mesh = g_model_is_view.contains( def.view_model_index ) && g_model_is_view[ def.view_model_index ];
		if ( view_mesh )
		{
			if ( const auto vm = discover_view_model( player , melee ) )
			{
				alias_view_model( vm , def.view_model_index );
				alias_studio( vm , def.view_model_index );
				patch_overlay_models( vm , def.view_model_index );

				hypervisor->write<std::uint8_t>( vm + offsets::vm_sequence_finished , 0 );
				// #region agent log
				{
					static int dbg_ticks = 0;
					if ( ( dbg_ticks++ % 25 ) == 0 )
					{
						const auto seq_before = hypervisor->read<std::uint16_t>( vm + offsets::vm_anim_sequence );
						const auto start_before = hypervisor->read<float>( vm + offsets::vm_anim_start_time );
						FILE* f = nullptr;
						fopen_s( &f , "C:\\Users\\hey\\Desktop\\apex-module\\debug-e177f0.log" , "a" );
						if ( f )
						{
							fprintf( f , "{\"sessionId\":\"e177f0\",\"hypothesisId\":\"A-D\",\"location\":\"heirloom.cc:pin\",\"message\":\"pin tick\",\"data\":{\"pinned\":%d,\"pinnedSeq\":%u,\"pinnedRate\":%.3f,\"frozenStart\":%.3f,\"liveSeq\":%u,\"liveStart\":%.3f},\"timestamp\":%llu}\n" ,
								g_sequence_pinned ? 1 : 0 , g_pinned_sequence , g_pinned_rate , g_anim_start_frozen ,
								seq_before , start_before ,
								static_cast<unsigned long long>( GetTickCount64( ) ) );
							fclose( f );
						}
					}
				}
				// #endregion
				if ( !g_sequence_pinned )
				{
					g_pinned_sequence = hypervisor->read<std::uint16_t>( vm + offsets::vm_anim_sequence );
					g_pinned_rate = hypervisor->read<float>( vm + offsets::vm_anim_playback_rate );
					g_anim_start_frozen = hypervisor->read<float>( vm + offsets::vm_anim_start_time );
					g_sequence_pinned = g_pinned_sequence != 0;
				}
				else
				{
					if ( hypervisor->read<std::uint16_t>( vm + offsets::vm_anim_sequence ) != g_pinned_sequence )
						hypervisor->write<std::uint16_t>( vm + offsets::vm_anim_sequence , g_pinned_sequence );
					hypervisor->write<float>( vm + offsets::vm_anim_start_time , g_anim_start_frozen );
					hypervisor->write<float>( vm + offsets::vm_anim_playback_rate , g_pinned_rate );
				}
			}
		}
		else
		{
			restore_model_alias( );
			g_held_valid = false;
		}

		if ( debug_on( ) )
		{
			std::uint64_t watched = 0;
			for ( int slot = 0; slot < 8 && !watched; ++slot )
			{
				const auto candidate = view_model_from_offset(
					player , offsets::m_h_view_models + slot * static_cast<int>( sizeof( std::uint32_t ) ) );
				if ( !candidate )
					continue;
				const auto wep = entity_from_handle( hypervisor->read<std::uint32_t>( candidate + offsets::vm_m_h_weapon ) );
				if ( wep == melee )
					watched = candidate;
			}
			if ( !watched && g_vm_player_offset > 0 )
				watched = view_model_from_offset( player , g_vm_player_offset );

			if ( watched )
			{
				const auto seq = hypervisor->read<std::uint16_t>( watched + offsets::vm_anim_sequence );
				const auto start = hypervisor->read<float>( watched + offsets::vm_anim_start_time );
				const auto rate = hypervisor->read<float>( watched + offsets::vm_anim_playback_rate );
				const auto cycle = hypervisor->read<float>( watched + offsets::vm_anim_start_cycle );
				const auto fidget = hypervisor->read<float>( watched + offsets::vm_next_fidget_time );
				const auto finished = hypervisor->read<std::uint8_t>( watched + offsets::vm_sequence_finished );

				const bool changed = !g_watch_valid
					|| seq != g_watch_sequence
					|| start != g_watch_start
					|| rate != g_watch_rate
					|| cycle != g_watch_cycle
					|| fidget != g_watch_fidget
					|| finished != g_watch_finished;

				if ( changed )
				{
					const auto overlay = hypervisor->read<std::uint64_t>( watched + offsets::vm_anim_overlay );
					const auto layers = hypervisor->read<std::int32_t>( watched + offsets::vm_anim_overlay_count );
					std::int32_t layer0 = -1;
					std::int32_t layer1 = -1;
					if ( is_usable( overlay ) && layers > 0 && layers <= 15 )
					{
						layer0 = hypervisor->read<std::int32_t>( overlay + offsets::vm_overlay_model_index );
						if ( layers > 1 )
							layer1 = hypervisor->read<std::int32_t>( overlay + offsets::vm_overlay_model_index + sizeof( std::int32_t ) );
					}
					const auto studio = hypervisor->read<std::uint64_t>( watched + offsets::vm_studio_hdr );
					const auto studio_model = is_usable( studio ) ? hypervisor->read<std::uint64_t>( studio + 0x08 ) : 0;
					const auto skip = hypervisor->read<std::int32_t>( hypervisor->m_base_address
						+ static_cast<std::uint64_t>( offsets::allow_offhand_skip_sequence_reset ) + offsets::cvar_value );

					log( "  anim vm=0x%llx seq %u -> %u | start %.3f -> %.3f | rate %.3f -> %.3f | cycle %.3f -> %.3f | fidget %.3f -> %.3f | finished %u -> %u" ,
						watched ,
						g_watch_valid ? g_watch_sequence : 0 , seq ,
						g_watch_valid ? g_watch_start : 0.0f , start ,
						g_watch_valid ? g_watch_rate : 0.0f , rate ,
						g_watch_valid ? g_watch_cycle : 0.0f , cycle ,
						g_watch_valid ? g_watch_fidget : 0.0f , fidget ,
						g_watch_valid ? g_watch_finished : 0 , finished );
					log( "    overlay ptr=0x%llx layers=%d layer0=%d '%s' layer1=%d '%s' | studio=0x%llx model=0x%llx skip_reset=%d" ,
						overlay , layers ,
						layer0 , model_path( layer0 ).c_str( ) ,
						layer1 , model_path( layer1 ).c_str( ) ,
						studio , studio_model , skip );

					g_watch_sequence = seq;
					g_watch_start = start;
					g_watch_rate = rate;
					g_watch_cycle = cycle;
					g_watch_fidget = fidget;
					g_watch_finished = finished;
					g_watch_valid = true;
				}
			}
		}

		++g_apply_count;
		const bool periodic = ( g_apply_count == 1 ) || ( g_apply_count % k_debug_apply_interval == 0 );
		if ( !debug_on( ) || !periodic )
			return true;

		const auto rb_name = hypervisor->read<std::int32_t>( melee + offsets::m_weapon_name_index );
		const auto rb_world = hypervisor->read<std::int32_t>( melee + offsets::m_i_world_model_index );
		const auto rb_class = read_class_name( melee );

		std::uint64_t vm0 = 0;
		std::int32_t rb_vm_render = -1;
		std::int32_t rb_vm_anim = -1;
		std::uint32_t rb_vm_weapon = 0;
		std::uint64_t rb_vm_weapon_ent = 0;
		int rb_vm_off = 0;
		for ( int slot = 0; slot < 8 && !vm0; ++slot )
		{
			const auto candidate = view_model_from_offset(
				player , offsets::m_h_view_models + slot * static_cast<int>( sizeof( std::uint32_t ) ) );
			if ( !candidate )
				continue;
			const auto wep = entity_from_handle( hypervisor->read<std::uint32_t>( candidate + offsets::vm_m_h_weapon ) );
			if ( wep == melee )
			{
				vm0 = candidate;
				rb_vm_off = offsets::m_h_view_models + slot * static_cast<int>( sizeof( std::uint32_t ) );
			}
		}
		if ( !vm0 && g_vm_player_offset > 0 )
		{
			vm0 = view_model_from_offset( player , g_vm_player_offset );
			rb_vm_off = g_vm_player_offset;
		}
		if ( vm0 )
		{
			rb_vm_render = hypervisor->read<std::int32_t>( vm0 + offsets::vm_render_model_index );
			rb_vm_anim = hypervisor->read<std::int32_t>( vm0 + offsets::vm_anim_model_index );
			rb_vm_weapon = hypervisor->read<std::uint32_t>( vm0 + offsets::vm_m_h_weapon );
			rb_vm_weapon_ent = entity_from_handle( rb_vm_weapon );
		}

		const auto rb_body = hypervisor->read<std::int32_t>( melee + offsets::m_n_model_index );
		const auto rb_holster = hypervisor->read<std::int32_t>( melee + offsets::m_holster_model_index );
		const auto rb_dropped = hypervisor->read<std::int32_t>( melee + offsets::m_dropped_model_index );
		const auto rb_effects = hypervisor->read<std::int32_t>( melee + offsets::m_f_effects );

		const auto aliased = ( g_alias_source > 0 )
			? hypervisor->read<std::uint64_t>( model_slot_ptr( g_alias_source ) ) : 0;
		const auto alias_live = ( g_alias_target > 0 )
			? hypervisor->read<std::uint64_t>( model_slot_ptr( g_alias_target ) ) : 0;

		const char* slot_label = ( g_melee_slot == k_melee_src_offhand ) ? "offhand" : ( g_melee_slot >= 0 ? "inv" : "?" );
		log( "heirloom -> apply #%d '%s' | melee=0x%llx src=%s(%d) vm_off=0x%x" ,
			g_apply_count ,
			def.name ? def.name : "?" ,
			melee ,
			slot_label ,
			g_melee_slot ,
			rb_vm_off );

		log( "  melee models | body=%d '%s' world=%d '%s' holster=%d '%s' dropped=%d '%s' fx=0x%x" ,
			rb_body , model_path( rb_body ).c_str( ) ,
			rb_world , model_path( rb_world ).c_str( ) ,
			rb_holster , model_path( rb_holster ).c_str( ) ,
			rb_dropped , model_path( rb_dropped ).c_str( ) ,
			rb_effects );

		log( "  wrote name_idx=%d skin=%d camo=%d vm_idx=%d | rb name=%d cls='%s'" ,
			def.weapon_name_index ,
			def.skin_index ,
			def.camo_index ,
			def.view_model_index ,
			rb_name ,
			rb_class.c_str( ) );

		log( "  vm=0x%llx bound_wep=0x%llx (handle=0x%08x melee=%s) | render=%d '%s' anim=%d '%s'" ,
			vm0 ,
			rb_vm_weapon_ent ,
			rb_vm_weapon ,
			( rb_vm_weapon_ent == melee ) ? "YES" : "no" ,
			rb_vm_render , model_path( rb_vm_render ).c_str( ) ,
			rb_vm_anim , model_path( rb_vm_anim ).c_str( ) );

		if ( vm0 )
		{
			const auto overlay = hypervisor->read<std::uint64_t>( vm0 + offsets::vm_anim_overlay );
			const auto layers = hypervisor->read<std::int32_t>( vm0 + offsets::vm_anim_overlay_count );
			std::int32_t layer0 = -1;
			if ( is_usable( overlay ) && layers > 0 && layers <= 15 )
				layer0 = hypervisor->read<std::int32_t>( overlay + offsets::vm_overlay_model_index );
			log( "  overlay ptr=0x%llx layers=%d layer0=%d '%s'" ,
				overlay , layers , layer0 , model_path( layer0 ).c_str( ) );
		}

		log( "  alias src=%d -> tgt=%d | slot_now=0x%llx saved=0x%llx target_model=0x%llx %s" ,
			g_alias_source ,
			g_alias_target ,
			aliased ,
			g_alias_saved_model ,
			alias_live ,
			( g_alias_source > 0 && aliased == alias_live ) ? "HOLDING" : "NOT-HOLDING" );

		return true;
	}

	auto build_labels( ) -> void
	{
		if ( !g_labels.empty( ) )
			return;

		const auto& list = heirloom::catalog( );
		g_labels.reserve( list.size( ) );
		for ( const auto& entry : list )
		{
			std::string line = entry.legend ? entry.legend : "?";
			line += " - ";
			line += entry.name ? entry.name : "?";
			g_labels.push_back( std::move( line ) );
		}
	}
}

namespace heirloom
{
	auto catalog_count( ) -> int
	{
		build_labels( );
		return static_cast<int>( catalog( ).size( ) );
	}

	auto catalog_label( int index ) -> const char*
	{
		build_labels( );
		if ( index < 0 || index >= static_cast<int>( g_labels.size( ) ) )
			return "None";
		return g_labels[ static_cast<std::size_t>( index ) ].c_str( );
	}

	auto tick( ) -> void
	{
		build_labels( );
		log( "heirloom -> thread online" );

		for ( ;; )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

			if ( !global::heirloom::enabled )
			{
				if ( g_was_enabled )
				{
					log( "heirloom -> disabled" );
					restore_world_draw( );
					g_was_enabled = false;
					g_melee_slot = -1;
					g_apply_count = 0;
					g_last_logged_selection = -1;
					g_inventory_dumped = false;
					restore_model_alias( );
					g_catalog_resolved = false;
					g_weapon_table = { };
					invalidate_model_cache( );
					g_vm_player_offset = 0;
					g_logged_offhand_melee = false;
					g_table_fail_logs = 0;
				}
				continue;
			}

			if ( !g_was_enabled )
			{
				g_was_enabled = true;
				g_catalog_resolved = false;
				g_weapon_table = { };
				invalidate_model_cache( );
				g_vm_player_offset = 0;
				g_logged_offhand_melee = false;
				g_apply_count = 0;
				log( "heirloom -> enabled | selection=%d '%s'" ,
					global::heirloom::selection ,
					catalog_label( global::heirloom::selection ) );
			}

			const auto player = hypervisor->read<std::uint64_t>( hypervisor->m_base_address + offsets::local_player );
			if ( !player || player == k_bad_entity )
			{
				if ( debug_on( ) && g_apply_count == 0 && g_was_enabled )
					log( "heirloom -> waiting for local player" );
				continue;
			}

			auto& list = catalog( );
			if ( list.empty( ) )
				continue;

			int pick = global::heirloom::selection;
			if ( pick < 0 )
				pick = 0;
			if ( pick >= static_cast<int>( list.size( ) ) )
				pick = static_cast<int>( list.size( ) ) - 1;

			if ( pick != g_last_logged_selection )
			{
				g_last_logged_selection = pick;
				g_melee_slot = -1;
				g_logged_offhand_melee = false;
				log( "heirloom -> selection=%d '%s'" , pick , catalog_label( pick ) );
			}

			auto& def = list[ static_cast<std::size_t>( pick ) ];
			resolve_indices( def );
			dump_inventory_once( player );

			if ( !apply( player , def ) )
			{
				if ( debug_on( ) && ( g_apply_count == 0 || g_apply_count % k_debug_apply_interval == 0 ) )
					log( "heirloom -> no melee entity (pull knife / check inventory dump above)" );
			}
		}
	}
}
