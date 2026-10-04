#pragma once

#include <dependencies/includes.h>
#include <src/driver/driver.cuh>

namespace data {
    struct _vector2 {
    public:
        float x , y , z;

        _vector2( float x = 0 , float y = 0 , float z = 0 )
            : x( x ) , y( y ) , z( z )
        {
        }

        inline bool operator==( const _vector2 & src ) const
        {
            return ( src.x == x ) && ( src.y == y ) && ( src.z == z );
        }
        inline bool operator!=( const _vector2 & src ) const
        {
            return ( src.x != x ) || ( src.y != y ) || ( src.z != z );
        }
        inline _vector2 & operator+=( const _vector2 & v )
        {
            x += v.x; y += v.y; z += v.z; return *this;
        }
        inline _vector2 & operator-=( const _vector2 & v )
        {
            x -= v.x; y -= v.y; z -= v.z; return *this;
        }
        inline _vector2 operator-( _vector2 ape ) { return { x - ape.x, y - ape.y, z - ape.z }; }
        inline _vector2 operator+( _vector2 ape ) { return { x + ape.x, y + ape.y, z + ape.z }; }
        inline _vector2 operator*( float ape ) { return { x * ape, y * ape, z * ape }; }
        inline _vector2 operator/( float ape ) { return { x / ape, y / ape, z / ape }; }

        inline float length( ) { return sqrt( ( x * x ) + ( y * y ) + ( z * z ) ); }
        inline float distance( _vector2 ape ) { return ( *this - ape ).length( ); }

        inline void clamp( )
        {
            if ( x > 180.0f ) y = 180.0f;
            else if ( y < -180.0f ) y = -180.0f;
            if ( x > 89.0f ) x = 89.0f;
            else if ( x < -89.0f ) x = -89.0f;
            z = 0;
        }

        inline _vector2 calc_angle( const _vector2 & src , const _vector2 & dst )
        {
            _vector2 delta = { src.x - dst.x, src.y - dst.y, src.z - dst.z };
            _vector2 angle = _vector2( );
            double hyp = sqrtf( ( delta.x * delta.x ) + ( delta.y * delta.y ) );
            angle.x = ( float ) ( atan( delta.z / hyp ) * 57.295779513082f );
            angle.y = ( float ) ( atan( delta.y / delta.x ) * 57.295779513082f );
            if ( delta.x >= 0.0 ) angle.y += 180.0f;
            return angle;
        }

        inline void normalize( )
        {
            while ( x > 89.0f )  x -= 180.f;
            while ( x < -89.0f ) x += 180.f;
            while ( y > 180.f )  y -= 360.f;
            while ( y < -180.f ) y += 360.f;
        }

        inline bool is_zero( )
        {
            return ( x > -0.1f && x < 0.1f && y > -0.1f && y < 0.1f && z > -0.1f && z < 0.1f );
        }
    };

    struct _vector3 {
    public:
        float x , y , z;

        _vector3( float x = 0 , float y = 0 , float z = 0 )
            : x( x ) , y( y ) , z( z )
        {
        }

        inline bool operator==( const _vector3 & src ) const
        {
            return ( src.x == x ) && ( src.y == y ) && ( src.z == z );
        }
        inline bool operator!=( const _vector3 & src ) const
        {
            return ( src.x != x ) || ( src.y != y ) || ( src.z != z );
        }
        inline _vector3 & operator+=( const _vector3 & v )
        {
            x += v.x; y += v.y; z += v.z; return *this;
        }
        inline _vector3 & operator-=( const _vector3 & v )
        {
            x -= v.x; y -= v.y; z -= v.z; return *this;
        }
        inline _vector3 operator-( const _vector3 & v ) const { return { x - v.x, y - v.y, z - v.z }; }
        inline _vector3 operator+( const _vector3 & v ) const { return { x + v.x, y + v.y, z + v.z }; }
        inline _vector3 operator*( float f )            const { return { x * f, y * f, z * f }; }
        inline _vector3 operator/( float f )            const { return { x / f, y / f, z / f }; }

        inline float dot( const _vector3 & v )   const { return x * v.x + y * v.y + z * v.z; }
        inline float length( )                   const { return sqrtf( x * x + y * y + z * z ); }
        inline float length_2d( )                const { return sqrtf( x * x + y * y ); }
        inline float distance( const _vector3 & v ) const { return ( *this - v ).length( ); }

