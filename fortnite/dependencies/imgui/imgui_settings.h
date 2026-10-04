#pragma once
#ifndef EMULATED_IMGUI_SETTINGS_H
#define EMULATED_IMGUI_SETTINGS_H

#include "imgui.h"
#include "imgui_internal.h"
#include <Windows.h>
#include <Xinput.h>
#include "../joyshock/JoyShockLibrary.h"
#include <string>
#include <atomic>
#pragma comment(lib, "xinput9_1_0.lib")

namespace c
{
	inline ImVec4 accent_color = ImColor(192, 239, 42);
	inline ImVec4 accent_low_color = ImColor((int)(accent_color.x * 255), (int)(accent_color.y * 255), (int)(accent_color.z * 255), 255 / 2);
	inline ImVec4 accent_low = ImColor((int)(accent_color.x * 255), (int)(accent_color.y * 255), (int)(accent_color.z * 255), 255 / 2);

	inline ImVec4 accent_text_color = ImColor(245, 245, 255);
	inline ImVec4 accent_warning_text_color = ImColor(255, 220, 150);

	inline ImVec4 accent_text_low_color = ImColor(245, 245, 255, 255 / 2);

	inline ImVec4 notify = ImColor(43, 48, 54);


	namespace bg
	{
		inline ImVec4 background = ImColor(18, 19, 19, 255);
		inline ImVec4 light_background = ImColor(25, 26, 26, 255);
		inline ImVec4 dark_background = ImColor(14, 15, 15, 255);
		inline ImVec4 outline_background = ImColor(250,250,250, 20);
		inline ImVec2 size = ImVec2(794, 490);
		inline float rounding = 12;
	}

	namespace child
	{
		inline ImVec4 background = ImColor(22, 23, 23, 250);
		inline ImVec4 outline_background = ImColor(27, 29, 32, 255);
		inline float rounding = 6;
	}

	namespace checkbox
	{
		inline ImVec4 circle_inactive = ImColor(32, 33, 33, 255);

		inline ImVec4 background = ImColor(14, 14, 15, 255);
		inline ImVec4 outline_background = ImColor(30, 32, 36, 255);
		inline float rounding = 30;
	}

	namespace slider
	{
		inline ImVec4 circle_inactive = ImColor(32, 33, 33, 255);

		inline ImVec4 background = ImColor(14, 15, 15, 255);
		inline ImVec4 outline_background = ImColor(16, 17, 17, 255);
		inline float rounding = 30;
	}

	namespace combo
	{
		inline ImVec4 background = ImColor(14, 15, 15, 255);
		inline ImVec4 outline_background = ImColor(16, 17, 17, 255);
		inline float rounding = 3;
	}

	namespace picker
	{
		inline ImVec4 background = ImColor(22, 23, 23, 255);
		inline ImVec4 outline_background = ImColor(14, 15, 15, 255);
		inline float rounding = 2;
	}

	namespace button
	{
		inline ImVec4 background = ImColor(14, 15, 15, 255);
		inline ImVec4 outline_background = ImColor(16, 17, 17, 255);
		inline float rounding = 4;
	}

	namespace input
	{
		inline ImVec4 background = ImColor(14, 15, 15, 255);
		inline ImVec4 outline_background = ImColor(16, 17, 17, 255);
		inline float rounding = 4;
	}

	namespace keybind
	{
		inline ImVec4 background = ImColor(27, 29, 32, 255);
		inline ImVec4 outline_background = ImColor(30, 32, 36, 255);
		inline float rounding = 3;
	}


	namespace text
	{
		inline ImVec4 text_hov = ImColor(245, 245, 255);
		inline ImVec4 text = ImColor(90, 93, 100);
		inline ImVec4 text2 = ImColor(90, 93, 100,0);
		inline ImVec4 hide_text = ImColor(43, 48, 54, 255);

	}
}

