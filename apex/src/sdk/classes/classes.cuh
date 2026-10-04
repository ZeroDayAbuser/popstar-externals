#pragma once

#include <dependencies/includes.h>

namespace classes
{
	class c_entry;
	class c_entity;

	class c_entry
	{
	public:
		std::uint64_t m_address = { 0 };
		std::vector<c_entry> m_entry {};
		data::_vector3 m_vel {};

		auto get( ) -> vi;
	};

	class c_entity : public classes::c_entry
	{
	public:
		int m_health { };
		int m_max_health { };
		int m_shield {};
		int m_max_shield { };
		std::int8_t m_team { };
		data::_vector3 m_origin { };
		std::string m_name {};
		std::string m_weapon {};
		bool m_visible { false };
		bool m_is_dummy { false };
		bool m_knocked { false };
		int m_flags { 0 };
		std::uint32_t m_ground_ent { 0 };
		bool m_anim_jumping { false };
		bool m_anim_landing { false };
		bool m_anim_in_air_walk { false };
		bool m_fast_falling { false };
		bool m_sticky_sprint { false };
		bool m_sliding { false };
		bool m_has_jumped { false };
		float m_fall_velocity { 0.f };
		float m_sprint_tilt { 0.f };
		float m_pose_move { 0.f };
		float m_origin_dh { 0.f };

		auto localplayer( ) -> classes::c_entity;
		auto get( int ix ) -> classes::c_entity;
		auto team( ) -> int;
		auto pos( ) -> data::_vector2;
		auto bone_pos( data::e_bone_type bone ) -> data::_vector3;
		auto player_names( ) -> std::string;
		auto camera_origin( ) const -> data::_vector3;
		auto view_angles( ) const -> data::_vector3;
		auto control_rotation( ) const -> data::_vector3;
		auto active_weapon( ) -> std::uint64_t;
		auto projectile_info( ) -> data::weapon_info_t;
		auto zoom_fov( ) -> float;
		auto view_fov( ) -> float;
		auto is_zooming( ) -> bool;
		auto last_visible_time( ) -> float;
		auto is_visible( float time_base ) -> bool;

	};
}