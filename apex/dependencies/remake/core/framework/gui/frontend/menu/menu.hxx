#pragma once

#include <cmath>
#include <memory>
#include <vector>
#include <string>

#include <core/framework/gui/frontend/widgets/classes/window.hxx>
#include <core/framework/gui/frontend/widgets/classes/context.hxx>
#include <core/framework/gui/frontend/widgets/classes/keybind_list.hxx>
#include <core/framework/gui/backend/animations/animations.hxx>
#include <core/framework/gui/backend/manager/keybinds/keybinds.hxx>
#include <core/framework/gui/backend/render/render.hxx>
#include <core/framework/gui/backend/math/math.hxx>
#include <src/utility/global/global.cuh>
#include <src/cheat/heirloom/heirloom.cuh>

namespace core::gui
{
	namespace binds
	{
		inline c_color esp_col { 144, 84, 189 };
		inline c_color hp_col { 51, 230, 77 };
		inline c_color sh_col { 77, 153, 255 };
		inline bool esp_col_on { true };
		inline bool hp_col_on { true };
		inline bool sh_col_on { true };
		inline key_var_t aim_key { VK_RBUTTON, key_mode_t::hold };

		inline auto from_f4( const float c[ 4 ] ) -> c_color
		{
			return c_color(
				static_cast<int>( c[ 0 ] * 255.f ) ,
				static_cast<int>( c[ 1 ] * 255.f ) ,
				static_cast<int>( c[ 2 ] * 255.f ) ,
				static_cast<int>( c[ 3 ] * 255.f ) );
		}

		inline auto to_f4( const c_color& c , float out[ 4 ] ) -> void
		{
			out[ 0 ] = c.r / 255.f;
			out[ 1 ] = c.g / 255.f;
			out[ 2 ] = c.b / 255.f;
			out[ 3 ] = c.a / 255.f;
		}

		inline auto pull( ) -> void
		{
			esp_col = from_f4( global::esp::color );
			hp_col = from_f4( global::esp::health_color );
			sh_col = from_f4( global::esp::shield_color );
			aim_key.key = global::aimbot::key;
		}

		inline auto push( ) -> void
		{
			to_f4( esp_col , global::esp::color );
			to_f4( hp_col , global::esp::health_color );
			to_f4( sh_col , global::esp::shield_color );
			global::aimbot::key = aim_key.key;
		}
	}

