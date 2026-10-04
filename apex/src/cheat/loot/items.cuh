#pragma once

#include <src/cheat/loot/types.cuh>
#include <src/cheat/loot/survival.cuh>
#include <src/utility/global/global.cuh>
#include <dependencies/imgui/imgui.h>
#include <src/driver/driver.cuh>
#include <src/sdk/offsets/offsets.cuh>
#include <cstring>
#include <cctype>
#include <cstdio>

namespace loot
{
	struct resolved_t
	{
		std::string name = "Loot";
		rarity_t rarity = rarity_t::common;
		category_t category = category_t::unknown;
	};

	inline auto color( rarity_t r ) -> ImU32
	{
		switch ( r )
		{
		case rarity_t::rare:      return IM_COL32( 30 , 144 , 255 , 255 );
		case rarity_t::epic:      return IM_COL32( 170 , 0 , 255 , 255 );
		case rarity_t::legendary: return IM_COL32( 255 , 205 , 60 , 255 );
		case rarity_t::heirloom:  return IM_COL32( 255 , 78 , 29 , 255 );
		default:                  return IM_COL32( 200 , 200 , 200 , 255 );
		}
	}

	inline auto allowed( category_t c ) -> bool
	{
		switch ( c )
		{
		case category_t::weapon:     return global::loot::weapons;
		case category_t::ammo:       return global::loot::ammo;
		case category_t::heal:       return global::loot::heals;
		case category_t::gear:       return global::loot::gear;
		case category_t::attachment: return global::loot::attachments;
		case category_t::grenade:    return global::loot::grenades;
		case category_t::deathbox:   return global::loot::death_box;
		case category_t::misc:       return global::loot::misc;
		default:                     return global::loot::misc;
		}
	}

	namespace detail
	{
		inline auto read_str( std::uint64_t ptr ) -> std::string
		{
			if ( !ptr || !hypervisor->is_valid( ptr ) )
				return {};

			auto buf = hypervisor->read<std::array<char , 128>>( ptr );
			buf.back( ) = '\0';
			if ( !buf[ 0 ] )
				return {};

			for ( int i = 0; i < static_cast<int>( buf.size( ) ) - 1 && buf[ i ]; ++i )
			{
				const auto c = static_cast<unsigned char>( buf[ i ] );
				if ( c < 32 || c == 127 )
				{
					buf[ i ] = '\0';
					break;
				}
			}
			return buf[ 0 ] ? std::string( buf.data( ) ) : std::string { };
		}

		inline auto table_str( std::uint64_t table , int index , std::uint64_t stride ) -> std::string
		{
			if ( index <= 0 || !table )
				return {};

			const std::uint64_t candidates[] = {
				table + static_cast<std::uint64_t>( index ) * stride ,
				table + static_cast<std::uint64_t>( index - 1 ) * stride ,
			};

			for ( const auto addr : candidates )
			{
				auto s = read_str( hypervisor->read<std::uint64_t>( addr ) );
				if ( !s.empty( ) )
					return s;

				s = read_str( addr );
				if ( !s.empty( ) && ( s.find( '/' ) != std::string::npos || s.find( "mp_" ) == 0 ) )
					return s;
			}
			return {};
		}

		struct match_t
		{
			const char* token;
			const char* name;
			rarity_t rarity;
			category_t category;
		};