        inline _vector3 cross( const _vector3 & v ) const
        {
            return { y * v.z - z * v.y,
                     z * v.x - x * v.z,
                     x * v.y - y * v.x };
        }

        inline _vector3 normalized( ) const
        {
            float len = length( );
            if ( len == 0.f ) return {};
            return { x / len, y / len, z / len };
        }

        inline bool is_zero( ) const
        {
            return ( x > -0.1f && x < 0.1f && y > -0.1f && y < 0.1f && z > -0.1f && z < 0.1f );
        }

        inline void clamp( )
        {
            if ( x > 180.0f ) y = 180.0f;
            else if ( y < -180.0f ) y = -180.0f;
            if ( x > 89.0f ) x = 89.0f;
            else if ( x < -89.0f ) x = -89.0f;
            z = 0;
        }
    };

    struct _vector4 {
    public:
        float x , y , z , w;

        _vector4( float x = 0 , float y = 0 , float z = 0 , float w = 0 )
            : x( x ) , y( y ) , z( z ) , w( w )
        {
        }

        inline bool operator==( const _vector4 & src ) const
        {
            return ( src.x == x ) && ( src.y == y ) && ( src.z == z ) && ( src.w == w );
        }
        inline bool operator!=( const _vector4 & src ) const
        {
            return ( src.x != x ) || ( src.y != y ) || ( src.z != z ) || ( src.w != w );
        }
        inline _vector4 & operator+=( const _vector4 & v )
        {
            x += v.x; y += v.y; z += v.z; w += v.w; return *this;
        }
        inline _vector4 & operator-=( const _vector4 & v )
        {
            x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this;
        }
        inline _vector4 operator+( const _vector4 & v ) const { return { x + v.x, y + v.y, z + v.z, w + v.w }; }
        inline _vector4 operator-( const _vector4 & v ) const { return { x - v.x, y - v.y, z - v.z, w - v.w }; }
        inline _vector4 operator*( float f )           const { return { x * f, y * f, z * f, w * f }; }
        inline _vector4 operator/( float f )           const { return { x / f, y / f, z / f, w / f }; }

        inline float dot( const _vector4 & v ) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
        inline float length( )                 const { return sqrtf( x * x + y * y + z * z + w * w ); }

        inline _vector4 normalized( ) const
        {
            float len = length( );
            if ( len == 0.f ) return {};
            return { x / len, y / len, z / len, w / len };
        }

        inline _vector3 xyz( ) const { return { x, y, z }; }

        inline bool is_zero( ) const
        {
            return ( x > -0.1f && x < 0.1f && y > -0.1f && y < 0.1f &&
                z > -0.1f && z < 0.1f && w > -0.1f && w < 0.1f );
        }
    };

    struct _cframe {
    public:
        float m [ 4 ][ 4 ];

        _cframe( )
        {
            // identity
            for ( int i = 0; i < 4; i++ )
                for ( int j = 0; j < 4; j++ )
                    m [ i ][ j ] = ( i == j ) ? 1.f : 0.f;
        }

        _cframe( const _vector3 & pos ) : _cframe( )
        {
            m [ 0 ][ 3 ] = pos.x;
            m [ 1 ][ 3 ] = pos.y;
            m [ 2 ][ 3 ] = pos.z;
        }

        _cframe( const _vector3 & pos , const _vector3 & right , const _vector3 & up , const _vector3 & fwd )
        {
            m [ 0 ][ 0 ] = right.x; m [ 0 ][ 1 ] = right.y; m [ 0 ][ 2 ] = right.z; m [ 0 ][ 3 ] = pos.x;
            m [ 1 ][ 0 ] = up.x;    m [ 1 ][ 1 ] = up.y;    m [ 1 ][ 2 ] = up.z;    m [ 1 ][ 3 ] = pos.y;
            m [ 2 ][ 0 ] = fwd.x;   m [ 2 ][ 1 ] = fwd.y;   m [ 2 ][ 2 ] = fwd.z;   m [ 2 ][ 3 ] = pos.z;
            m [ 3 ][ 0 ] = 0.f;     m [ 3 ][ 1 ] = 0.f;     m [ 3 ][ 2 ] = 0.f;     m [ 3 ][ 3 ] = 1.f;
        }

        inline float * operator[]( int i ) { return m [ i ]; }
        inline const float * operator[]( int i ) const { return m [ i ]; }