	class c_menu
	{
	public:
		void initialize( )
		{
			if ( m_initialized )
				return;

			binds::pull( );

			std::shared_ptr<c_window> main = std::make_shared<c_window>( "menu", c_vector_2d( 80.f, 60.f ), c_vector_2d( 748.f, 576.f ) );

			{
				auto child = main->build_child( "AIMBOT", child_width::half, 0.f, []( c_child* child )
				{
					child->add_checkbox( "Enabled", &global::aimbot::enabled )->attach_binds( );
					child->add_keybind( "Aim Key", &binds::aim_key );
					child->add_checkbox( "Prediction", &global::aimbot::prediction );
					child->add_checkbox( "Visible Check", &global::aimbot::visible_check );
					child->add_checkbox( "Snapline", &global::aimbot::snapline );
					child->add_checkbox( "FOV Circle", &global::aimbot::fov_circle );
					child->add_slider<float>( "Smoothing", &global::aimbot::smoothness, 0.f, 20.f )->attach_binds( );
					child->add_slider<float>( "FOV", &global::aimbot::fov, 0.f, 180.f )->attach_binds( );
				} );
				child->attach_child( "Combat", "" );
			}

			{
				auto child = main->build_child( "TARGET", child_width::half, 0.f, []( c_child* child )
				{
					child->add_dropdown( "Bone", &global::aimbot::bone_mode, { "Head", "Neck", "Chest", "Closest" } )->attach_binds( );
				} );
				child->attach_child( "Combat", "" );
			}

			{
				auto child = main->build_child( "PLAYER", child_width::half, 0.f, []( c_child* child )
				{
					child->add_checkbox( "Draw", &global::esp::draw );
					child->add_checkbox( "Box", &global::esp::box )->attach_popup( "Box", []( c_popup* pop )
					{
						pop->section( []( c_popup_section* sec )
						{
							sec->add_dropdown( "Style", &global::esp::box_type, { "Normal", "Cornered" } );
							sec->add_checkbox( "Outline", &global::esp::box_outline );
							sec->add_slider<float>( "Thickness", &global::esp::box_thickness, 0.5f, 4.f );
							sec->add_colorpicker( "Color", &binds::esp_col, false );
						} );
					} );
					child->add_checkbox( "Health Bar", &global::esp::health_bar )->attach_popup( "Health Bar", []( c_popup* pop )
					{
						pop->section( []( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &binds::hp_col, false );
						} );
					} );
					child->add_checkbox( "Shield Bar", &global::esp::shield )->attach_popup( "Shield Bar", []( c_popup* pop )
					{
						pop->section( []( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &binds::sh_col, false );
						} );
					} );
					child->add_checkbox( "Name", &global::esp::name )->attach_popup( "Name", []( c_popup* pop )
					{
						pop->section( []( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &binds::esp_col, false );
						} );
					} );
					child->add_checkbox( "Weapon", &global::esp::weapon )->attach_popup( "Weapon", []( c_popup* pop )
					{
						pop->section( []( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &binds::esp_col, false );
						} );
					} );
					child->add_checkbox( "Distance", &global::esp::distance )->attach_popup( "Distance", []( c_popup* pop )
					{
						pop->section( []( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &binds::esp_col, false );
						} );
					} );
					child->add_checkbox( "Flags", &global::esp::flags )->attach_popup( "Flags", []( c_popup* pop )
					{
						pop->section( []( c_popup_section* sec )
						{
							sec->add_colorpicker( "Color", &binds::esp_col, false );
						} );
					} );
					child->add_checkbox( "Head Dot", &global::esp::head_dot )->attach_popup( "Head Dot", []( c_popup* pop )
					{
						pop->section( []( c_popup_section* sec )
						{
							sec->add_slider<float>( "Scale", &global::esp::head_dot_scale, 0.35f, 2.5f );
							sec->add_colorpicker( "Color", &binds::esp_col, false );
						} );
					} );
					child->add_checkbox( "Skeleton", &global::esp::skeleton )->attach_popup( "Skeleton", []( c_popup* pop )
					{
						pop->section( []( c_popup_section* sec )
						{
							sec->add_slider<float>( "Thickness", &global::esp::skeleton_thickness, 0.5f, 4.f );
							sec->add_colorpicker( "Color", &binds::esp_col, false );
						} );
					} );
					child->add_checkbox( "Visible Check", &global::esp::visible_check )->attach_popup( "Visible Check", []( c_popup* pop )
					{
						pop->section( []( c_popup_section* sec )
						{
							sec->add_slider<float>( "Occluded Alpha", &global::esp::occluded_alpha, 0.05f, 1.f );
						} );
					} );
				} );
				child->attach_child( "Visuals", "Player", "Enemies" );
			}

			{
				auto child = main->build_child( "COLORS", child_width::half, 0.f, []( c_child* child )
				{
					child->add_colorpicker( "ESP", &binds::esp_col, false, &binds::esp_col_on );
					child->add_colorpicker( "Health", &binds::hp_col, false, &binds::hp_col_on );
					child->add_colorpicker( "Shield", &binds::sh_col, false, &binds::sh_col_on );
				} );
				child->attach_child( "Visuals", "Player", "Enemies" );
			}

			{
				auto child = main->build_child( "LOOT", child_width::half, 0.f, []( c_child* child )
				{
					child->add_checkbox( "Enabled", &global::loot::enabled );
					child->add_checkbox( "Name", &global::loot::name );
					child->add_checkbox( "Distance", &global::loot::distance );
					child->add_checkbox( "Death Boxes", &global::loot::death_box );
					child->add_checkbox( "Weapons", &global::loot::weapons );
					child->add_checkbox( "Ammo", &global::loot::ammo );
					child->add_checkbox( "Heals", &global::loot::heals );
					child->add_checkbox( "Gear", &global::loot::gear );
					child->add_checkbox( "Attachments", &global::loot::attachments );
					child->add_checkbox( "Grenades", &global::loot::grenades );
					child->add_checkbox( "Misc", &global::loot::misc );
					child->add_slider<float>( "Max Distance", &global::loot::max_distance, 25.f, 400.f );
					child->add_dropdown( "Min Rarity", &global::loot::min_rarity, { "Common", "Rare", "Epic", "Legendary", "Heirloom" } );
				} );
				child->attach_child( "Visuals", "World" );
			}

			{
				std::vector<std::string> heirloom_items;
				const int heirloom_count = heirloom::catalog_count( );
				heirloom_items.reserve( static_cast<std::size_t>( heirloom_count ) );
				for ( int i = 0; i < heirloom_count; ++i )
					heirloom_items.emplace_back( heirloom::catalog_label( i ) );

				auto child = main->build_child( "HEIRLOOM", child_width::half, 0.f, [ heirloom_items = std::move( heirloom_items ) ]( c_child* child ) mutable
				{
					child->add_checkbox( "Enabled", &global::heirloom::enabled );
					child->add_checkbox( "Debug Log", &global::heirloom::debug );
					child->add_dropdown( "Melee", &global::heirloom::selection, std::move( heirloom_items ) );
				} );
				child->attach_child( "Visuals", "World" );
			}

			{
				auto child = main->build_child( "MENU", child_width::half, 0.f, []( c_child* child )
				{
					child->add_checkbox( "Keybind List", &global::misc::keybind_list )->attach_popup( "Keybind List", []( c_popup* pop )
					{
						pop->section( []( c_popup_section* sec )
						{
							sec->add_checkbox( "Active Only", &global::misc::keybind_list_active_only );
						} );
					} );
				} );
				child->attach_child( "Settings", "" );
			}

			if ( g_keybind_list )
			{
				g_keybind_list->set_enabled( &global::misc::keybind_list );
				g_keybind_list->set_only_active( &global::misc::keybind_list_active_only );
			}

			m_windows.push_back( main );
			if ( g_ctx )
			{
				g_ctx->m_open = false;
				g_ctx->m_open_anim = 0.f;
			}
			m_initialized = true;
		}

		void runtime( )
		{
			if ( !m_initialized )
				return;

			if ( g_input )
				g_input->update( );

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

			binds::push( );

			const bool menu_vis = g_ctx && g_ctx->m_open_anim >= 0.001f;
			if ( menu_vis )
			{
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
			}

			if ( g_keybind_list && global::misc::keybind_list )
			{
				if ( g_render )
				{
					if ( !menu_vis )
						g_render->setup( );
					g_keybind_list->paint( );
				}
			}
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
