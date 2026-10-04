#pragma once

#include <dependencies/includes.h>

namespace global
{
	namespace aimbot
	{
		inline bool enabled = { false };
		inline bool prediction = { true };
		inline bool visible_check = { true };
		inline bool snapline = { true };
		inline bool fov_circle = { true };
		inline float fov = { 100.0f };
		inline float smoothness = { 5.0f };
		inline int key = { VK_RBUTTON };
		inline int bone_mode = { 3 };
	}

	namespace esp
	{
		inline bool draw = { false };

		inline bool box = { false };
		inline int box_type = { 0 };
		inline float box_thickness = { 2.f };
		inline bool box_outline = { true };

		inline bool health_bar = { false };

		inline bool shield = { false };
		inline bool name = { false };
		inline bool weapon = { true };
		inline bool distance = { true };
		inline bool flags = { true };
		inline bool head_dot = { false };
		inline float head_dot_scale = { 1.f };
		inline bool skeleton = { false };
		inline float skeleton_thickness = { 1.5f };
		inline bool visible_check = { true };
		inline float occluded_alpha = { 0.35f };

		inline float color[ 4 ] = { 144.f / 255.f , 84.f / 255.f , 189.f / 255.f , 1.0f };
		inline float health_color[ 4 ] = { 0.2f , 0.9f , 0.3f , 1.0f };
		inline float shield_color[ 4 ] = { 0.3f , 0.6f , 1.0f , 1.0f };
	}

	namespace loot
	{
		inline bool enabled = { false };
		inline bool name = { true };
		inline bool distance = { true };
		inline bool death_box = { true };
		inline bool weapons = { true };
		inline bool ammo = { true };
		inline bool heals = { true };
		inline bool gear = { true };
		inline bool attachments = { true };
		inline bool grenades = { true };
		inline bool misc = { true };
		inline float max_distance = { 150.f };
		inline int min_rarity = { 0 };
	}

	namespace heirloom
	{
		inline bool enabled = { false };
		inline bool debug = { true };
		inline int selection = { 0 };
	}

	namespace misc
	{
		inline bool keybind_list = { true };
		inline bool keybind_list_active_only = { false };
	}
}
