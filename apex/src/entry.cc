#include <dependencies/includes.h>
#include <src/driver/driver.cuh>
#include <src/cheat/aimbot/aimbot.cuh>
#include <src/cheat/heirloom/heirloom.cuh>
#include <src/overlay/hijack.cuh>
#include <cstdio>

namespace
{
	struct console_hold_t
	{
		~console_hold_t( )
		{
			std::fflush( stdout );
			std::fflush( stderr );
			std::fputs( "\npress enter to exit\n" , stdout );
			std::fflush( stdout );
			( void )std::getchar( );
		}
	};
}

auto main( std::int32_t argc , std::int8_t** argv ) -> std::int32_t
{
	( void )argc;
	( void )argv;

	console_hold_t hold_console {};

	logger->setup( "dont hug me im scared" );

	if ( !hypervisor->setup( ) )
	{
		logger->print( "hypervisor -> not found" );
		return -1;
	}

	logger->print( "hypervisor -> initialized" );
	logger->print( "waiting for apex window" );

	HWND game_wnd = nullptr;
	while ( !( game_wnd = FindWindowA( nullptr , "Apex Legends" ) ) )
		std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );

	logger->print( "apex found" );

	if ( !hypervisor->attach( L"r5apex_dx12.exe" ) )
	{
		logger->print( "failed to attach to apex" );
		return -1;
	}

	logger->print( "we're in the proc -> %lu" , hypervisor->m_pid );
	logger->print( "base addy -> 0x%llx" , hypervisor->m_base_address );
	logger->print( "greetings cr3 -> 0x%llx" , hypervisor->m_cr3 );

	const auto base = hypervisor->m_base_address;

	auto dump_abs = [ & ]( const char* name , std::int32_t off )
	{
		logger->print( "%s -> 0x%llx" , name , base + static_cast<std::uint64_t>( off ) );
	};

	dump_abs( "entity_list" , offsets::entity_list );
	dump_abs( "local_player" , offsets::local_player );
	dump_abs( "observer_list" , offsets::observer_list );
	dump_abs( "name_list" , offsets::name_list );
	dump_abs( "view_render" , offsets::view_render );
	dump_abs( "cl_fov_scale" , offsets::cl_fov_scale );

	const auto view_render_ptr = hypervisor->read<std::uint64_t>( base + offsets::view_render );
	logger->print( "view_render.ptr -> 0x%llx" , view_render_ptr );
	if ( view_render_ptr )
		logger->print( "view_matrix -> 0x%llx" , view_render_ptr + offsets::view_matrix );
	else
		logger->print( "view_matrix -> unresolved (view_render null), rva 0x%x" , offsets::view_matrix );

	const auto local = hypervisor->read<std::uint64_t>( base + offsets::local_player );
	logger->print( "local_player.ptr -> 0x%llx" , local );
	if ( local )
	{
		logger->print( "camera_origin -> 0x%llx" , local + offsets::off_camera_origin );
		logger->print( "m_i_health -> 0x%llx" , local + offsets::m_i_health );
		logger->print( "m_i_max_health -> 0x%llx" , local + offsets::m_i_max_health );
		logger->print( "m_shield_health -> 0x%llx" , local + offsets::m_shield_health );
		logger->print( "last_visible_time -> 0x%llx" , local + offsets::last_visible_time );
		logger->print( "m_viewangle -> 0x%llx" , local + offsets::off_view_angles );
		logger->print( "off_bone_array -> 0x%llx" , local + offsets::off_bone_array );
	}
	else
	{
		logger->print( "camera_origin rva -> 0x%x" , offsets::off_camera_origin );
		logger->print( "m_i_health rva -> 0x%x" , offsets::m_i_health );
		logger->print( "m_i_max_health rva -> 0x%x" , offsets::m_i_max_health );
		logger->print( "m_shield_health rva -> 0x%x" , offsets::m_shield_health );
		logger->print( "last_visible_time rva -> 0x%x" , offsets::last_visible_time );
		logger->print( "m_viewangle rva -> 0x%x" , offsets::off_view_angles );
		logger->print( "off_bone_array rva -> 0x%x" , offsets::off_bone_array );
	}
	
	std::thread( [ ] { cache->tick( ); } ).detach( );
	std::thread( [ ] { aimbot::tick( ); } ).detach( );
	std::thread( [ ] { heirloom::tick( ); } ).detach( );

	if ( !hijack->hook( game_wnd ) )
		return -1;

	return 0;
}