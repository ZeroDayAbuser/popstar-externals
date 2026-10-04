#pragma once

#include <Windows.h>
#include <Xinput.h>
#include <algorithm>
#include "../../dependencies/imgui/imgui.h"
#include "../../dependencies/imgui/imgui_settings.h"
#include "../../dependencies/imgui/gui.h"
#include "../../workspace/core/unreal-engine/settings/settings.h"
#include "../../dependencies/oxorany/oxorany.h"

namespace detection {
	inline bool onboarding_checked = false;
	inline bool onboarding_active = false;
	inline float hold_time = 0.0f;
	inline float onboarding_visible_time = 0.0f;

	inline bool has_connected_controller ( ) {
		if ( CustomWidgets::joyshock_connected ( ) ) {
			return true;
		}

		for ( DWORD i = 0; i < XUSER_MAX_COUNT; ++i ) {
			XINPUT_STATE state {};
			if ( XInputGetState ( i , &state ) == ERROR_SUCCESS ) {
				return true;
			}
		}
		return false;
	}

	inline bool is_hold_combo_active ( ) {
		if ( CustomWidgets::joyshock_hold_combo_active ( ) ) {
			return true;
		}

		for ( DWORD i = 0; i < XUSER_MAX_COUNT; ++i ) {
			XINPUT_STATE state {};
			if ( XInputGetState ( i , &state ) != ERROR_SUCCESS ) {
				continue;
			}

			const bool lt_down = state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
			const bool rt_down = state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
			if ( lt_down && rt_down ) {
				return true;
			}
		}
		return false;
	}

	inline void draw_onboarding_card ( float hold_seconds , float hold_required ) {
		const float progress = ( std::min ) ( hold_seconds / hold_required , 1.0f );

		ImDrawList* draw = ImGui::GetForegroundDrawList ( );
		const ImVec2 vp = ImGui::GetMainViewport ( )->WorkPos;
		const ImVec2 vs = ImGui::GetMainViewport ( )->WorkSize;

		const float panel_w = 360.0f;
		const float panel_h = 76.0f;
		const ImVec2 panel_min ( vp.x + vs.x - panel_w - 14.0f , vp.y + 84.0f );
		const ImVec2 panel_max ( panel_min.x + panel_w , panel_min.y + panel_h );

		const ImU32 accent = ImGui::GetColorU32 ( ui::colors::main );
		const ImU32 outline = ImGui::GetColorU32 ( ui::colors::outline );
		draw->AddRectFilled ( panel_min , panel_max , ImGui::GetColorU32 ( ui::colors::background ) , 6.0f );
		draw->AddRect ( panel_min , panel_max , outline , 6.0f );
		draw->AddRectFilled ( panel_min , ImVec2 ( panel_min.x + 4.0f , panel_max.y ) , accent , 6.0f , ImDrawFlags_RoundCornersLeft );

		draw->AddText ( ImVec2 ( panel_min.x + 16.0f , panel_min.y + 10.0f ) , IM_COL32 ( 245 , 245 , 245 , 255 ) , oxorany ( "Controller Setup" ) );
		draw->AddText ( ImVec2 ( panel_min.x + 16.0f , panel_min.y + 30.0f ) , ImGui::GetColorU32 ( ui::colors::text ) , oxorany ( "Hold LT + RT for 5 seconds" ) );

		char hold_text [ 64 ] = {};
		sprintf_s ( hold_text , "%.1fs / 5.0s" , hold_seconds );
		draw->AddText ( ImVec2 ( panel_max.x - 80.0f , panel_min.y + 30.0f ) , ImGui::GetColorU32 ( ui::colors::text ) , hold_text );

		const ImVec2 bar_min ( panel_min.x + 16.0f , panel_min.y + 52.0f );
		const ImVec2 bar_max ( panel_max.x - 16.0f , panel_min.y + 64.0f );
		draw->AddRectFilled ( bar_min , bar_max , ImGui::GetColorU32 ( ui::colors::background_dark ) , 4.0f );

		const float fill_x = bar_min.x + ( bar_max.x - bar_min.x ) * progress;
		draw->AddRectFilled ( bar_min , ImVec2 ( fill_x , bar_max.y ) , accent , 4.0f );
		draw->AddRect ( bar_min , bar_max , outline , 4.0f );
	}

	inline void find_controllers ( float delta_time ) {
		if ( !onboarding_checked ) {
			onboarding_checked = true;

			if ( has_connected_controller ( ) && !g_aimbot::controller_support ) {
				onboarding_active = true;
				ui::add_notification ( oxorany ( "B" ) , oxorany ( "Controller detected: hold LT + RT for 5 seconds to enable controller support." ) , ImVec4 ( 0.95f , 0.82f , 0.20f , 1.0f ) );
			}
		}

		if ( !onboarding_active || g_aimbot::controller_support ) {
			return;
		}

		onboarding_visible_time += delta_time;
		if ( onboarding_visible_time >= 15.0f ) {
			onboarding_active = false;
			hold_time = 0.0f;
			onboarding_visible_time = 0.0f;
			return;
		}

		if ( !has_connected_controller ( ) ) {
			onboarding_active = false;
			hold_time = 0.0f;
			onboarding_visible_time = 0.0f;
			return;
		}

		if ( is_hold_combo_active ( ) ) {
			hold_time += delta_time;
		}
		else {
			hold_time = ( std::max ) ( 0.0f , hold_time - ( delta_time * 1.5f ) );
		}

		const float hold_required = 5.0f;
		draw_onboarding_card ( hold_time , hold_required );

		if ( hold_time >= hold_required ) {
			g_aimbot::controller_support = true;
			onboarding_active = false;
			hold_time = 0.0f;
			onboarding_visible_time = 0.0f;
			ui::add_notification ( oxorany ( "B" ) , oxorany ( "Controller support enabled." ) , ImVec4 ( 0.20f , 0.85f , 0.35f , 1.0f ) );
		}
	}
}