		inline constexpr match_t k_matches[] = {
			{ "ammo_sc" , "Light Rounds" , rarity_t::common , category_t::ammo } ,
			{ "ammo_nrg" , "Energy Ammo" , rarity_t::common , category_t::ammo } ,
			{ "ammo_shg" , "Shotgun Shells" , rarity_t::common , category_t::ammo } ,
			{ "ammo_hc" , "Heavy Rounds" , rarity_t::common , category_t::ammo } ,
			{ "ammo_sniper" , "Sniper Ammo" , rarity_t::common , category_t::ammo } ,
			{ "arrows_mn" , "Arrows" , rarity_t::common , category_t::ammo } ,
			{ "arrow_single" , "Arrow" , rarity_t::common , category_t::ammo } ,

			{ "phoenix_kit" , "Phoenix Kit" , rarity_t::epic , category_t::heal } ,
			{ "health_main_large" , "Med Kit" , rarity_t::rare , category_t::heal } ,
			{ "health_main_small" , "Syringe" , rarity_t::common , category_t::heal } ,
			{ "shield_battery_large" , "Shield Battery" , rarity_t::rare , category_t::heal } ,
			{ "shield_battery_small" , "Shield Cell" , rarity_t::common , category_t::heal } ,
			{ "ultimate_accelerant" , "Ult Accel" , rarity_t::rare , category_t::heal } ,

			{ "thermite_grenade" , "Thermite" , rarity_t::common , category_t::grenade } ,
			{ "f_grenade" , "Frag" , rarity_t::common , category_t::grenade } ,
			{ "shuriken" , "Arc Star" , rarity_t::common , category_t::grenade } ,
			{ "frag_grenade" , "Frag" , rarity_t::common , category_t::grenade } ,

			{ "shield_upgrade_head" , "Helmet" , rarity_t::common , category_t::gear } ,
			{ "shield_upgrade_body" , "Body Shield" , rarity_t::common , category_t::gear } ,
			{ "shield_down" , "Knockdown Shield" , rarity_t::common , category_t::gear } ,
			{ "backpack_light" , "Backpack" , rarity_t::common , category_t::gear } ,
			{ "backpack_medium" , "Backpack" , rarity_t::rare , category_t::gear } ,
			{ "backpack_heavy" , "Backpack" , rarity_t::epic , category_t::gear } ,
			{ "heat_shield" , "Heat Shield" , rarity_t::rare , category_t::misc } ,
			{ "beacon_capsule" , "Mobile Respawn" , rarity_t::rare , category_t::misc } ,
			{ "golden_ticket" , "Golden Ticket" , rarity_t::legendary , category_t::misc } ,
			{ "keycard" , "Keycard" , rarity_t::legendary , category_t::misc } ,
			{ "treasure_box" , "Treasure Pack" , rarity_t::legendary , category_t::misc } ,

			{ "optic_cq_hcog_r1" , "1x HCOG" , rarity_t::common , category_t::attachment } ,
			{ "optic_cq_hcog_r2" , "2x HCOG" , rarity_t::rare , category_t::attachment } ,
			{ "optic_rng_hcog" , "3x HCOG" , rarity_t::rare , category_t::attachment } ,
			{ "optic_cq_threat" , "1x Digi Threat" , rarity_t::legendary , category_t::attachment } ,
			{ "optic_cq_holo_var_2x" , "1x-2x Holo" , rarity_t::rare , category_t::attachment } ,
			{ "optic_cq_holo" , "1x Holo" , rarity_t::common , category_t::attachment } ,
			{ "optic_rng_aog" , "2x-4x AOG" , rarity_t::rare , category_t::attachment } ,
			{ "optic_sni_dcom" , "6x Sniper" , rarity_t::rare , category_t::attachment } ,
			{ "optic_sni_var" , "4x-8x Sniper" , rarity_t::epic , category_t::attachment } ,
			{ "optic_sni_threat" , "4x-10x Digi" , rarity_t::legendary , category_t::attachment } ,

			{ "suppr_v2b" , "Barrel Stabilizer" , rarity_t::common , category_t::attachment } ,
			{ "lasersight" , "Laser Sight" , rarity_t::common , category_t::attachment } ,
			{ "mag_energy" , "Energy Mag" , rarity_t::common , category_t::attachment } ,
			{ "mag_sniper" , "Sniper Mag" , rarity_t::common , category_t::attachment } ,
			{ "mag_v1b" , "Light Mag" , rarity_t::common , category_t::attachment } ,
			{ "mag_v2b" , "Heavy Mag" , rarity_t::common , category_t::attachment } ,
			{ "mag_v3b" , "Shotgun Bolt" , rarity_t::common , category_t::attachment } ,
			{ "stock_folded_regular" , "Standard Stock" , rarity_t::common , category_t::attachment } ,
			{ "stock_folded_sniper" , "Sniper Stock" , rarity_t::common , category_t::attachment } ,
			{ "mods_chip" , "Hop-Up" , rarity_t::epic , category_t::attachment } ,

			{ "mp_weapon_rspn101" , "R-301" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_r97" , "R-99" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_vinson" , "Flatline" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_hemlok" , "Hemlok" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_energy_ar" , "HAVOC" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_alternator" , "Alternator" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_volt" , "Volt" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_car" , "C.A.R." , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_pdw" , "Prowler" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_autopistol" , "RE-45" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_semipistol" , "P2020" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_wingman" , "Wingman" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_shotgun_pistol" , "Mozambique" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_shotgun" , "EVA-8" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_energy_shotgun" , "Peacekeeper" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_mastiff" , "Mastiff" , rarity_t::legendary , category_t::weapon } ,
			{ "mp_weapon_lmg" , "Spitfire" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_esaw" , "Devotion" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_lstar" , "L-STAR" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_dragon_lmg" , "Rampage" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_g2" , "G7 Scout" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_3030" , "30-30" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_doubletake" , "Triple Take" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_dmr" , "Longbow" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_sentinel" , "Sentinel" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_defender" , "Charge Rifle" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_bow" , "Bocek" , rarity_t::common , category_t::weapon } ,
			{ "mp_weapon_sniper" , "Kraber" , rarity_t::legendary , category_t::weapon } ,
			{ "mp_weapon_nemesis" , "Nemesis" , rarity_t::common , category_t::weapon } ,
		};

		inline auto match_path( const std::string& path ) -> resolved_t
		{
			resolved_t out { };
			if ( path.empty( ) )
				return out;

			auto lower = path;
			for ( auto& c : lower )
				c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );

			std::size_t best_len = 0;
			for ( const auto& n : k_matches )
			{
				const auto len = std::strlen( n.token );
				if ( len <= best_len )
					continue;
				if ( lower.find( n.token ) == std::string::npos )
					continue;
				out.name = n.name;
				out.rarity = n.rarity;
				out.category = n.category;
				best_len = len;
			}

			if ( best_len == 0 )
			{
				out.category = category_t::unknown;
				return out;
			}

			if ( out.category == category_t::gear )
			{
				if ( lower.find( "_l4" ) != std::string::npos || lower.find( "level_4" ) != std::string::npos )
					out.rarity = rarity_t::legendary;
				else if ( lower.find( "_l3" ) != std::string::npos || lower.find( "level_3" ) != std::string::npos )
					out.rarity = rarity_t::epic;
				else if ( lower.find( "_l2" ) != std::string::npos || lower.find( "level_2" ) != std::string::npos )
					out.rarity = rarity_t::rare;
			}

			return out;
		}

