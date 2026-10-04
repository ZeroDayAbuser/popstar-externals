#pragma once

#include <Windows.h>

namespace overlay {

    HWND find_window ( const char* className , const char* windowName ) {
        HWND result = nullptr;

        struct EnumData {
            const char* className;
            const char* windowName;
            HWND result;
        };

        EnumData data;
        data.className = className;
        data.windowName = windowName;
        data.result = nullptr;

        EnumWindows ( [ ] ( HWND hwnd , LPARAM lParam ) -> BOOL {
            auto* pData = reinterpret_cast< EnumData* >( lParam );

            char windowClass [ 256 ];
            char windowTitle [ 256 ];

            GetClassNameA ( hwnd , windowClass , sizeof ( windowClass ) );
            GetWindowTextA ( hwnd , windowTitle , sizeof ( windowTitle ) );

            bool classMatch = ( pData->className == nullptr ||
                strcmp ( windowClass , pData->className ) == 0 );
            bool titleMatch = ( pData->windowName == nullptr ||
                strcmp ( windowTitle , pData->windowName ) == 0 );

            if ( classMatch && titleMatch ) {
                pData->result = hwnd;
                return FALSE;
            }

            return TRUE;
            } , reinterpret_cast< LPARAM >( &data ) );

        return data.result;
    }

    HWND get_hwnd_from_pid ( DWORD pid ) {
        HWND result = nullptr;
        struct Data {
            DWORD pid;
            HWND* hwnd;
        } data = { pid, &result };

        EnumWindows ( [ ] ( HWND h , LPARAM lp ) -> BOOL {
            Data* data = reinterpret_cast< Data* >( lp );
            DWORD window_pid;
            GetWindowThreadProcessId ( h , &window_pid );
            if ( window_pid == data->pid && IsWindowVisible ( h ) ) {
                *( data->hwnd ) = h;
                return FALSE;
            }
            return TRUE;
            } , reinterpret_cast< LPARAM >( &data ) );

        return result;
    }


}

