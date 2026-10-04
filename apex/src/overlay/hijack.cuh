#pragma once

#include <Windows.h>
#include <memory>

namespace overlay
{
	class c_hijack
	{
	public:
		HWND m_hwnd{ };

		auto hook( HWND game ) -> bool;
		static auto game_client_bounds( HWND game , int& x , int& y , int& w , int& h ) -> bool;

	private:
		auto nvidia( ) -> HWND;
		auto discord( ) -> HWND;
		auto style( HWND hwnd , int x , int y , int w , int h ) -> vi;
	};
}

inline auto hijack = std::make_shared<overlay::c_hijack>( );