		inline auto skin_rarity( int skin , rarity_t fallback ) -> rarity_t
		{
			switch ( skin )
			{
			case 1: return rarity_t::rare;
			case 2: return rarity_t::epic;
			case 3: return rarity_t::legendary;
			case 4: return rarity_t::heirloom;
			default: return fallback;
			}
		}

		inline auto clean_weapon( std::string raw ) -> std::string
		{
			if ( raw.empty( ) )
				return {};

			const char* prefixes[] = { "mp_weapon_" , "mp_ability_" , "weapon_" };
			for ( auto p : prefixes )
			{
				const auto n = std::strlen( p );
				if ( raw.size( ) > n && raw.compare( 0 , n , p ) == 0 )
				{
					raw = raw.substr( n );
					break;
				}
			}

			for ( auto& c : raw )
			{
				if ( c == '_' )
					c = ' ';
			}

			if ( !raw.empty( ) )
				raw[ 0 ] = static_cast<char>( std::toupper( static_cast<unsigned char>( raw[ 0 ] ) ) );

			return raw;
		}
	}

	inline auto weapon_name( int weapon_name_index ) -> std::string
	{
		if ( weapon_name_index <= 0 )
			return {};

		const auto table = hypervisor->m_base_address + offsets::weapon_names;
		auto s = detail::table_str( table , weapon_name_index , 0x8 );
		if ( s.empty( ) )
			s = detail::table_str( table , weapon_name_index , 0x10 );
		if ( s.empty( ) )
			s = detail::table_str( table , weapon_name_index , 0x18 );
		return detail::clean_weapon( std::move( s ) );
	}

	inline auto model_path( int model_index ) -> std::string
	{
		if ( model_index <= 0 )
			return {};

		const auto table = hypervisor->m_base_address + offsets::model_names;
		auto s = detail::table_str( table , model_index , 0x10 );
		if ( s.empty( ) )
			s = detail::table_str( table , model_index , 0x8 );
		if ( s.empty( ) )
			s = detail::table_str( table , model_index , 0x18 );
		return s;
	}

