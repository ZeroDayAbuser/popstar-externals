#pragma once

#include <mutex>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <string>

namespace logging {
    struct rgb_t { int r , g , b; };

    // Helper functions
    inline rgb_t lerp_rgb ( const rgb_t& a , const rgb_t& b , float t ) {
        if ( t <= 0.0f ) return a;
        if ( t >= 1.0f ) return b;
        return {
            a.r + ( int ) ( ( b.r - a.r ) * t ),
            a.g + ( int ) ( ( b.g - a.g ) * t ),
            a.b + ( int ) ( ( b.b - a.b ) * t )
        };
    }

    inline float smooth_step ( float t ) {
        if ( t <= 0.0f ) return 0.0f;
        if ( t >= 1.0f ) return 1.0f;
        return t * t * ( 3.0f - 2.0f * t );
    }

    template<typename... Args>
    inline void print ( const char* format , Args... args ) {
        static std::mutex print_mutex;
        std::lock_guard<std::mutex> lock ( print_mutex );
        static constexpr rgb_t k_grad_a = { 7, 62, 100 };
        static constexpr rgb_t k_grad_b = { 44, 187, 232 };
        static constexpr rgb_t k_grad_c = { 16, 130, 198 };

        std::time_t now = std::time ( nullptr );
        std::tm local_tm {};
        localtime_s ( &local_tm , &now );
        char ts [ 32 ] = {};
        std::strftime ( ts , sizeof ( ts ) , "%H:%M:%S %d/%m/%Y" , &local_tm );

        char msg [ 4096 ];
        int n = snprintf ( msg , sizeof ( msg ) , format , args... );
        if ( n < 0 || n >= ( int ) sizeof ( msg ) ) {
            const std::string prefix = std::string ( "[ " ) + ts + (" ] [ emulated.vip ]: ");
            const size_t total = prefix.size ( );
            for ( size_t i = 0; i < total; ++i ) {
                float t = ( total > 1 ) ? ( float ) i / ( float ) ( total - 1 ) : 0.0f;
                rgb_t c = {};
                if ( t < 0.5f ) {
                    c = lerp_rgb ( k_grad_a , k_grad_b , smooth_step ( t * 2.0f ) );
                }
                else {
                    c = lerp_rgb ( k_grad_b , k_grad_c , smooth_step ( ( t - 0.5f ) * 2.0f ) );
                }
                printf ( "\x1b[38;2;%d;%d;%dm%c" , c.r , c.g , c.b , prefix [ i ] );
            }
            printf ( format , args... );
            printf ( "\x1b[0m\n" );
            return;
        }

        size_t len = strlen ( msg );
        while ( len && ( msg [ len - 1 ] == '\n' || msg [ len - 1 ] == '\r' ) ) msg [ --len ] = '\0';

        const std::string prefix = std::string ( "[ " ) + ts + " ] [ emulated.vip ]: ";
        const size_t total = prefix.size ( );
        for ( size_t i = 0; i < total; ++i ) {
            float t = ( total > 1 ) ? ( float ) i / ( float ) ( total - 1 ) : 0.0f;
            rgb_t c = {};
            if ( t < 0.5f ) {
                c = lerp_rgb ( k_grad_a , k_grad_b , smooth_step ( t * 2.0f ) );
            }
            else {
                c = lerp_rgb ( k_grad_b , k_grad_c , smooth_step ( ( t - 0.5f ) * 2.0f ) );
            }
            printf ( "\x1b[38;2;%d;%d;%dm%c" , c.r , c.g , c.b , prefix [ i ] );
        }
        printf ( "\x1b[97m%s" , msg );
        printf ( "\x1b[0m\n" );
    }
}