namespace CustomWidgets
{
	inline bool controller_capture_only = false;
	inline constexpr int k_controller_bind_base = 0x10000;
	inline constexpr int k_controller_bind_a = k_controller_bind_base + 1;
	inline constexpr int k_controller_bind_b = k_controller_bind_base + 2;
	inline constexpr int k_controller_bind_x = k_controller_bind_base + 3;
	inline constexpr int k_controller_bind_y = k_controller_bind_base + 4;
	inline constexpr int k_controller_bind_lb = k_controller_bind_base + 5;
	inline constexpr int k_controller_bind_rb = k_controller_bind_base + 6;
	inline constexpr int k_controller_bind_back = k_controller_bind_base + 7;
	inline constexpr int k_controller_bind_start = k_controller_bind_base + 8;
	inline constexpr int k_controller_bind_l3 = k_controller_bind_base + 9;
	inline constexpr int k_controller_bind_r3 = k_controller_bind_base + 10;
	inline constexpr int k_controller_bind_dpad_up = k_controller_bind_base + 11;
	inline constexpr int k_controller_bind_dpad_down = k_controller_bind_base + 12;
	inline constexpr int k_controller_bind_dpad_left = k_controller_bind_base + 13;
	inline constexpr int k_controller_bind_dpad_right = k_controller_bind_base + 14;
	inline constexpr int k_controller_bind_trigger_left = k_controller_bind_base + 15;
	inline constexpr int k_controller_bind_trigger_right = k_controller_bind_base + 16;
	inline constexpr int k_controller_bind_lstick_up = k_controller_bind_base + 17;
	inline constexpr int k_controller_bind_lstick_down = k_controller_bind_base + 18;
	inline constexpr int k_controller_bind_lstick_left = k_controller_bind_base + 19;
	inline constexpr int k_controller_bind_lstick_right = k_controller_bind_base + 20;
	inline constexpr int k_controller_bind_rstick_up = k_controller_bind_base + 21;
	inline constexpr int k_controller_bind_rstick_down = k_controller_bind_base + 22;
	inline constexpr int k_controller_bind_rstick_left = k_controller_bind_base + 23;
	inline constexpr int k_controller_bind_rstick_right = k_controller_bind_base + 24;

	struct Keybind
	{
		int key = 0;
		bool listening = false;
		std::atomic<bool> is_down { false };
	};

	inline bool IsControllerBind ( int key )
	{
		return key >= k_controller_bind_base;
	}

	inline void ensure_joyshock_connected ( ) {
		static bool initialized = false;
		if ( !initialized ) {
			JslConnectDevices ( );
			initialized = true;
		}
	}

	inline bool joyshock_connected ( ) {
		ensure_joyshock_connected ( );
		int handles [ 16 ] = {};
		return JslGetConnectedDeviceHandles ( handles , 16 ) > 0;
	}

	inline bool joyshock_hold_combo_active ( ) {
		ensure_joyshock_connected ( );
		int handles [ 16 ] = {};
		const int count = JslGetConnectedDeviceHandles ( handles , 16 );
		for ( int i = 0; i < count; ++i ) {
			const int id = handles [ i ];
			if ( !JslStillConnected ( id ) ) continue;
			const bool lt = JslGetLeftTrigger ( id ) > 0.5f || ( JslGetButtons ( id ) & JSMASK_ZL ) != 0;
			const bool rt = JslGetRightTrigger ( id ) > 0.5f || ( JslGetButtons ( id ) & JSMASK_ZR ) != 0;
			if ( lt && rt ) return true;
		}
		return false;
	}

