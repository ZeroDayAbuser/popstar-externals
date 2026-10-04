#pragma once

#include <memory>
#include <string>
#include <windows.h>

#include <imgui.h>

#include <core/framework/gui/backend/math/math.hxx>

namespace core::gui
{
	enum mouse_buttons
	{
		left = 0,
		right = 1,
		middle = 2,
		x1 = 3,
		x2 = 4,
		count = 5
	};

	class c_input
	{
	public:
		void update( )
		{
			for ( int i = 0; i < 256; ++i )
			{
				const bool down = ( GetAsyncKeyState( i ) & 0x8000 ) != 0;
				m_key_pressed[i] = down && !m_key_down[i];
				m_key_released[i] = !down && m_key_down[i];
				m_key_down[i] = down;
			}
		}

		c_vector_2d get_mouse_position( ) const
		{
			const ImVec2 p = ImGui::GetIO( ).MousePos;
			return c_vector_2d( p.x, p.y );
		}

		bool mouse_in_region( const c_vector_2d& pos, const c_vector_2d& size ) const
		{
			const c_vector_2d m = get_mouse_position( );
			return m.x >= pos.x && m.y >= pos.y && m.x <= pos.x + size.x && m.y <= pos.y + size.y;
		}

		bool mouse_in_region( float x, float y, float w, float h ) const
		{
			return mouse_in_region( c_vector_2d( x, y ), c_vector_2d( w, h ) );
		}

		bool clicked( mouse_buttons button ) const
		{
			return ImGui::IsMouseClicked( static_cast<ImGuiMouseButton>( button ) );
		}

		bool click_down( mouse_buttons button ) const
		{
			return ImGui::IsMouseDown( static_cast<ImGuiMouseButton>( button ) );
		}

		bool click_released( mouse_buttons button ) const
		{
			return ImGui::IsMouseReleased( static_cast<ImGuiMouseButton>( button ) );
		}

		float get_wheel_value( ) const
		{
			return ImGui::GetIO( ).MouseWheel;
		}

		float get_wheel_h( ) const
		{
			return ImGui::GetIO( ).MouseWheelH;
		}

		bool key_down( int vk ) const
		{
			if ( vk < 0 || vk >= 256 )
				return false;
			return m_key_down[vk];
		}

		bool key_pressed( int vk ) const
		{
			if ( vk < 0 || vk >= 256 )
				return false;
			return m_key_pressed[vk];
		}

		bool key_released( int vk ) const
		{
			if ( vk < 0 || vk >= 256 )
				return false;
			return m_key_released[vk];
		}

		bool any_key_pressed( ) const
		{
			for ( int i = 0; i < 256; ++i )
			{
				if ( m_key_pressed[i] )
					return true;
			}
			return false;
		}

		int get_pressed_key( ) const
		{
			for ( int i = 1; i < 256; ++i )
			{
				if ( m_key_pressed[i] )
					return i;
			}
			return 0;
		}

		std::string get_clipboard( ) const
		{
			const char* text = ImGui::GetClipboardText( );
			return text ? std::string( text ) : std::string {};
		}

		void set_clipboard( const std::string& text ) const
		{
			ImGui::SetClipboardText( text.c_str( ) );
		}

	private:
		bool m_key_down[256] {};
		bool m_key_pressed[256] {};
		bool m_key_released[256] {};
	};

	inline std::shared_ptr<c_input> g_input = std::make_shared<c_input>( );
}
