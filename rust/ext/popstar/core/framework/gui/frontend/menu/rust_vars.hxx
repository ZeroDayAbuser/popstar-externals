#pragma once

#include <array>
#include <string>

#include <core/framework/gui/backend/math/math.hxx>
#include <core/framework/gui/backend/manager/keybinds/keybinds.hxx>

namespace rust_menu
{
	struct vars_t
	{
		struct aimbot_t
		{
			bool enabled = false;
			bool silent = false;
			int silent_hitchance = 100;
			bool smart_target = false;
			bool target_360 = false;
			bool auto_shoot = false;
			bool prediction = true;
			float smoothing = 2.f;
			int max_distance = 350;
			int fov_size = 125;
			bool fov_circle = true;
			bool skip_teammates = true;
			bool skip_scientists = true;
			bool skip_dwellers = true;
			bool skip_sleepers = true;
			bool skip_wounded = true;
			bool skip_animals = true;
			int target_bone = 0;
			float prediction_strength = 1.f;
			float prediction_drop = 1.f;
			int prediction_ping = 80;
			core::gui::key_var_t aim_key {};

			bool automatic_weapons = false;
			bool instant_bow = false;
			bool instant_eoka = false;
			int eoka_chance = 100;
			bool rapid_fire = false;
			bool no_sway = false;
			bool no_animation = false;
			bool thick_bullet = false;
			bool hitbox_override = false;
			int hitbox_bone = 0;
			bool fast_shoot = false;
			bool hit_material = false;
			bool override_spread = false;
			int spread_amount = 0;
			bool override_recoil = false;
			int recoil_amount = 0;
			bool override_melee = false;
			int melee_amount = 100;
			bool no_viewmodel_lower = false;
			bool hide_viewmodel = false;
		} aimbot;

		struct visuals_t
		{
			bool players_enabled = true;
			bool name = true;
			bool steam_avatar = false;
			bool box = false;
			bool skeleton = true;
			bool view_line = false;
			bool snap_lines = false;
			bool oof_arrows = false;
			bool arrow_glow = false;
			bool item_name = false;
			bool item_icon = false;
			bool player_inventory = false;
			bool vischeck = false;
			bool radar = false;
			bool radar_players_only = false;
			float radar_size = 120.f;
			float radar_range = 150.f;
			bool crosshair = false;
			bool crosshair_spin = false;
			bool show_on_radar = false;
			int flags = 0;
			bool flag_distance = true;
			bool flag_sleeper = false;
			bool flag_wounded = false;
			bool flag_team = false;
			bool flag_ads = false;
			core::gui::c_color name_col { 255, 255, 255 };
			core::gui::c_color box_col { 255, 255, 255 };
			core::gui::c_color skeleton_col { 255, 255, 255 };
			core::gui::c_color view_col { 255, 255, 255 };
			core::gui::c_color snap_col { 255, 255, 255 };
			core::gui::c_color arrow_col { 255, 255, 255 };
			core::gui::c_color item_col { 255, 255, 255 };
			core::gui::c_color crosshair_col { 255, 85, 200 };

			struct player_esp_t
			{
				bool enabled = true;
				bool name = true;
				bool steam_avatar = false;
				bool box = false;
				bool skeleton = true;
				bool view_line = false;
				bool snap_lines = false;
				bool oof_arrows = false;
				bool item_name = false;
				bool item_icon = false;
				core::gui::c_color name_col { 255, 255, 255 };
				core::gui::c_color box_col { 90, 220, 140 };
				core::gui::c_color skeleton_col { 90, 220, 140 };
			} team;

			bool entities_enabled = true;
			bool ores = false;
			bool crates = false;
			bool deployables = false;
			bool traps = false;
			bool npcs = false;
			bool monuments = false;
			bool vehicles = false;
			bool dropped_weapons = false;
			bool dropped_items = false;
			core::gui::c_color ores_col { 220, 200, 140 };
			core::gui::c_color crates_col { 255, 180, 80 };
			core::gui::c_color npc_col { 255, 90, 90 };
			core::gui::c_color vehicle_col { 90, 200, 255 };
		} visuals;

		struct misc_t
		{
			bool fast_loot = false;
			bool instant_untie = false;
			bool instant_interactions = false;
			bool instant_revive = false;
			core::gui::key_var_t instant_revive_key {};

			bool spider_man = false;
			core::gui::key_var_t spider_key {};
			bool fly = false;
			core::gui::key_var_t fly_key {};
			bool silent_walk = false;
			bool no_fall = false;
			bool speed_hack = false;
			bool omni_sprint = false;
			bool remove_water_drag = false;
			bool walk_on_water = false;
			bool anti_aim = false;

			bool custom_fov = false;
			float field_of_view = 90.f;
			float zoom_fov = 35.f;
			core::gui::key_var_t zoom_key {};
			bool third_person = false;
			bool bullet_tracers = false;
			core::gui::c_color tracer_col { 255, 200, 80 };
			bool debug_camera = false;
			float debug_cam_speed = 10.f;
			float mouse_sens = 40.f;
			core::gui::key_var_t debug_cam_key {};
			bool layer_toggle = false;
			core::gui::key_var_t layer_key {};
			bool hide_construction = false;
			bool hide_transparent = false;
			bool hide_debris = false;
			bool hide_terrain = false;
			bool hide_tree = false;
			bool hide_water = false;

			bool time_changer = false;
			float time = 18.f;
			bool bright_night = false;
			bool world_mod = false;
		} misc;

		struct settings_t
		{
			bool vsync = false;
			bool override_accent = false;
			core::gui::c_color accent { 148, 112, 255 };
			bool enable_memory_writes = false;
			std::string config_name = "default";
			int config_index = 0;
		} settings;
	};

	inline vars_t g {};
}