	inline const char* ControllerButtonName ( int bind_key )
	{
		if ( bind_key == k_controller_bind_a ) return "A";
		if ( bind_key == k_controller_bind_b ) return "B";
		if ( bind_key == k_controller_bind_x ) return "X";
		if ( bind_key == k_controller_bind_y ) return "Y";
		if ( bind_key == k_controller_bind_lb ) return "LB";
		if ( bind_key == k_controller_bind_rb ) return "RB";
		if ( bind_key == k_controller_bind_back ) return "Back";
		if ( bind_key == k_controller_bind_start ) return "Start";
		if ( bind_key == k_controller_bind_l3 ) return "L3";
		if ( bind_key == k_controller_bind_r3 ) return "R3";
		if ( bind_key == k_controller_bind_dpad_up ) return "DPad Up";
		if ( bind_key == k_controller_bind_dpad_down ) return "DPad Down";
		if ( bind_key == k_controller_bind_dpad_left ) return "DPad Left";
		if ( bind_key == k_controller_bind_dpad_right ) return "DPad Right";
		if ( bind_key == k_controller_bind_trigger_left ) return "LT";
		if ( bind_key == k_controller_bind_trigger_right ) return "RT";
		if ( bind_key == k_controller_bind_lstick_up ) return "LS Up";
		if ( bind_key == k_controller_bind_lstick_down ) return "LS Down";
		if ( bind_key == k_controller_bind_lstick_left ) return "LS Left";
		if ( bind_key == k_controller_bind_lstick_right ) return "LS Right";
		if ( bind_key == k_controller_bind_rstick_up ) return "RS Up";
		if ( bind_key == k_controller_bind_rstick_down ) return "RS Down";
		if ( bind_key == k_controller_bind_rstick_left ) return "RS Left";
		if ( bind_key == k_controller_bind_rstick_right ) return "RS Right";
		return "None";
	}

