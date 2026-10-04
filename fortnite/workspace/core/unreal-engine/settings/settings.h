#pragma once
#include <imgui_settings.h>


namespace g_triggerbot {
	inline bool enable = false;
}

namespace g_aimbot {
	inline bool enable = true;
	inline bool controller_support = false;
	inline int fov = 150;
	inline int aimbot_type = 0;
	inline float max_distance = 300.0f;
	inline float smoothing = 5.0f;
	inline CustomWidgets::Keybind aimbot_key;
	inline CustomWidgets::Keybind secondary_aimbot_key;
	inline int hitbox_type;
	inline bool visible_check = false;
	inline bool show_fov = false;
}

namespace g_loot {
	inline bool enable = false;
	inline bool containers = false;
	inline bool show_distance = true;
	inline float max_distance = 75.0f;

}

namespace g_exploits {
	inline bool chams = false;
}

namespace g_settings {
	inline bool vsync = true;
	inline bool box = false;
	inline float esp_max_distance = 300.0f;
	inline bool wins = false;
	inline bool rank = false;
	inline bool china_hat = false;
	inline bool platform = false;
	inline bool reload_bar = false;
	inline bool kills = false;
	inline bool level = false;
	inline bool player_name = false;
	inline bool distance = false;
	inline bool skeleton = false;
	inline bool weapon = false;

	inline int line_esp = 0;
	inline bool fov_arrows = false;


	inline bool crosshair_enabled = false;
	inline float crosshair_color [ 4 ] = { 1.f, 1.f, 1.f, 1.f };
	inline float crosshair_radius = 12.f;
	inline float crosshair_thickness = 1.f;
	inline int crosshair_segments = 12;
	inline bool crosshair_outline = true;

	inline bool radar_enabled = false;
	inline int radar_pos_x = 100;
	inline int radar_pos_y = 100;
	inline int radar_size = 250;
	inline int radar_range = 150;
	inline bool radar_grid = true;
	inline int radar_grid_divisions = 4;
	inline float radar_opacity = 200.0f;

	inline float color_visible [ 4 ] = { 140.0f / 255.0f, 168.0f / 255.0f, 1.0f, 1.0f };
	inline float color_hidden [ 4 ] = { 1.0f, 138.0f / 255.0f, 93.0f / 255.0f, 1.0f };
	inline float color_downed [ 4 ] = { 75.0f / 255.0f, 87.0f / 255.0f, 219.0f / 255.0f, 1.0f };
	inline float color_teammate [ 4 ] = { 93.0f / 255.0f, 225.0f / 255.0f, 1.0f, 1.0f };
	inline float color_outline [ 4 ] = { 0.0f, 0.0f, 0.0f, 1.0f };
	inline float color_text [ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
	inline float color_box_visible [ 4 ] = { 140.0f / 255.0f, 168.0f / 255.0f, 1.0f, 1.0f };
	inline float color_box_hidden [ 4 ] = { 1.0f, 138.0f / 255.0f, 93.0f / 255.0f, 1.0f };
	inline float color_skeleton_visible [ 4 ] = { 140.0f / 255.0f, 168.0f / 255.0f, 1.0f, 1.0f };
	inline float color_skeleton_hidden [ 4 ] = { 1.0f, 138.0f / 255.0f, 93.0f / 255.0f, 1.0f };
	inline float color_china_hat_visible [ 4 ] = { 140.0f / 255.0f, 168.0f / 255.0f, 1.0f, 1.0f };
	inline float color_china_hat_hidden [ 4 ] = { 1.0f, 138.0f / 255.0f, 93.0f / 255.0f, 1.0f };
	inline float color_rank_text_visible [ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
	inline float color_rank_text_hidden [ 4 ] = { 0.8f, 0.8f, 0.8f, 1.0f };
	inline float color_platform_text_visible [ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
	inline float color_platform_text_hidden [ 4 ] = { 0.8f, 0.8f, 0.8f, 1.0f };
	inline float color_name_text_visible [ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
	inline float color_name_text_hidden [ 4 ] = { 0.8f, 0.8f, 0.8f, 1.0f };
	inline float color_fov_arrow_visible [ 4 ] = { 140.0f / 255.0f, 168.0f / 255.0f, 1.0f, 1.0f };
	inline float color_fov_arrow_hidden [ 4 ] = { 1.0f, 138.0f / 255.0f, 93.0f / 255.0f, 1.0f };
}