        inline _vector3 position( ) const { return { m [ 0 ][ 3 ], m [ 1 ][ 3 ], m [ 2 ][ 3 ] }; }
        inline _vector3 right( )   const { return { m [ 0 ][ 0 ], m [ 0 ][ 1 ], m [ 0 ][ 2 ] }; }
        inline _vector3 up( )      const { return { m [ 1 ][ 0 ], m [ 1 ][ 1 ], m [ 1 ][ 2 ] }; }
        inline _vector3 forward( ) const { return { m [ 2 ][ 0 ], m [ 2 ][ 1 ], m [ 2 ][ 2 ] }; }

        inline _vector3 transform_point( const _vector3 & v ) const
        {
            return {
                m [ 0 ][ 0 ] * v.x + m [ 0 ][ 1 ] * v.y + m [ 0 ][ 2 ] * v.z + m [ 0 ][ 3 ],
                m [ 1 ][ 0 ] * v.x + m [ 1 ][ 1 ] * v.y + m [ 1 ][ 2 ] * v.z + m [ 1 ][ 3 ],
                m [ 2 ][ 0 ] * v.x + m [ 2 ][ 1 ] * v.y + m [ 2 ][ 2 ] * v.z + m [ 2 ][ 3 ]
            };
        }

        inline _vector3 transform_direction( const _vector3 & v ) const
        {
            return {
                m [ 0 ][ 0 ] * v.x + m [ 0 ][ 1 ] * v.y + m [ 0 ][ 2 ] * v.z,
                m [ 1 ][ 0 ] * v.x + m [ 1 ][ 1 ] * v.y + m [ 1 ][ 2 ] * v.z,
                m [ 2 ][ 0 ] * v.x + m [ 2 ][ 1 ] * v.y + m [ 2 ][ 2 ] * v.z
            };
        }

        inline _cframe operator*( const _cframe & o ) const
        {
            _cframe r;
            for ( int i = 0; i < 4; i++ )
                //
                for ( int j = 0; j < 4; j++ )
                {
                    r.m [ i ][ j ] = 0.f;
                    for ( int k = 0; k < 4; k++ )
                        r.m [ i ][ j ] += m [ i ][ k ] * o.m [ k ][ j ];
                }
            return r;
        }

        inline _cframe inverse( ) const
        {
            _cframe r;

            for ( int i = 0; i < 3; i++ )
                for ( int j = 0; j < 3; j++ )
                    r.m [ i ][ j ] = m [ j ][ i ];
            //
            r.m [ 0 ][ 3 ] = -( r.m [ 0 ][ 0 ] * m [ 0 ][ 3 ] + r.m [ 0 ][ 1 ] * m [ 1 ][ 3 ] + r.m [ 0 ][ 2 ] * m [ 2 ][ 3 ] );
            r.m [ 1 ][ 3 ] = -( r.m [ 1 ][ 0 ] * m [ 0 ][ 3 ] + r.m [ 1 ][ 1 ] * m [ 1 ][ 3 ] + r.m [ 1 ][ 2 ] * m [ 2 ][ 3 ] );
            r.m [ 2 ][ 3 ] = -( r.m [ 2 ][ 0 ] * m [ 0 ][ 3 ] + r.m [ 2 ][ 1 ] * m [ 1 ][ 3 ] + r.m [ 2 ][ 2 ] * m [ 2 ][ 3 ] );
            r.m [ 3 ][ 0 ] = r.m [ 3 ][ 1 ] = r.m [ 3 ][ 2 ] = 0.f;
            r.m [ 3 ][ 3 ] = 1.f;
            return r;
        }

        inline bool is_identity( ) const
        {
            for ( int i = 0; i < 4; i++ )
                for ( int j = 0; j < 4; j++ )
                    if ( m [ i ][ j ] != ( ( i == j ) ? 1.f : 0.f ) ) return false;
            return true;
        }
    };

    struct _matrix {
        float _matrix [ 16 ];
    };

    struct _matrix3x4 {
        float m [ 3 ][ 4 ];

        inline float * operator[]( int i ) { return m [ i ]; }
        inline const float * operator[]( int i ) const { return m [ i ]; }
        inline float        at( int row , int col ) const { return m [ row ][ col ]; }
        inline float * base( ) { return &m [ 0 ][ 0 ]; }
        inline const float * base( ) const { return &m [ 0 ][ 0 ]; }
    };