	inline bool GetPressedControllerButton ( int* out_key )
	{
		if ( !out_key )
			return false;

		for ( DWORD i = 0; i < XUSER_MAX_COUNT; ++i )
		{
			XINPUT_STATE state {};
			if ( XInputGetState ( i , &state ) != ERROR_SUCCESS )
				continue;

			if ( ( state.Gamepad.wButtons & XINPUT_GAMEPAD_A ) != 0 ) { *out_key = k_controller_bind_a; return true; }
			if ( ( state.Gamepad.wButtons & XINPUT_GAMEPAD_B ) != 0 ) { *out_key = k_controller_bind_b; return true; }
			if ( ( state.Gamepad.wButtons & XINPUT_GAMEPAD_X ) != 0 ) { *out_key = k_controller_bind_x; return true; }
			if ( ( state.Gamepad.wButtons & XINPUT_GAMEPAD_Y ) != 0 ) { *out_key = k_controller_bind_y; return true; }
			if ( ( state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER ) != 0 ) { *out_key = k_controller_bind_lb; return true; }
			if ( ( state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER ) != 0 ) { *out_key = k_controller_bind_rb; return true; }
			if ( ( state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK ) != 0 ) { *out_key = k_controller_bind_back; return true; }
			if ( ( state.Gamepad.wButtons & XINPUT_GAMEPAD_START ) != 0 ) { *out_key = k_controller_bind_start; return true; }
			if ( ( state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB ) != 0 ) { *out_key = k_controller_bind_l3; return true; }
			if ( ( state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB ) != 0 ) { *out_key = k_controller_bind_r3; return true; }
			if ( ( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP ) != 0 ) { *out_key = k_controller_bind_dpad_up; return true; }
			if ( ( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN ) != 0 ) { *out_key = k_controller_bind_dpad_down; return true; }
			if ( ( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT ) != 0 ) { *out_key = k_controller_bind_dpad_left; return true; }
			if ( ( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT ) != 0 ) { *out_key = k_controller_bind_dpad_right; return true; }

			if ( state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ) {
				*out_key = k_controller_bind_trigger_left;
				return true;
			}

			if ( state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ) {
				*out_key = k_controller_bind_trigger_right;
				return true;
			}

			if ( state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ) {
				*out_key = k_controller_bind_lstick_right;
				return true;
			}
			if ( state.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ) {
				*out_key = k_controller_bind_lstick_left;
				return true;
			}
			if ( state.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ) {
				*out_key = k_controller_bind_lstick_up;
				return true;
			}
			if ( state.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ) {
				*out_key = k_controller_bind_lstick_down;
				return true;
			}

			if ( state.Gamepad.sThumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ) {
				*out_key = k_controller_bind_rstick_right;
				return true;
			}
			if ( state.Gamepad.sThumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ) {
				*out_key = k_controller_bind_rstick_left;
				return true;
			}
			if ( state.Gamepad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ) {
				*out_key = k_controller_bind_rstick_up;
				return true;
			}
			if ( state.Gamepad.sThumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ) {
				*out_key = k_controller_bind_rstick_down;
				return true;
			}
		}

		// JoyShockLibrary fallback/backend for non-XInput controllers.
		ensure_joyshock_connected ( );
		int handles [ 16 ] = {};
		const int count = JslGetConnectedDeviceHandles ( handles , 16 );
		for ( int i = 0; i < count; ++i ) {
			const int id = handles [ i ];
			if ( !JslStillConnected ( id ) ) continue;

			const JOY_SHOCK_STATE st = JslGetSimpleState ( id );
			const int buttons = st.buttons;

			if ( ( buttons & JSMASK_S ) != 0 ) { *out_key = k_controller_bind_a; return true; }
			if ( ( buttons & JSMASK_E ) != 0 ) { *out_key = k_controller_bind_b; return true; }
			if ( ( buttons & JSMASK_W ) != 0 ) { *out_key = k_controller_bind_x; return true; }
			if ( ( buttons & JSMASK_N ) != 0 ) { *out_key = k_controller_bind_y; return true; }
			if ( ( buttons & JSMASK_L ) != 0 ) { *out_key = k_controller_bind_lb; return true; }
			if ( ( buttons & JSMASK_R ) != 0 ) { *out_key = k_controller_bind_rb; return true; }
			if ( ( buttons & JSMASK_MINUS ) != 0 ) { *out_key = k_controller_bind_back; return true; }
			if ( ( buttons & JSMASK_PLUS ) != 0 ) { *out_key = k_controller_bind_start; return true; }
			if ( ( buttons & JSMASK_LCLICK ) != 0 ) { *out_key = k_controller_bind_l3; return true; }
			if ( ( buttons & JSMASK_RCLICK ) != 0 ) { *out_key = k_controller_bind_r3; return true; }
			if ( ( buttons & JSMASK_UP ) != 0 ) { *out_key = k_controller_bind_dpad_up; return true; }
			if ( ( buttons & JSMASK_DOWN ) != 0 ) { *out_key = k_controller_bind_dpad_down; return true; }
			if ( ( buttons & JSMASK_LEFT ) != 0 ) { *out_key = k_controller_bind_dpad_left; return true; }
			if ( ( buttons & JSMASK_RIGHT ) != 0 ) { *out_key = k_controller_bind_dpad_right; return true; }
			if ( st.lTrigger > 0.5f || ( buttons & JSMASK_ZL ) != 0 ) { *out_key = k_controller_bind_trigger_left; return true; }
			if ( st.rTrigger > 0.5f || ( buttons & JSMASK_ZR ) != 0 ) { *out_key = k_controller_bind_trigger_right; return true; }
			if ( st.stickLX > 0.5f ) { *out_key = k_controller_bind_lstick_right; return true; }
			if ( st.stickLX < -0.5f ) { *out_key = k_controller_bind_lstick_left; return true; }
			if ( st.stickLY > 0.5f ) { *out_key = k_controller_bind_lstick_up; return true; }
			if ( st.stickLY < -0.5f ) { *out_key = k_controller_bind_lstick_down; return true; }
			if ( st.stickRX > 0.5f ) { *out_key = k_controller_bind_rstick_right; return true; }
			if ( st.stickRX < -0.5f ) { *out_key = k_controller_bind_rstick_left; return true; }
			if ( st.stickRY > 0.5f ) { *out_key = k_controller_bind_rstick_up; return true; }
			if ( st.stickRY < -0.5f ) { *out_key = k_controller_bind_rstick_down; return true; }
		}

		return false;
	}

