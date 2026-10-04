#include <dependencies/includes.h>
#include <src/overlay/hijack.cuh>

auto overlay::c_hijack::nvidia( ) -> HWND
{
	return ::FindWindowA( "CEF-OSC-WIDGET" , "NVIDIA GeForce Overlay" );
}

auto overlay::c_hijack::discord( ) -> HWND
{
	HWND hwnd = ::FindWindowA( "Chrome_WidgetWin_1" , "Discord Overlay" );
	if ( hwnd )
		return hwnd;

	for ( int i = 0; i < 10 && !hwnd; ++i )
	{
		std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
		hwnd = ::FindWindowA( "Chrome_WidgetWin_1" , "Discord Overlay" );
	}

	return hwnd;
}

auto overlay::c_hijack::game_client_bounds( HWND game , int& x , int& y , int& w , int& h ) -> bool
{
	if ( !game || !::IsWindow( game ) )
		return false;

	RECT cr {};
	if ( !::GetClientRect( game , &cr ) )
		return false;

	POINT tl { cr.left , cr.top };
	POINT br { cr.right , cr.bottom };
	::ClientToScreen( game , &tl );
	::ClientToScreen( game , &br );

	x = tl.x;
	y = tl.y;
	w = br.x - tl.x;
	h = br.y - tl.y;
	return w > 8 && h > 8;
}

auto overlay::c_hijack::style( HWND hwnd , int x , int y , int w , int h ) -> vi
{
	::SetWindowLongA( hwnd , GWL_EXSTYLE , WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED );
	::MoveWindow( hwnd , x , y , w , h , TRUE );

	MARGINS margin = { -1 , -1 , -1 , -1 };
	::DwmExtendFrameIntoClientArea( hwnd , &margin );

	::SetLayeredWindowAttributes( hwnd , 0 , 255 , LWA_ALPHA );
	::SetWindowPos( hwnd , HWND_TOPMOST , x , y , w , h ,
		SWP_NOACTIVATE | SWP_SHOWWINDOW );
	::ShowWindow( hwnd , SW_SHOWNOACTIVATE );
	::UpdateWindow( hwnd );
}

auto overlay::c_hijack::hook( HWND game ) -> bool
{
	if ( !game || !::IsWindow( game ) )
	{
		logger->print( "hijack -> wrong window" );
		return false;
	}

	int x = 0 , y = 0 , w = 0 , h = 0;
	if ( !game_client_bounds( game , x , y , w , h ) )
	{
		w = ::GetSystemMetrics( SM_CXSCREEN );
		h = ::GetSystemMetrics( SM_CYSCREEN );
		x = 0;
		y = 0;
	}

	const char* name = "nvidia";
	m_hwnd = nvidia( );

	if ( !m_hwnd )
	{
		logger->print( "nvidia -> overlay not found" );
		logger->print( "discord -> falling back" );

		name = "discord";
		m_hwnd = discord( );

		if ( !m_hwnd )
		{
			logger->print( "discord -> overlay not found" );
			return false;
		}
	}

	style( m_hwnd , x , y , w , h );

	window_handle = m_hwnd;
	game_hwnd = game;
	data::width_ = w;
	data::height_ = h;

	if ( !g_overlay->setup( m_hwnd ) )
	{
		logger->print( "%s -> d3d bind failed" , name );
		return false;
	}

	logger->print( "%s -> hooked (%dx%d @ %d,%d)" , name , w , h , x , y );
	g_overlay->overlay( );
	return true;
}