	inline auto resolve( int script_id , int weapon_name_index , int model_index , int skin ) -> resolved_t
	{
		if ( const auto* rec = survival::lookup( script_id ) )
		{
			resolved_t out { };
			out.name = rec->name;

			static constexpr category_t k_cats[] = {
				category_t::weapon ,
				category_t::ammo ,
				category_t::heal ,
				category_t::gear ,
				category_t::attachment ,
				category_t::grenade ,
				category_t::misc ,
			};
			out.category = rec->category < 7 ? k_cats[ rec->category ] : category_t::misc;

			switch ( rec->tier )
			{
			case 0:
			case 1: out.rarity = rarity_t::common; break;
			case 2: out.rarity = rarity_t::rare; break;
			case 3: out.rarity = rarity_t::epic; break;
			case 4: out.rarity = rarity_t::legendary; break;
			default: out.rarity = rarity_t::heirloom; break;
			}
			return out;
		}

		if ( auto wpn = weapon_name( weapon_name_index ); !wpn.empty( ) )
		{
			resolved_t out { };
			out.name = std::move( wpn );
			out.category = category_t::weapon;
			out.rarity = ( skin >= 3 ) ? rarity_t::legendary : rarity_t::common;
			return out;
		}

		auto out = detail::match_path( model_path( model_index ) );
		if ( out.category != category_t::unknown )
		{
			if ( out.category == category_t::gear || out.category == category_t::attachment )
				out.rarity = detail::skin_rarity( skin , out.rarity );
			return out;
		}

		if ( script_id > 0 )
		{
			char buf[ 32 ];
			std::snprintf( buf , sizeof( buf ) , "Item %d" , script_id );
			return { buf , detail::skin_rarity( skin , rarity_t::common ) , category_t::unknown };
		}

		return { "Loot" , rarity_t::common , category_t::unknown };
	}

	inline auto held_weapon( std::uint64_t weapon ) -> std::string
	{
		if ( !weapon || !hypervisor->is_valid( weapon ) )
			return {};

		auto parse_script = [ ]( const std::string& raw ) -> std::string
		{
			if ( raw.empty( ) )
				return {};

			auto from = detail::match_path( raw );
			if ( !from.name.empty( ) && from.category == category_t::weapon )
				return from.name;

			auto pretty = detail::clean_weapon( raw );
			if ( pretty.size( ) < 2 )
				return {};

			bool has_alpha = false;
			for ( char c : pretty )
			{
				if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) )
				{
					has_alpha = true;
					break;
				}
			}
			if ( !has_alpha )
				return {};

			if ( pretty.size( ) <= 8 )
			{
				auto lower = pretty;
				for ( auto& c : lower )
					c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
				if ( lower.find( ' ' ) == std::string::npos && lower.find( "weapon" ) != std::string::npos )
					return {};
			}

			return pretty;
		};

		{
			const auto name_ptr = hypervisor->read<std::uint64_t>( weapon + offsets::m_weapon_class_name );
			if ( auto s = parse_script( detail::read_str( name_ptr ) ); !s.empty( ) )
				return s;

			auto inline_name = hypervisor->read<std::array<char , 64>>( weapon + offsets::m_weapon_class_name );
			inline_name.back( ) = '\0';
			if ( auto s = parse_script( inline_name.data( ) ); !s.empty( ) )
				return s;
		}

		{
			const auto meta = weapon + offsets::weapon_settings_meta_base;
			const auto short_ptr = hypervisor->read<std::uint64_t>( meta + offsets::shortprintname );
			if ( auto s = detail::read_str( short_ptr ); !s.empty( ) )
				return s;

			const auto print_ptr = hypervisor->read<std::uint64_t>( meta + offsets::printname );
			if ( auto s = detail::read_str( print_ptr ); !s.empty( ) )
				return s;
		}

		{
			const auto sig_ptr = hypervisor->read<std::uint64_t>( weapon + offsets::m_i_signifier_name );
			if ( auto s = parse_script( detail::read_str( sig_ptr ) ); !s.empty( ) )
				return s;

			const int model_idx = hypervisor->read<int>( weapon + offsets::m_n_model_index );
			if ( auto s = parse_script( model_path( model_idx ) ); !s.empty( ) )
				return s;
		}

		return {};
	}
}