	inline const char* KeyName ( int vk )
	{
		if ( IsControllerBind ( vk ) ) {
			return ControllerButtonName ( vk );
		}

		switch ( vk )
		{
		case VK_LBUTTON: return "LMB";
		case VK_RBUTTON: return "RMB";
		case VK_MBUTTON: return "MMB";
		case VK_XBUTTON1: return "Mouse4";
		case VK_XBUTTON2: return "Mouse5";
		case VK_INSERT: return "Insert";
		case VK_DELETE: return "Delete";
		case VK_HOME: return "Home";
		case VK_END: return "End";
		case VK_PRIOR: return "PageUp";
		case VK_NEXT: return "PageDown";
		case VK_SPACE: return "Space";
		case VK_TAB: return "Tab";
		case VK_RETURN: return "Enter";
		case VK_BACK: return "Backspace";
		case VK_CAPITAL: return "CapsLock";
		case VK_NUMLOCK: return "NumLock";
		case VK_SCROLL: return "ScrollLock";
		case VK_PAUSE: return "Pause";
		case VK_SNAPSHOT: return "PrintScreen";
		case VK_LEFT: return "Left";
		case VK_RIGHT: return "Right";
		case VK_UP: return "Up";
		case VK_DOWN: return "Down";
		case VK_LWIN: return "LWin";
		case VK_RWIN: return "RWin";
		case VK_APPS: return "Menu";
		case VK_OEM_3: return "`";
		case VK_OEM_MINUS: return "-";
		case VK_OEM_PLUS: return "=";
		case VK_OEM_4: return "[";
		case VK_OEM_6: return "]";
		case VK_OEM_5: return "\\";
		case VK_OEM_1: return ";";
		case VK_OEM_7: return "'";
		case VK_OEM_COMMA: return ",";
		case VK_OEM_PERIOD: return ".";
		case VK_OEM_2: return "/";
		case VK_MULTIPLY: return "Num *";
		case VK_ADD: return "Num +";
		case VK_SUBTRACT: return "Num -";
		case VK_DECIMAL: return "Num .";
		case VK_DIVIDE: return "Num /";
		case VK_NUMPAD0: return "Num0";
		case VK_NUMPAD1: return "Num1";
		case VK_NUMPAD2: return "Num2";
		case VK_NUMPAD3: return "Num3";
		case VK_NUMPAD4: return "Num4";
		case VK_NUMPAD5: return "Num5";
		case VK_NUMPAD6: return "Num6";
		case VK_NUMPAD7: return "Num7";
		case VK_NUMPAD8: return "Num8";
		case VK_NUMPAD9: return "Num9";
		case VK_SHIFT: return "Shift";
		case VK_LSHIFT: return "LShift";
		case VK_RSHIFT: return "RShift";
		case VK_CONTROL: return "Ctrl";
		case VK_LCONTROL: return "LCtrl";
		case VK_RCONTROL: return "RCtrl";
		case VK_MENU: return "Alt";
		case VK_LMENU: return "LAlt";
		case VK_RMENU: return "RAlt";
		case VK_ESCAPE: return "None";
		default: break;
		}

		if ( vk >= VK_F1 && vk <= VK_F24 ) {
			static char fn_name [ 8 ];
			wsprintfA ( fn_name , "F%d" , ( vk - VK_F1 + 1 ) );
			return fn_name;
		}

		static char key_name [ 64 ];
		UINT sc = MapVirtualKeyA ( static_cast< UINT >( vk ) , MAPVK_VK_TO_VSC );
		LONG lparam = static_cast< LONG >( sc << 16 );
		int len = GetKeyNameTextA ( lparam , key_name , static_cast< int >( sizeof ( key_name ) ) );
		if ( len > 0 )
			return key_name;

		static char fallback [ 16 ];
		wsprintfA ( fallback , "VK_%d" , vk );
		return fallback;
	}

	inline bool RenderKeybind ( const char* label , Keybind* bind )
	{
		if ( !bind )
			return false;

		const bool changed = ImGui::Keybind ( label , &bind->key , true );
		bind->listening = ( ImGui::GetActiveID ( ) == ImGui::GetCurrentWindow ( )->GetID ( label ) );
		return changed;
	}