    enum class e_bone_type {
        head = 0 ,
        neck = 1 ,
        upper_chest = 2 ,
        lower_chest = 3 ,
        stomach = 4 ,
        pelvis = 5 ,
        left_shoulder = 6 ,
        left_elbow = 7 ,
        left_hand = 8 ,
        right_shoulder = 9 ,
        right_elbow = 10 ,
        right_hand = 11 ,
        left_hip = 12 ,
        left_knee = 13 ,
        left_foot = 14 ,
        right_hip = 16 ,
        right_knee = 17 ,
        right_foot = 18 ,
    };

    struct weapon_info_t
    {
        float bullet_speed { 0.f };
        std::uint8_t pad[ 8 ] { };
        float bullet_scale { 0.f };
    };

    inline auto angle_to_forward( const data::_vector3& angles ) -> data::_vector3
    {
        const float pitch = angles.x * ( 3.14159265f / 180.f );
        const float yaw = angles.y * ( 3.14159265f / 180.f );
        const float cp = cosf( pitch );
        return {
            cp * cosf( yaw ) ,
            cp * sinf( yaw ) ,
            -sinf( pitch )
        };
    }

    inline auto bullet_trajectory( const data::_vector3 & shoot_from , const data::_vector3 & target , const data::_vector3 & velocity , float bullet_speed , float bullet_scale ) -> data::_vector3
    {
        if ( bullet_speed < 1.f )
            return target;

        auto grav = 750.f * bullet_scale;
        auto aim = target;

        for ( int i = 0; i < 5; i++ )
        {
            auto dist = aim.distance( shoot_from );
            auto t = dist / bullet_speed;
            aim = target + velocity * t;
            aim.z += 0.5f * grav * t * t;
        }

        return aim;
    }

    inline int width_ = GetSystemMetrics( SM_CXSCREEN );
    inline int height_ = GetSystemMetrics( SM_CYSCREEN );

    inline auto dimensions( ) -> ImVec2
    {
        return ImVec2( static_cast<float>( width_ ) , static_cast<float>( height_ ) );
    }

    inline auto on_screen( const data::_vector2& p ) -> bool
    {
        return p.x > 0.f && p.y > 0.f && p.x < width_ && p.y < height_;
    }

    inline auto dist2( const data::_vector3& a , const data::_vector3& b ) -> float
    {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        const float dz = a.z - b.z;
        return dx * dx + dy * dy + dz * dz;
    }

    inline auto get_view_matrix( ) -> _matrix
    {
        auto vrender = hypervisor->read<std::uint64_t>( hypervisor->m_base_address + offsets::view_render );
        if ( !vrender ) return { };

        auto vmatrix = hypervisor->read<std::uint64_t>( vrender + offsets::view_matrix );
        if ( !vmatrix ) return { };

        return hypervisor->read<_matrix>( vmatrix );
    }

    inline auto world_to_screen( const data::_vector3 pos ) -> data::_vector2
    {
        data::_vector2 to = data::_vector2( );
        auto m_vmatrix = get_view_matrix( )._matrix;

        float w = m_vmatrix [ 12 ] * pos.x + m_vmatrix [ 13 ] * pos.y
            + m_vmatrix [ 14 ] * pos.z + m_vmatrix [ 15 ];

        if ( w < 0.01f ) return data::_vector2( );

        float invw = 1.0f / w;
        float nx = ( m_vmatrix [ 0 ] * pos.x + m_vmatrix [ 1 ] * pos.y + m_vmatrix [ 2 ] * pos.z + m_vmatrix [ 3 ] ) * invw;
        float ny = ( m_vmatrix [ 4 ] * pos.x + m_vmatrix [ 5 ] * pos.y + m_vmatrix [ 6 ] * pos.z + m_vmatrix [ 7 ] ) * invw;

        if ( !std::isfinite( nx ) || !std::isfinite( ny ) ) return data::_vector2( );
        if ( fabsf( nx ) > 2.5f || fabsf( ny ) > 2.5f ) return data::_vector2( );

        float x = width_ / 2;
        float y = height_ / 2;

        x += 0.5f * nx * width_ + 0.5f;
        y -= 0.5f * ny * height_ + 0.5f;

        if ( !std::isfinite( x ) || !std::isfinite( y ) ) return data::_vector2( );

        to.x = x; to.y = y; to.z = 0;

        return to;
    }
}