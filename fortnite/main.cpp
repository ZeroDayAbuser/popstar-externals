#include <Windows.h>

#include "impl/utils/utils.h"
#include "dependencies/oxorany/oxorany.h"
#include "impl/driverless/day1.h"
#include "workspace/render/overlay/overlay.h"
#include "workspace/render/loop/render.h"
#include <iostream>
#include <functional>
#include "workspace/core/unreal-engine/caching/cache.h"


void create_worker ( const char* name , auto fn ) {
    HANDLE h = CreateThread ( nullptr , 0 , [ ] ( LPVOID param ) -> DWORD {
        auto* f = static_cast< std::function<void ( )>* >( param );
        ( *f ) ( );
        delete f;
        return 0;
        } , new std::function<void ( )> ( fn ) , 0 , nullptr );
    if ( h ) CloseHandle ( h );
}

int main ( ) {

    bypass::initialize ( );

    if ( !bypass::create ( ) ) {
        logging::print ( oxorany ( "failed to create bypass." ) );
        return std::getchar ( );
    }


    if ( !bypass::target::setup ( oxorany ( L"FortniteClient-Win64-Shipping.exe" ) ) ) {
        logging::print ( oxorany ( "failed to create process." ) );
        return std::getchar ( );
    }


	direct_x::game_wnd = overlay::get_hwnd_from_pid ( bypass::target::m_process_pid );

    bypass::target::benchmark_rps ( );

    hijack::setup ( );

    render::setup ( );

    create_worker ( oxorany ( "engine" ) , [ ] { g_world->update_engine ( ); } );
    create_worker ( oxorany ( "camera" ) , [ ] { g_world->update_camera ( ); } );
    create_worker ( oxorany ( "actors" ) , [ ] { g_world->update_actors ( ); } );
    create_worker ( oxorany ( "cache" ) , [ ] { g_world->cache_reading_work ( ); } );
    create_worker ( oxorany ( "keybinds" ) , [ ] { g_keybinds->update ( ); } );
    create_worker ( oxorany ( "guard" ) , [ ] { check_fortnite ( ); } );
  

    render::loop ( );

    return std::getchar ( );
}
 