	inline bool RenderControllerKeybind ( const char* label , Keybind* bind )
	{
		if ( !bind )
			return false;

		// Use the same styled keybind widget as keyboard binds.
		controller_capture_only = true;
		bool changed = ImGui::Keybind ( label , &bind->key , true );
		controller_capture_only = false;
		const ImGuiID bind_id = ImGui::GetCurrentWindow ( )->GetID ( label );
		bind->listening = ( ImGui::GetActiveID ( ) == bind_id );

		if ( bind->listening ) {
			int captured_key = 0;
			if ( GetPressedControllerButton ( &captured_key ) ) {
				bind->key = captured_key;
				changed = true;
				ImGui::ClearActiveID ( );
				bind->listening = false;
			}
		}

		return changed;
	}

	inline void UpdateKeybindState ( Keybind* bind )
	{
		if ( !bind || bind->key == 0 )
		{
			if ( bind ) bind->is_down.store ( false , std::memory_order_relaxed );
			return;
		}

		bind->is_down.store ( ( GetAsyncKeyState ( bind->key ) & 0x8000 ) != 0 , std::memory_order_relaxed );
	}

	inline void UpdateControllerKeybindState ( Keybind* bind )
	{
		if ( !bind || bind->key == 0 || !IsControllerBind ( bind->key ) )
		{
			if ( bind ) bind->is_down.store ( false , std::memory_order_relaxed );
			return;
		}

		bool is_down = false;

		for ( DWORD i = 0; i < XUSER_MAX_COUNT; ++i )
		{
			XINPUT_STATE state {};
			if ( XInputGetState ( i , &state ) != ERROR_SUCCESS )
				continue;

			if ( bind->key == k_controller_bind_a ) is_down = ( state.Gamepad.wButtons & XINPUT_GAMEPAD_A ) != 0;
			else if ( bind->key == k_controller_bind_b ) is_down = ( state.Gamepad.wButtons & XINPUT_GAMEPAD_B ) != 0;
			else if ( bind->key == k_controller_bind_x ) is_down = ( state.Gamepad.wButtons & XINPUT_GAMEPAD_X ) != 0;
			else if ( bind->key == k_controller_bind_y ) is_down = ( state.Gamepad.wButtons & XINPUT_GAMEPAD_Y ) != 0;
			else if ( bind->key == k_controller_bind_lb ) is_down = ( state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER ) != 0;
			else if ( bind->key == k_controller_bind_rb ) is_down = ( state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER ) != 0;
			else if ( bind->key == k_controller_bind_back ) is_down = ( state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK ) != 0;
			else if ( bind->key == k_controller_bind_start ) is_down = ( state.Gamepad.wButtons & XINPUT_GAMEPAD_START ) != 0;
			else if ( bind->key == k_controller_bind_l3 ) is_down = ( state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB ) != 0;
			else if ( bind->key == k_controller_bind_r3 ) is_down = ( state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB ) != 0;
			else if ( bind->key == k_controller_bind_dpad_up ) is_down = ( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP ) != 0;
			else if ( bind->key == k_controller_bind_dpad_down ) is_down = ( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN ) != 0;
			else if ( bind->key == k_controller_bind_dpad_left ) is_down = ( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT ) != 0;
			else if ( bind->key == k_controller_bind_dpad_right ) is_down = ( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT ) != 0;
			else if ( bind->key == k_controller_bind_trigger_left ) {
				is_down = state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
			}
			else if ( bind->key == k_controller_bind_trigger_right ) {
				is_down = state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
			}
			else if ( bind->key == k_controller_bind_lstick_right ) {
				is_down = state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
			}
			else if ( bind->key == k_controller_bind_lstick_left ) {
				is_down = state.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
			}
			else if ( bind->key == k_controller_bind_lstick_up ) {
				is_down = state.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
			}
			else if ( bind->key == k_controller_bind_lstick_down ) {
				is_down = state.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
			}
			else if ( bind->key == k_controller_bind_rstick_right ) {
				is_down = state.Gamepad.sThumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
			}
			else if ( bind->key == k_controller_bind_rstick_left ) {
				is_down = state.Gamepad.sThumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
			}
			else if ( bind->key == k_controller_bind_rstick_up ) {
				is_down = state.Gamepad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
			}
			else if ( bind->key == k_controller_bind_rstick_down ) {
				is_down = state.Gamepad.sThumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
			}

			if ( is_down ) break;
		}

		// JoyShockLibrary fallback/backend.
		if ( !is_down ) {
			ensure_joyshock_connected ( );
			int handles [ 16 ] = {};
			const int count = JslGetConnectedDeviceHandles ( handles , 16 );
			for ( int i = 0; i < count; ++i ) {
				const int id = handles [ i ];
				if ( !JslStillConnected ( id ) ) continue;
				const JOY_SHOCK_STATE st = JslGetSimpleState ( id );
				const int buttons = st.buttons;

				if ( bind->key == k_controller_bind_a ) is_down = ( buttons & JSMASK_S ) != 0;
				else if ( bind->key == k_controller_bind_b ) is_down = ( buttons & JSMASK_E ) != 0;
				else if ( bind->key == k_controller_bind_x ) is_down = ( buttons & JSMASK_W ) != 0;
				else if ( bind->key == k_controller_bind_y ) is_down = ( buttons & JSMASK_N ) != 0;
				else if ( bind->key == k_controller_bind_lb ) is_down = ( buttons & JSMASK_L ) != 0;
				else if ( bind->key == k_controller_bind_rb ) is_down = ( buttons & JSMASK_R ) != 0;
				else if ( bind->key == k_controller_bind_back ) is_down = ( buttons & JSMASK_MINUS ) != 0;
				else if ( bind->key == k_controller_bind_start ) is_down = ( buttons & JSMASK_PLUS ) != 0;
				else if ( bind->key == k_controller_bind_l3 ) is_down = ( buttons & JSMASK_LCLICK ) != 0;
				else if ( bind->key == k_controller_bind_r3 ) is_down = ( buttons & JSMASK_RCLICK ) != 0;
				else if ( bind->key == k_controller_bind_dpad_up ) is_down = ( buttons & JSMASK_UP ) != 0;
				else if ( bind->key == k_controller_bind_dpad_down ) is_down = ( buttons & JSMASK_DOWN ) != 0;
				else if ( bind->key == k_controller_bind_dpad_left ) is_down = ( buttons & JSMASK_LEFT ) != 0;
				else if ( bind->key == k_controller_bind_dpad_right ) is_down = ( buttons & JSMASK_RIGHT ) != 0;
				else if ( bind->key == k_controller_bind_trigger_left ) is_down = st.lTrigger > 0.5f || ( buttons & JSMASK_ZL ) != 0;
				else if ( bind->key == k_controller_bind_trigger_right ) is_down = st.rTrigger > 0.5f || ( buttons & JSMASK_ZR ) != 0;
				else if ( bind->key == k_controller_bind_lstick_right ) is_down = st.stickLX > 0.5f;
				else if ( bind->key == k_controller_bind_lstick_left ) is_down = st.stickLX < -0.5f;
				else if ( bind->key == k_controller_bind_lstick_up ) is_down = st.stickLY > 0.5f;
				else if ( bind->key == k_controller_bind_lstick_down ) is_down = st.stickLY < -0.5f;
				else if ( bind->key == k_controller_bind_rstick_right ) is_down = st.stickRX > 0.5f;
				else if ( bind->key == k_controller_bind_rstick_left ) is_down = st.stickRX < -0.5f;
				else if ( bind->key == k_controller_bind_rstick_up ) is_down = st.stickRY > 0.5f;
				else if ( bind->key == k_controller_bind_rstick_down ) is_down = st.stickRY < -0.5f;

				if ( is_down ) break;
			}
		}

		bind->is_down.store ( is_down , std::memory_order_relaxed );
	}
}

#endif // EMULATED_IMGUI_SETTINGS_H
