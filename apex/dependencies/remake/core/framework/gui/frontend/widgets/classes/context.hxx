#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>

namespace core::gui
{
	class c_base_element;

	struct tab_data_t
	{
		std::string m_name {};
		std::vector<std::string> m_subtab {};
		int m_active_subtab {};
		std::string m_cur_subtab {};
		bool m_subtabs_present { false };
	};

	enum class focus_priority : int
	{
		none = 0,
		hover,
		interactive,
		persistent,
		modal
	};

	struct focus_entry_t
	{
		c_base_element* element { nullptr };
		focus_priority priority { focus_priority::none };
	};

	class c_context
	{
	public:
		bool m_open { true };
		bool m_dragging { false };
		bool m_click_consumed { false };
		float m_open_anim { 1.f };

		std::string m_cur_tab {};
		int m_active_tab {};
		std::vector<tab_data_t> m_tabs {};

		c_base_element* m_focus_took { nullptr };
		c_base_element* m_focus_took_by_popup { nullptr };
		c_base_element* m_modal_owner { nullptr };
		c_base_element* m_hovered { nullptr };
		std::vector<focus_entry_t> m_focus_stack {};
		std::function<bool( c_base_element* )> m_modal_contains {};
		struct modal_frame_t
		{
			c_base_element* owner { nullptr };
			std::function<bool( c_base_element* )> contains {};
		};
		std::vector<modal_frame_t> m_modal_stack {};

		void push_focus( c_base_element* element, focus_priority priority )
		{
			for ( const focus_entry_t& entry : m_focus_stack )
			{
				if ( entry.element == element )
					return;
			}
			m_focus_stack.push_back( { element, priority } );
		}

		void pop_focus( c_base_element* element )
		{
			m_focus_stack.erase(
				std::remove_if( m_focus_stack.begin( ), m_focus_stack.end( ),
					[element]( const focus_entry_t& e ) { return e.element == element; } ),
				m_focus_stack.end( ) );
		}

		bool is_focused( c_base_element* element ) const
		{
			for ( const focus_entry_t& e : m_focus_stack )
			{
				if ( e.element == element )
					return true;
			}
			return false;
		}

		c_base_element* top_focus( ) const
		{
			if ( m_focus_stack.empty( ) )
				return nullptr;
			return std::max_element( m_focus_stack.begin( ), m_focus_stack.end( ),
				[]( const focus_entry_t& a, const focus_entry_t& b )
				{
					return static_cast<int>( a.priority ) < static_cast<int>( b.priority );
				} )->element;
		}

		focus_priority top_priority( ) const
		{
			if ( m_focus_stack.empty( ) )
				return focus_priority::none;
			return std::max_element( m_focus_stack.begin( ), m_focus_stack.end( ),
				[]( const focus_entry_t& a, const focus_entry_t& b )
				{
					return static_cast<int>( a.priority ) < static_cast<int>( b.priority );
				} )->priority;
		}

		bool can_interact( c_base_element* element, focus_priority my_priority ) const
		{
			if ( m_modal_owner != nullptr )
			{
				if ( element == m_modal_owner )
					return true;
				if ( m_modal_contains && m_modal_contains( element ) )
					return true;
				return false;
			}

			c_base_element* top = top_focus( );
			if ( top == nullptr )
				return true;
			if ( top == element )
				return true;
			return static_cast<int>( my_priority ) > static_cast<int>( top_priority( ) );
		}

		void set_modal( c_base_element* owner, std::function<bool( c_base_element* )> contains = {} )
		{
			if ( owner && m_modal_owner && m_modal_owner != owner )
				m_modal_stack.push_back( { m_modal_owner, std::move( m_modal_contains ) } );

			m_modal_owner = owner;
			m_modal_contains = std::move( contains );
			if ( owner )
				push_focus( owner, focus_priority::modal );
		}

		void clear_modal( c_base_element* owner )
		{
			if ( m_modal_owner != owner )
			{
				pop_focus( owner );
				m_modal_stack.erase(
					std::remove_if( m_modal_stack.begin( ), m_modal_stack.end( ),
						[owner]( const modal_frame_t& f ) { return f.owner == owner; } ),
					m_modal_stack.end( ) );
				return;
			}

			pop_focus( owner );
			if ( !m_modal_stack.empty( ) )
			{
				modal_frame_t prev = std::move( m_modal_stack.back( ) );
				m_modal_stack.pop_back( );
				m_modal_owner = prev.owner;
				m_modal_contains = std::move( prev.contains );
			}
			else
			{
				m_modal_owner = nullptr;
				m_modal_contains = {};
			}
		}

		void clear_all_modals( )
		{
			if ( m_modal_owner )
				pop_focus( m_modal_owner );
			for ( const modal_frame_t& frame : m_modal_stack )
			{
				if ( frame.owner )
					pop_focus( frame.owner );
			}
			m_modal_stack.clear( );
			m_modal_owner = nullptr;
			m_modal_contains = {};
		}

		std::string get_key_name( int virtual_key ) const
		{
			UINT code = MapVirtualKeyA( static_cast<UINT>( virtual_key ), MAPVK_VK_TO_VSC );
			char buffer[128] {};
			int result = 0;

			switch ( virtual_key )
			{
			case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
			case VK_RCONTROL: case VK_RMENU:
			case VK_LWIN: case VK_RWIN: case VK_APPS:
			case VK_PRIOR: case VK_NEXT:
			case VK_END: case VK_HOME:
			case VK_INSERT: case VK_DELETE:
			case VK_DIVIDE:
			case VK_NUMLOCK:
				code |= KF_EXTENDED;
			default:
				result = GetKeyNameTextA( static_cast<LONG>( code << 16 ), buffer, 128 );
				break;
			}

			if ( result == 0 || virtual_key == VK_CAPITAL )
			{
				switch ( virtual_key )
				{
				case VK_XBUTTON1: return "M4";
				case VK_XBUTTON2: return "M5";
				case VK_LBUTTON: return "M1";
				case VK_MBUTTON: return "M3";
				case VK_RBUTTON: return "M2";
				case VK_CAPITAL: return "CAPS";
				default: return "NONE";
				}
			}

			std::string transformer( buffer );
			std::transform( transformer.begin( ), transformer.end( ), transformer.begin( ),
				[]( unsigned char c ) { return static_cast<char>( std::toupper( c ) ); } );
			return transformer;
		}
	};

	inline std::shared_ptr<c_context> g_ctx = std::make_shared<c_context>( );
}
