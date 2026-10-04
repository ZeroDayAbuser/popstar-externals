#pragma once

#include <cmath>
#include <memory>
#include <vector>

#include <core/framework/gui/frontend/widgets/classes/window.hxx>
#include <core/framework/gui/frontend/widgets/classes/context.hxx>
#include <core/framework/gui/backend/animations/animations.hxx>
#include <core/framework/gui/backend/manager/keybinds/keybinds.hxx>
#include <core/framework/gui/backend/render/render.hxx>
#include <core/framework/gui/backend/math/math.hxx>
#include <settings/settings.hpp>
#include <sdk/globals.hpp>
#include <popstar_glue.hpp>

namespace core::gui
{
	class c_menu
	{
	public:
		void initialize( )
		{
			if ( m_initialized )
				return;

			std::shared_ptr<c_window> main = std::make_shared<c_window>( "menu", c_vector_2d( 80.f, 60.f ), c_vector_2d( 620.f, 460.f ) );
			auto& A = settings.aimbot;
			auto& P = settings.visuals.players;
			auto& V = settings.visuals;
			auto& M = settings.misc;
			auto& L = settings.visuals.local;
			auto& W = settings.visuals.world;
			auto& U = settings.settings.ui;
			auto& C = settings.settings.configs;

			{
				auto child = main->build_child( "AIM", child_width::half, 0.f, [&]( c_child* child )
				{
					child->add_checkbox( "Memory aim", &A.general.main.enabled )->attach_popup( "Memory aim", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_slider<float>( "Smoothing", &A.general.humanization.smoothing, 0.f, 10.f );
							sec->add_slider<int>( "Max distance", &A.general.main.max_distance, 0, 1000 );
						} );
					} );
					child->add_checkbox( "Silent aim", &A.general.main.silent_aim )->attach_popup( "Silent aim", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_slider<int>( "Hit chance", &A.general.main.silent_aim_hit_chance, 1, 100 );
						} );
					} );
					child->add_dropdown( "Target bone", &A.general.main.target_bone, { "Head", "Neck", "Chest", "Stomach", "Pelvis", "Knee" } );
					child->add_checkbox( "Smart target", &A.general.main.smart_target );
					child->add_checkbox( "Target 360", &A.general.main.target_360 );
					child->add_checkbox( "Auto shoot", &A.general.main.auto_shoot )->attach_popup( "Auto shoot", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_checkbox( "Also enable Target 360", &A.general.main.target_360 );
							sec->add_checkbox( "Silent aim", &A.general.main.silent_aim );
						} );
					} );
					child->add_checkbox( "Prediction", &A.general.main.prediction )->attach_popup( "Prediction", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_slider<float>( "Lead", &A.general.main.prediction_strength, 0.f, 2.f );
							sec->add_slider<float>( "Drop", &A.general.main.prediction_drop, 0.f, 2.f );
							sec->add_slider<int>( "Ping", &A.general.main.prediction_ping_comp, 0, 250 );
						} );
					} );
					child->add_checkbox( "Prediction path", &A.visualization.prediction_path )->attach_popup( "Path", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &A.visualization.prediction_path_color );
						} );
					} );
					child->add_checkbox( "FOV circle", &A.visualization.fov_circle )->attach_popup( "FOV", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_slider<int>( "Size", &A.visualization.fov_circle_size, 25, 500 );
							sec->add_colorpicker( "Color", &A.visualization.fov_circle_color );
							sec->add_checkbox( "Target line", &A.visualization.fov_circle_target_line );
							sec->add_colorpicker( "Line color", &A.visualization.fov_circle_target_line_color );
						} );
					} );
					child->add_checkbox( "Filters", &A.general.prerequisites.scale_by_distance )->attach_popup( "Filters", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_checkbox( "Skip teammates", &A.general.filters.skip_teammates );
							sec->add_checkbox( "Skip scientists", &A.general.filters.skip_scientists );
							sec->add_checkbox( "Skip dwellers", &A.general.filters.skip_dwellers );
							sec->add_checkbox( "Skip sleepers", &A.general.filters.skip_sleepers );
							sec->add_checkbox( "Skip wounded", &A.general.filters.skip_wounded );
							sec->add_checkbox( "Skip animals", &A.general.filters.skip_animals );
						} );
					} );
					child->add_keybind( "Aim key", &A.aim_key );
				} );
				child->attach_child( "Aimbot", "General" );
			}

			{
				static const char* kGroupNames[7] = { "Rifles", "Snipers", "Shotguns", "Pistols", "Bows", "LMGs", "SMGs" };
				auto child = main->build_child( "GROUPS", child_width::half, 0.f, [&]( c_child* child )
				{
					for ( int gi = 0; gi < 7; ++gi )
					{
						auto& G = A.groups[gi];
						child->add_checkbox( kGroupNames[gi], &G.main.enabled )->attach_popup( kGroupNames[gi], [&A, gi]( c_popup* pop )
						{
							auto& G = A.groups[gi];
							pop->section( [&G]( c_popup_section* sec )
							{
								sec->add_slider<int>( "Max distance", &G.main.max_distance, 0, 1000 );
								sec->add_checkbox( "Silent aim", &G.main.silent_aim );
								sec->add_slider<int>( "Hit chance", &G.main.silent_aim_hit_chance, 1, 100 );
								sec->add_dropdown( "Target bone", &G.main.target_bone, { "Head", "Neck", "Chest", "Stomach", "Pelvis", "Knee" } );
								sec->add_checkbox( "Smart target", &G.main.smart_target );
								sec->add_checkbox( "Target 360", &G.main.target_360 );
								sec->add_checkbox( "Auto shoot", &G.main.auto_shoot );
								sec->add_checkbox( "Prediction", &G.main.prediction );
								sec->add_slider<float>( "Lead", &G.main.prediction_strength, 0.f, 2.f );
								sec->add_slider<float>( "Drop", &G.main.prediction_drop, 0.f, 2.f );
								sec->add_slider<int>( "Ping", &G.main.prediction_ping_comp, 0, 250 );
								sec->add_slider<float>( "Smoothing", &G.humanization.smoothing, 0.f, 10.f );
								sec->add_checkbox( "Skip teammates", &G.filters.skip_teammates );
								sec->add_checkbox( "Skip scientists", &G.filters.skip_scientists );
								sec->add_checkbox( "Skip dwellers", &G.filters.skip_dwellers );
								sec->add_checkbox( "Skip sleepers", &G.filters.skip_sleepers );
								sec->add_checkbox( "Skip wounded", &G.filters.skip_wounded );
								sec->add_checkbox( "Skip animals", &G.filters.skip_animals );
							} );
						} );
					}
				} );
				child->attach_child( "Aimbot", "General" );
			}

			{
				auto child = main->build_child( "WEAPONS", child_width::half, 0.f, [&]( c_child* child )
				{
					child->add_checkbox( "Automatic weapons", &A.weapons.automatic_weapons );
					child->add_checkbox( "Instant bow", &A.weapons.instant_bow );
					child->add_checkbox( "Instant eoka", &A.weapons.instant_eoka )->attach_popup( "Eoka", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_slider<int>( "Strike chance", &A.weapons.instant_eoka_strikechance, 1, 100 );
						} );
					} );
					child->add_checkbox( "Rapid fire", &A.weapons.rapid_fire );
					child->add_checkbox( "No sway", &A.weapons.no_sway );
					child->add_checkbox( "No animation", &A.weapons.no_animation );
					child->add_checkbox( "Fast shoot", &A.weapons.fast_shoot );
					child->add_checkbox( "Hide viewmodel", &A.weapons.hide_viewmodel );
				} );
				child->attach_child( "Aimbot", "Combat" );
			}

			{
				auto child = main->build_child( "OVERRIDES", child_width::half, 0.f, [&]( c_child* child )
				{
					child->add_checkbox( "Weapon spread", &A.weapons.override_weapon_spread )->attach_popup( "Spread", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_slider<int>( "Amount", &A.weapons.override_weapon_spread_amount, 0, 100 );
						} );
					} );
					child->add_checkbox( "Weapon recoil", &A.weapons.override_weapon_recoil )->attach_popup( "Recoil", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_slider<int>( "Amount", &A.weapons.override_weapon_recoil_amount, 0, 100 );
						} );
					} );
					child->add_checkbox( "Melee range", &A.weapons.override_melee_range )->attach_popup( "Melee", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_slider<int>( "Amount", &A.weapons.override_melee_range_amount, 100, 200 );
						} );
					} );
					child->add_checkbox( "Thick bullet", &A.weapons.thick_bullet );
					child->add_checkbox( "Hitbox override", &A.weapons.hitbox_override )->attach_popup( "Hitbox", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_dropdown( "Bone", &A.weapons.hitbox_override_bone, { "Head", "Neck", "Chest", "Stomach", "Pelvis", "Knee" } );
						} );
					} );
					child->add_checkbox( "Hit material", &A.weapons.hit_material_override );
					child->add_checkbox( "No viewmodel lower", &A.weapons.no_viewmodel_lower );
				} );
				child->attach_child( "Aimbot", "Combat" );
			}

			{
				auto child = main->build_child( "PLAYERS", child_width::half, 0.f, [&]( c_child* child )
				{
					child->add_checkbox( "Enabled", &P.enabled )->attach_popup( "Players", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_slider<int>( "Max distance", &P.max_distance, 0, 1000 );
						} );
					} );
					child->add_checkbox( "Show sleepers", &P.show_sleepers );
					child->add_checkbox( "Vischeck", &P.vischeck )->attach_popup( "Vischeck", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_checkbox( "Visible color", &P.vis_visible_color_enabled );
							sec->add_colorpicker( "Visible", &P.vis_visible_color );
							sec->add_checkbox( "Invisible color", &P.vis_invisible_color_enabled );
							sec->add_colorpicker( "Invisible", &P.vis_invisible_color );
							sec->add_checkbox( "Dim instead of hide", &P.vis_dim_instead_of_hide );
							sec->add_checkbox( "Per-bone skeleton", &P.vis_per_bone_skeleton );
						} );
					} );
					child->add_checkbox( "Name", &P.name )->attach_popup( "Name", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &P.name_color );
						} );
					} );
					child->add_checkbox( "Steam avatar", &P.name_show_avatar );
					child->add_checkbox( "Bounding box", &P.bounding_box )->attach_popup( "Box", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &P.bounding_box_color );
							sec->add_checkbox( "Gradient", &P.bounding_box_gradient );
							sec->add_colorpicker( "Gradient color", &P.bounding_box_gradient_color );
							sec->add_checkbox( "Fill", &P.bounding_box_fill );
							sec->add_colorpicker( "Fill top", &P.bounding_box_fill_color_top );
							sec->add_colorpicker( "Fill bottom", &P.bounding_box_fill_color_bottom );
						} );
					} );
					child->add_checkbox( "Skeleton", &P.skeleton )->attach_popup( "Skeleton", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &P.skeleton_color );
							sec->add_checkbox( "Rounded", &P.skeleton_rounded );
							sec->add_checkbox( "Vis colors", &P.skeleton_vis_colors );
						} );
					} );
					child->add_checkbox( "Head circle", &P.head_circle )->attach_popup( "Head", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &P.head_circle_color );
						} );
					} );
					child->add_checkbox( "View line", &P.view_line )->attach_popup( "View line", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &P.view_line_color );
						} );
					} );
					child->add_checkbox( "Snap lines", &P.snap_lines )->attach_popup( "Snap", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &P.snap_lines_color );
							sec->add_dropdown( "Alignment", &P.snap_lines_alignment, { "Top", "Middle", "Bottom" } );
						} );
					} );
					child->add_dropdown( "Chams", &P.chams, g_material_names );
					child->add_checkbox( "Show on radar", &P.include_in_radar );
				} );
				child->attach_child( "Visuals", "", "Enemies" );
			}

			{
				auto child = main->build_child( "OVERLAY", child_width::half, 0.f, [&]( c_child* child )
				{
					child->add_checkbox( "OOF arrows", &P.out_of_view_arrows )->attach_popup( "Arrows", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &P.out_of_view_arrows_color );
							sec->add_checkbox( "Glow", &P.arrow_glow );
							sec->add_slider<float>( "Size", &P.out_of_view_arrows_size, 0.f, 30.f );
							sec->add_slider<float>( "Radius", &P.out_of_view_arrows_radius, 0.f, 2.f );
						} );
					} );
					child->add_checkbox( "Radar", &V.radar.enabled )->attach_popup( "Radar", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_checkbox( "Players only", &V.radar.players_only );
							sec->add_slider<float>( "Size", &V.radar.size, 60.f, 300.f );
							sec->add_slider<float>( "Range", &V.radar.range, 50.f, 500.f );
						} );
					} );
					child->add_checkbox( "Crosshair", &V.screen.crosshair )->attach_popup( "Crosshair", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &V.screen.crosshair_color );
							sec->add_checkbox( "Spinning triangle", &V.screen.crosshair_spin );
						} );
					} );
					child->add_checkbox( "Player inventory", &V.screen.player_inventory );
					child->add_checkbox( "Item name", &P.item_name )->attach_popup( "Item name", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &P.item_name_color );
						} );
					} );
					child->add_checkbox( "Item icon", &P.item_icon )->attach_popup( "Item icon", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &P.item_icon_color );
						} );
					} );
				} );
				child->attach_child( "Visuals", "", "Enemies" );
			}

			{
				auto child = main->build_child( "ENTITIES", child_width::half, 0.f, [&]( c_child* child )
				{
					child->add_checkbox( "Master", &V.entities.enabled );
					child->add_checkbox( "Ores & collectibles", &V.entities.ores_collectibles.enabled );
					child->add_checkbox( "Crates & barrels", &V.entities.crates_barrels.enabled );
					child->add_checkbox( "Deployables", &V.entities.deployables.enabled );
					child->add_checkbox( "Traps & turrets", &V.entities.traps_turrets.enabled );
					child->add_checkbox( "NPCs & animals", &V.entities.npcs_animals.enabled );
					child->add_colorpicker( "Scientist color", &V.entities.npcs_animals.scientist.color );
				} );
				child->attach_child( "Visuals", "", "World" );
			}

			{
				auto child = main->build_child( "WORLD ESP", child_width::half, 0.f, [&]( c_child* child )
				{
					child->add_checkbox( "Monuments", &V.entities.static_monuments.enabled );
					child->add_checkbox( "Vehicles", &V.entities.vehicles.enabled );
					child->add_colorpicker( "Minicopter color", &V.entities.vehicles.minicopter.color );
					child->add_checkbox( "Dropped weapons", &V.entities.dropped_weapons.enabled );
					child->add_checkbox( "Dropped items", &V.entities.dropped_items.enabled );
					child->add_colorpicker( "Stone color", &V.entities.ores_collectibles.stone.color );
					child->add_colorpicker( "Elite crate color", &V.entities.crates_barrels.elite_crate.color );
				} );
				child->attach_child( "Visuals", "", "World" );
			}

			{
				auto child = main->build_child( "WORLD", child_width::half, 0.f, [&]( c_child* child )
				{
					child->add_checkbox( "Time changer", &W.time_changer )->attach_popup( "Time", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_slider<float>( "Hour", &W.time, 0.f, 24.f );
						} );
					} );
					child->add_checkbox( "Bright night", &W.bright_night );
					child->add_checkbox( "World modulation", &W.full_world_modulation )->attach_popup( "Modulate", [&]( c_popup* pop )
					{
						static const char* kLabels[7] = { "Sun", "Lights", "Rays", "Sky", "Clouds", "Fog", "Ambience" };
						pop->section( [&]( c_popup_section* sec )
						{
							for ( int i = 0; i < 7; ++i )
							{
								auto& m = W.modulates[i];
								sec->add_checkbox( kLabels[i], &m.enabled );
								sec->add_colorpicker( kLabels[i], &m.color );
							}
						} );
					} );
				} );
				child->attach_child( "Visuals", "", "World" );
			}

			{
				auto child = main->build_child( "MOVEMENT", child_width::half, 0.f, [&]( c_child* child )
				{
					child->add_checkbox( "Spiderman", &M.movement.spider_man )->attach_popup( "Spider", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_keybind( "Key", &M.movement.spider_man_key );
						} );
					} );
					child->add_checkbox( "Flyhack", &M.movement.fly )->attach_popup( "Fly", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_keybind( "Key", &M.movement.fly_key );
						} );
					} );
					child->add_checkbox( "Silent walk", &M.movement.silent_walk );
					child->add_checkbox( "No fall", &M.movement.no_fall );
					child->add_checkbox( "Speed hack", &M.movement.speed_hack );
					child->add_checkbox( "Omni sprint", &M.movement.omni_sprint );
					child->add_checkbox( "Walk on water", &M.movement.walk_on_water );
					child->add_checkbox( "Remove water drag", &M.movement.remove_water_drag );
					child->add_checkbox( "Anti aim", &M.movement.anti_aim );
				} );
				child->attach_child( "Misc", "Movement" );
			}

			{
				auto child = main->build_child( "GENERAL", child_width::half, 0.f, [&]( c_child* child )
				{
					child->add_checkbox( "Fast loot", &M.general.fast_loot );
					child->add_checkbox( "Quick untie", &M.general.instant_untie_crate );
					child->add_checkbox( "Instant interactions", &M.general.instant_interactions );
					child->add_checkbox( "Instant revive", &M.general.instant_revive )->attach_popup( "Revive", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_keybind( "Key", &M.general.instant_revive_key );
						} );
					} );
				} );
				child->attach_child( "Misc", "Movement" );
			}

			{
				auto child = main->build_child( "CAMERA", child_width::half, 0.f, [&]( c_child* child )
				{
					child->add_checkbox( "FOV changer", &L.custom_fov )->attach_popup( "FOV", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_slider<float>( "Field of view", &L.field_of_view, 60.f, 120.f );
							sec->add_slider<float>( "Zoom FOV", &L.zoom_fov, 20.f, 90.f );
							sec->add_keybind( "Zoom key", &L.zoom_key );
						} );
					} );
					child->add_checkbox( "Third person", &L.third_person );
					child->add_dropdown( "Weapon chams", &L.weapon_chams, g_material_names );
					child->add_dropdown( "Arm chams", &L.arm_chams, g_material_names );
					child->add_checkbox( "Bullet tracers", &L.bullet_tracers )->attach_popup( "Tracers", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &L.bullet_tracers_color );
						} );
					} );
					child->add_checkbox( "Debug camera", &L.debug_camera )->attach_popup( "Debug cam", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_keybind( "Toggle key", &L.debug_camera_key );
							sec->add_slider<float>( "Move speed", &L.debug_camera_speed, 1.f, 50.f );
							sec->add_slider<float>( "Sensitivity", &L.mouse_sensitivity, 10.f, 200.f );
						} );
					} );
					child->add_checkbox( "Layer toggle", &M.layer_toggle.enabled )->attach_popup( "Layers", [&]( c_popup* pop )
					{
						pop->section( [&]( c_popup_section* sec )
						{
							sec->add_keybind( "Toggle key", &M.layer_toggle.key );
							sec->add_checkbox( "Construction", &M.layer_toggle.hide_construction );
							sec->add_checkbox( "Transparent", &M.layer_toggle.hide_transparent );
							sec->add_checkbox( "Debris", &M.layer_toggle.hide_debris );
							sec->add_checkbox( "Default", &M.layer_toggle.hide_default );
							sec->add_checkbox( "Deployed", &M.layer_toggle.hide_deployed );
							sec->add_checkbox( "Ragdoll", &M.layer_toggle.hide_ragdoll );
							sec->add_checkbox( "Terrain", &M.layer_toggle.hide_terrain );
							sec->add_checkbox( "Tree", &M.layer_toggle.hide_tree );
							sec->add_checkbox( "World", &M.layer_toggle.hide_world );
							sec->add_checkbox( "Water", &M.layer_toggle.hide_water );
							sec->add_checkbox( "Clutter", &M.layer_toggle.hide_clutter );
						} );
					} );
				} );
				child->attach_child( "Visuals", "", "Local" );
			}

			{
				auto child = main->build_child( "UI", child_width::half, 0.f, [&]( c_child* child )
				{
					child->add_checkbox( "Vertical sync", &U.vertical_sync );
					child->add_checkbox( "Override accent", &U.override_accent );
					child->add_colorpicker( "Accent", &U.accent_color );
					child->add_checkbox( "Enable memory writes", &U.enable_memory_writes );
				} );
				child->attach_child( "Settings", "" );
			}

			{
				auto child = main->build_child( "CONFIG", child_width::half, 0.f, [&]( c_child* child )
				{
					child->add_input_box( "Config name", &C.new_config_name );
					if ( auto dd = child->add_dropdown( "Configs", &C.selected_index, C.file_list ) )
						dd->bind_items( &C.file_list );
					child->add_button( "Config", [] { settings_io::CreateNewFile( settings.settings.configs.new_config_name ); }, "Create" );
					child->add_button( "Config", [] {
						if ( settings.settings.configs.selected_index >= 0 &&
							settings.settings.configs.selected_index < static_cast<int>( settings.settings.configs.file_list.size( ) ) )
							settings_io::SaveByName( settings.settings.configs.file_list.at( settings.settings.configs.selected_index ) );
					}, "Save" );
					child->add_button( "Config", [] {
						if ( settings.settings.configs.selected_index >= 0 &&
							settings.settings.configs.selected_index < static_cast<int>( settings.settings.configs.file_list.size( ) ) )
							settings_io::LoadByName( settings.settings.configs.file_list.at( settings.settings.configs.selected_index ) );
					}, "Load" );
					child->add_button( "Config", [] { settings_io::Reset( ); }, "Reset" );
				} );
				child->attach_child( "Settings", "" );
			}

			m_windows.push_back( main );
			if ( g_ctx )
			{
				g_ctx->m_open = true;
				g_ctx->m_open_anim = 1.f;
			}
			m_initialized = true;
		}

		void runtime( )
		{
			if ( !m_initialized )
				return;

			popstar_glue::pull( );

			if ( g_input )
				g_input->update( );

			if ( g_ctx && g_input && g_input->key_pressed( VK_INSERT ) )
				g_ctx->m_open = !g_ctx->m_open;

			if ( g_ctx )
			{
				const float target = g_ctx->m_open ? 1.f : 0.f;
				const float dt = ImGui::GetIO( ).DeltaTime;
				g_ctx->m_open_anim += ( target - g_ctx->m_open_anim ) * ( 1.f - std::exp( -4.2f * dt ) );
				if ( std::fabs( g_ctx->m_open_anim - target ) < 0.0005f )
					g_ctx->m_open_anim = target;
			}

			if ( g_keybinds )
				g_keybinds->update_all( );
			if ( g_bind_hub )
				g_bind_hub->update( );

			if ( g_animator )
				g_animator->update( );

			animations::tick_presets( );

			if ( !g_ctx || g_ctx->m_open_anim < 0.001f )
				return;

			if ( g_render )
			{
				g_render->setup( );
				g_render->begin_layers( );
			}

			for ( const std::shared_ptr<c_window>& window : m_windows )
			{
				if ( !window )
					continue;
				window->input( );
				window->paint( );
			}

			if ( g_render )
				g_render->end_layers( );

			popstar_glue::push( );
		}

		std::vector<std::shared_ptr<c_window>>& windows( )
		{
			return m_windows;
		}

	private:
		std::vector<std::shared_ptr<c_window>> m_windows {};
		bool m_initialized { false };
	};

	inline std::shared_ptr<c_menu> g_menu = std::make_shared<c_menu>( );
}
