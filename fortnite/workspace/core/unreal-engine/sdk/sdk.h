#pragma once

#include <Windows.h>
#include <cmath>
#include <cstdint>
#include <random>
#include <chrono>
#include <unordered_map>
#include <array>
#include <algorithm>
#include <cfloat>
#include <d3d9.h>
#include "../../impl/driverless/day1.h"
#include "../memory/offsets.h"
#include <imgui.h>
#define M_PI 3.14159265358979323846264338327950288419716939937510


inline int width_sdk = GetSystemMetrics ( SM_CXSCREEN );
inline int height_sdk = GetSystemMetrics ( SM_CYSCREEN );

namespace prediction {
	inline float projectile_speed;
	inline float projectile_gravity;
}





inline constexpr float DegToRad ( float val ) {
	return val * ( M_PI / 180.f );
}

inline constexpr float RadToDeg ( float val ) {
	return val * ( 180.f / M_PI );
}

class Vector2
{
public:
	double x , y;

	Vector2 ( ) : x ( 0.0 ) , y ( 0.0 ) { }
	Vector2 ( double _x , double _y ) : x ( _x ) , y ( _y ) { }
	~Vector2 ( ) { }

	// subtraction
	Vector2 operator-( const Vector2& v ) const
	{
		return Vector2 ( x - v.x , y - v.y );
	}

	// addition
	Vector2 operator+( const Vector2& v ) const
	{
		return Vector2 ( x + v.x , y + v.y );
	}

	// multiply
	Vector2 operator*( double s ) const
	{
		return Vector2 ( x * s , y * s );
	}

	// divide
	Vector2 operator/( double s ) const
	{
		return Vector2 ( x / s , y / s );
	}

	Vector2 Normalize ( )
	{
		if ( x > 89.0 )
			x = 89.0;
		else if ( x < -89.0 )
			x = -89.0;

		while ( y > 180.0 ) y -= 360.0;
		while ( y < -180.0 ) y += 360.0;

		if ( y > 179.0 )
			y = 179.0;
		else if ( y < -179.0 )
			y = -179.0;

		return *this;
	}

	bool is_valid ( ) const
	{
		return x != 0.0 || y != 0.0;
	}

	ImVec2 vec ( ) const
	{
		return ImVec2 ( ( float ) x , ( float ) y );
	}
};

struct Vector3
{
	Vector3 ( ) : x ( ) , y ( ) , z ( ) { }
	Vector3 ( double x , double y , double z ) : x ( x ) , y ( y ) , z ( z ) { }

	Vector3 operator + ( const Vector3& other ) const { return { this->x + other.x, this->y + other.y, this->z + other.z }; }
	Vector3 operator - ( const Vector3& other ) const { return { this->x - other.x, this->y - other.y, this->z - other.z }; }
	Vector3 operator * ( double offset ) const { return { this->x * offset, this->y * offset, this->z * offset }; }
	Vector3 operator / ( double offset ) const { return { this->x / offset, this->y / offset, this->z / offset }; }
	Vector3 operator * ( const Vector3& other ) const { return { this->x * other.x, this->y * other.y, this->z * other.z }; }
	Vector3 operator / ( const Vector3& other ) const { return { this->x / other.x, this->y / other.y, this->z / other.z }; }

	Vector3& operator *= ( const double other ) { this->x *= other; this->y *= other; this->z *= other; return *this; }
	Vector3& operator /= ( const double other ) { this->x /= other; this->y /= other; this->z /= other; return *this; }

	Vector3& operator = ( const Vector3& other ) { this->x = other.x; this->y = other.y; this->z = other.z; return *this; }
	Vector3& operator += ( const Vector3& other ) { this->x += other.x; this->y += other.y; this->z += other.z; return *this; }
	Vector3& operator -= ( const Vector3& other ) { this->x -= other.x; this->y -= other.y; this->z -= other.z; return *this; }
	Vector3& operator *= ( const Vector3& other ) { this->x *= other.x; this->y *= other.y; this->z *= other.z; return *this; }
	Vector3& operator /= ( const Vector3& other ) { this->x /= other.x; this->y /= other.y; this->z /= other.z; return *this; }

	operator bool ( ) { return bool ( this->x || this->y || this->z ); }
	friend bool operator == ( const Vector3& a , const Vector3& b ) { return a.x == b.x && a.y == b.y && a.z == b.z; }
	friend bool operator != ( const Vector3& a , const Vector3& b ) { return !( a == b ); }

	double Dot ( const Vector3& V ) { return x * V.x + y * V.y + z * V.z; }
	double SizeSquared ( ) { return x * x + y * y + z * z; }

	inline double Distance ( Vector3 v )
	{
		return double ( sqrtf ( powf ( v.x - x , 2.0 ) + powf ( v.y - y , 2.0 ) + powf ( v.z - z , 2.0 ) ) );
	}

	Vector3 operator*( float scalar ) const {
		return Vector3 ( x * scalar , y * scalar , z * scalar );
	}


	double Dot1 ( const Vector3& Other ) const
	{
		return x * Other.x + y * Other.y + z * Other.z;
	}

	// Alternative name (if you prefer "Scalar" instead of "Dot")
	double Scalar ( const Vector3& Other ) const
	{
		return Dot1 ( Other );

	}
	void Normalize ( )
	{
		while ( this->x > 180.0f )
		{
			this->x -= 360.0f;
		}
		while ( this->x < -180.0f )
		{
			this->x += 360.0f;
		}
		while ( this->y > 180.0f )
		{
			this->y -= 360.0f;
		}
		while ( this->y < -180.0f )
		{
			this->y += 360.0f;
		}

		this->z = 0;

	}

	Vector3 normalized ( ) const {
		auto len = sqrt ( x * x + y * y + z * z );
		if ( len < 1e-8 ) {
			return { 0.0, 0.0, 0.0 };
		}

		return { x / len, y / len, z / len };
	}

	bool is_valid ( ) const { return this->x && this->y && this->z; }


	void add_scale ( const Vector3& v , float scale ) {
		x += v.x * scale; y += v.y * scale; z += v.z * scale;
	}

	void Clamp ( float Min , float Max )
	{
		if ( this->x < Min )
		{
			this->x = Min;
		}
		else if ( this->x > Max )
		{
			this->x = Max;
		}

		if ( this->y < Min )
		{
			this->y = Min;
		}
		else if ( this->y > Max )
		{
			this->y = Max;
		}

		this->z = 0;
	}

	inline double length ( ) const {
		return sqrt ( x * x + y * y + z * z );
	}

	bool InScreen ( ) {
		if ( x > 0 && x < width_sdk && y > 0 && y < height_sdk )
			return true;
		else
			return false;
	}

	double x , y , z;
};




struct FRotator
{
	FRotator ( ) : Pitch ( ) , Yaw ( ) , Roll ( ) { }
	FRotator ( double Pitch , double Yaw , double Roll ) : Pitch ( Pitch ) , Yaw ( Yaw ) , Roll ( Roll ) { }

	FRotator operator + ( const FRotator& other ) const { return { this->Pitch + other.Pitch, this->Yaw + other.Yaw, this->Roll + other.Roll }; }
	FRotator operator - ( const FRotator& other ) const { return { this->Pitch - other.Pitch, this->Yaw - other.Yaw, this->Roll - other.Roll }; }
	FRotator operator * ( double offset ) const { return { this->Pitch * offset, this->Yaw * offset, this->Roll * offset }; }
	FRotator operator / ( double offset ) const { return { this->Pitch / offset, this->Yaw / offset, this->Roll / offset }; }

	FRotator& operator *= ( const double other ) { this->Pitch *= other; this->Yaw *= other; this->Roll *= other; return *this; }
	FRotator& operator /= ( const double other ) { this->Pitch /= other; this->Yaw /= other; this->Roll /= other; return *this; }

	FRotator& operator = ( const FRotator& other ) { this->Pitch = other.Pitch; this->Yaw = other.Yaw; this->Roll = other.Roll; return *this; }
	FRotator& operator += ( const FRotator& other ) { this->Pitch += other.Pitch; this->Yaw += other.Yaw; this->Roll += other.Roll; return *this; }
	FRotator& operator -= ( const FRotator& other ) { this->Pitch -= other.Pitch; this->Yaw -= other.Yaw; this->Roll -= other.Roll; return *this; }
	FRotator& operator *= ( const FRotator& other ) { this->Pitch *= other.Pitch; this->Yaw *= other.Yaw; this->Roll *= other.Roll; return *this; }
	FRotator& operator /= ( const FRotator& other ) { this->Pitch /= other.Pitch; this->Yaw /= other.Yaw; this->Roll /= other.Roll; return *this; }

	operator bool ( ) { return bool ( this->Pitch || this->Yaw || this->Roll ); }
	friend bool operator == ( const FRotator& a , const FRotator& b ) { return a.Pitch == b.Pitch && a.Yaw == b.Yaw && a.Roll == b.Roll; }
	friend bool operator != ( const FRotator& a , const FRotator& b ) { return !( a == b ); }

	Vector3 Euler ( ) const
	{
		return Vector3 ( Pitch , Yaw , Roll );
	}

	Vector3 GetForwardVector ( ) const
	{
		const double RadPitch = Pitch * ( M_PI / 180.0 );
		const double RadYaw = Yaw * ( M_PI / 180.0 );

		const double CosPitch = cos ( RadPitch );
		const double SinPitch = sin ( RadPitch );
		const double CosYaw = cos ( RadYaw );
		const double SinYaw = sin ( RadYaw );

		return Vector3 (
			CosPitch * CosYaw , // X
			CosPitch * SinYaw , // Y
			SinPitch           // Z
		);
	}

	FRotator Normalize ( )
	{
		while ( this->Yaw > 180.0 )
			this->Yaw -= 360.0;
		while ( this->Yaw < -180.0 )
			this->Yaw += 360.0;

		while ( this->Pitch > 180.0 )
			this->Pitch -= 360.0;
		while ( this->Pitch < -180.0 )
			this->Pitch += 360.0;

		this->Roll = 0.0;
		return *this;
	}

	double Pitch , Yaw , Roll;
};

struct FVector
{
	FVector ( ) : X ( ) , Y ( ) , Z ( ) { }
	FVector ( double X , double Y , double Z ) : X ( X ) , Y ( Y ) , Z ( Z ) { }

	FVector operator + ( const FVector& other ) const { return { this->X + other.X, this->Y + other.Y, this->Z + other.Z }; }
	FVector operator - ( const FVector& other ) const { return { this->X - other.X, this->Y - other.Y, this->Z - other.Z }; }
	FVector operator * ( double offset ) const { return { this->X * offset, this->Y * offset, this->Z * offset }; }
	FVector operator / ( double offset ) const { return { this->X / offset, this->Y / offset, this->Z / offset }; }
	FVector operator * ( const FVector& other ) const { return { this->X * other.X, this->Y * other.Y, this->Z * other.Z }; }
	FVector operator / ( const FVector& other ) const { return { this->X / other.X, this->Y / other.Y, this->Z / other.Z }; }

	FVector& operator *= ( const double other ) { this->X *= other; this->Y *= other; this->Z *= other; return *this; }
	FVector& operator /= ( const double other ) { this->X /= other; this->Y /= other; this->Z /= other; return *this; }

	FVector& operator = ( const FVector& other ) { this->X = other.X; this->Y = other.Y; this->Z = other.Z; return *this; }
	FVector& operator += ( const FVector& other ) { this->X += other.X; this->Y += other.Y; this->Z += other.Z; return *this; }
	FVector& operator -= ( const FVector& other ) { this->X -= other.X; this->Y -= other.Y; this->Z -= other.Z; return *this; }
	FVector& operator *= ( const FVector& other ) { this->X *= other.X; this->Y *= other.Y; this->Z *= other.Z; return *this; }
	FVector& operator /= ( const FVector& other ) { this->X /= other.X; this->Y /= other.Y; this->Z /= other.Z; return *this; }

	operator bool ( ) { return bool ( this->X || this->Y || this->Z ); }
	friend bool operator == ( const FVector& a , const FVector& b ) { return a.X == b.X && a.Y == b.Y && a.Z == b.Z; }
	friend bool operator != ( const FVector& a , const FVector& b ) { return !( a == b ); }

	double Dot ( const FVector& V ) { return X * V.X + Y * V.Y + Z * V.Z; }
	double SizeSquared ( ) { return X * X + Y * Y + Z * Z; }

	inline double Distance ( FVector v )
	{
		return double ( sqrtf ( powf ( v.X - X , 2.0 ) + powf ( v.Y - Y , 2.0 ) + powf ( v.Z - Z , 2.0 ) ) );
	}

	FVector operator*( float scalar ) const {
		return FVector ( X * scalar , Y * scalar , Z * scalar );
	}


	double Dot1 ( const FVector& Other ) const
	{
		return X * Other.X + Y * Other.Y + Z * Other.Z;
	}

	// Alternative name (if you prefer "Scalar" instead of "Dot")
	double Scalar ( const FVector& Other ) const
	{
		return Dot1 ( Other );

	}

	void add_scale ( const FVector& v , float scale ) {
		X += v.X * scale; Y += v.Y * scale; Z += v.Z * scale;
	}

	void Normalize ( )
	{
		while ( this->X > 180.0f )
		{
			this->X -= 360.0f;
		}
		while ( this->X < -180.0f )
		{
			this->X += 360.0f;
		}
		while ( this->Y > 180.0f )
		{
			this->Y -= 360.0f;
		}
		while ( this->Y < -180.0f )
		{
			this->Y += 360.0f;
		}

		this->Z = 0;

	}

	void Clamp ( float Min , float Max )
	{
		if ( this->X < Min )
		{
			this->X = Min;
		}
		else if ( this->X > Max )
		{
			this->X = Max;
		}

		if ( this->Y < Min )
		{
			this->Y = Min;
		}
		else if ( this->Y > Max )
		{
			this->Y = Max;
		}

		this->Z = 0;
	}

	inline double length ( ) {
		return sqrt ( X * X + Y * Y + Z * Z );
	}

	double X , Y , Z;
};

struct f_name {
	int32_t comparison_index;
	int32_t number;
};

struct f_curve_table_row_handle {
	uintptr_t curve_table;     // 0x00: u_curve_table
	f_name row_name;            // 0x08: f_name
	uint8_t pad_0C [ 0x4 ];      // 0x0C: padding
};

struct f_data_registry_type {
	f_name name;               // 0x00: FName
};

struct f_scalable_float {
	float value;                      // 0x00
	uint8_t pad_04 [ 0x4 ];              // 0x04
	f_curve_table_row_handle curve;      // 0x08 
	f_data_registry_type registry_type;  // 0x18 
	uint8_t pad_1C [ 0xC ];             // 0x1C
};

struct fplane : Vector3 {
	double w;
};

struct ftransform {
	fplane rot;
	Vector3 translation;
	std::uint8_t pad [ 8 ];
	Vector3 scale;
	std::uint8_t pad2 [ 8 ];

	D3DMATRIX to_matrix_with_scale ( ) const {
		Vector3 scale_3d (
			( scale.x == 0.0 ) ? 1.0 : scale.x ,
			( scale.y == 0.0 ) ? 1.0 : scale.y ,
			( scale.z == 0.0 ) ? 1.0 : scale.z
		);

		double x2 = rot.x + rot.x;
		double y2 = rot.y + rot.y;
		double z2 = rot.z + rot.z;
		double xx2 = rot.x * x2;
		double yy2 = rot.y * y2;
		double zz2 = rot.z * z2;
		double yz2 = rot.y * z2;
		double wx2 = rot.w * x2;
		double xy2 = rot.x * y2;
		double wz2 = rot.w * z2;
		double xz2 = rot.x * z2;
		double wy2 = rot.w * y2;

		D3DMATRIX m;

		m._41 = translation.x;
		m._42 = translation.y;
		m._43 = translation.z;

		m._11 = ( 1.0 - ( yy2 + zz2 ) ) * scale_3d.x;
		m._22 = ( 1.0 - ( xx2 + zz2 ) ) * scale_3d.y;
		m._33 = ( 1.0 - ( xx2 + yy2 ) ) * scale_3d.z;

		m._32 = ( yz2 - wx2 ) * scale_3d.z;
		m._23 = ( yz2 + wx2 ) * scale_3d.y;

		m._21 = ( xy2 - wz2 ) * scale_3d.y;
		m._12 = ( xy2 + wz2 ) * scale_3d.x;

		m._31 = ( xz2 + wy2 ) * scale_3d.z;
		m._13 = ( xz2 - wy2 ) * scale_3d.x;

		m._14 = 0.0f;
		m._24 = 0.0f;
		m._34 = 0.0f;
		m._44 = 1.0f;

		return m;
	}
};

struct FVector2D
{
	FVector2D ( ) : X ( ) , Y ( ) { }
	FVector2D ( double X , double Y ) : X ( X ) , Y ( Y ) { }

	FVector2D operator + ( const FVector2D& other ) const { return { this->X + other.X, this->Y + other.Y }; }
	FVector2D operator - ( const FVector2D& other ) const { return { this->X - other.X, this->Y - other.Y }; }
	FVector2D operator * ( double offset ) const { return { this->X * offset, this->Y * offset }; }
	FVector2D operator / ( double offset ) const { return { this->X / offset, this->Y / offset }; }

	FVector2D& operator *= ( const double other ) { this->X *= other; this->Y *= other; return *this; }
	FVector2D& operator /= ( const double other ) { this->X /= other; this->Y /= other; return *this; }

	FVector2D& operator = ( const FVector2D& other ) { this->X = other.X; this->Y = other.Y; return *this; }
	FVector2D& operator += ( const FVector2D& other ) { this->X += other.X; this->Y += other.Y; return *this; }
	FVector2D& operator -= ( const FVector2D& other ) { this->X -= other.X; this->Y -= other.Y; return *this; }
	FVector2D& operator *= ( const FVector2D& other ) { this->X *= other.X; this->Y *= other.Y; return *this; }
	FVector2D& operator /= ( const FVector2D& other ) { this->X /= other.X; this->Y /= other.Y; return *this; }

	operator bool ( ) { return bool ( this->X || this->Y ); }
	friend bool operator == ( const FVector2D& A , const FVector2D& B ) { return A.X == B.X && A.Y == A.Y; }
	friend bool operator != ( const FVector2D& A , const FVector2D& B ) { return !( A == B ); }

	double X , Y;

	bool is_valid ( ) const { return this->X && this->Y; }

};


struct FPlane : public FVector
{
	FPlane ( ) : W ( ) { }
	FPlane ( double W ) : W ( W ) { }

	double W;
};

struct alignas( 16 ) matrix_elements {
	double m11 , m12 , m13 , m14;
	double m21 , m22 , m23 , m24;
	double m31 , m32 , m33 , m34;
	double m41 , m42 , m43 , m44;

	matrix_elements ( ) : m11 ( 0 ) , m12 ( 0 ) , m13 ( 0 ) , m14 ( 0 ) ,
		m21 ( 0 ) , m22 ( 0 ) , m23 ( 0 ) , m24 ( 0 ) ,
		m31 ( 0 ) , m32 ( 0 ) , m33 ( 0 ) , m34 ( 0 ) ,
		m41 ( 0 ) , m42 ( 0 ) , m43 ( 0 ) , m44 ( 0 ) {
	}
};

struct alignas( 16 ) dbl_matrix {
	union {
		matrix_elements elements;
		double m [ 4 ][ 4 ];
	};

	dbl_matrix ( ) : elements ( ) { }

	double& operator()( size_t row , size_t col ) { return m [ row ][ col ]; }
	const double& operator()( size_t row , size_t col ) const { return m [ row ][ col ]; }
};

struct alignas( 16 ) FMatrix : public dbl_matrix {
	FPlane x_plane;
	FPlane y_plane;
	FPlane z_plane;
	FPlane w_plane;

	FMatrix ( ) : dbl_matrix ( ) , x_plane ( ) , y_plane ( ) , z_plane ( ) , w_plane ( ) { }
};




D3DMATRIX Matrix ( FRotator rot , Vector3 origin = Vector3 ( 0 , 0 , 0 ) )
{
	float radPitch = ( rot.Pitch * float ( M_PI ) / 180.f );
	float radYaw = ( rot.Yaw * float ( M_PI ) / 180.f );
	float radRoll = ( rot.Roll * float ( M_PI ) / 180.f );

	float SP = sinf ( radPitch );
	float CP = cosf ( radPitch );
	float SY = sinf ( radYaw );
	float CY = cosf ( radYaw );
	float SR = sinf ( radRoll );
	float CR = cosf ( radRoll );

	D3DMATRIX matrix;
	matrix.m [ 0 ][ 0 ] = CP * CY;
	matrix.m [ 0 ][ 1 ] = CP * SY;
	matrix.m [ 0 ][ 2 ] = SP;
	matrix.m [ 0 ][ 3 ] = 0.f;

	matrix.m [ 1 ][ 0 ] = SR * SP * CY - CR * SY;
	matrix.m [ 1 ][ 1 ] = SR * SP * SY + CR * CY;
	matrix.m [ 1 ][ 2 ] = -SR * CP;
	matrix.m [ 1 ][ 3 ] = 0.f;

	matrix.m [ 2 ][ 0 ] = -( CR * SP * CY + SR * SY );
	matrix.m [ 2 ][ 1 ] = CY * SR - CR * SP * SY;
	matrix.m [ 2 ][ 2 ] = CR * CP;
	matrix.m [ 2 ][ 3 ] = 0.f;

	matrix.m [ 3 ][ 0 ] = origin.x;
	matrix.m [ 3 ][ 1 ] = origin.y;
	matrix.m [ 3 ][ 2 ] = origin.z;
	matrix.m [ 3 ][ 3 ] = 1.f;

	return matrix;
}

struct FQuat { double x , y , z , w; };
struct FTransform
{
	FQuat rotation;
	Vector3 translation;
	uint8_t pad1c [ 0x8 ];
	Vector3 scale3d;
	uint8_t pad2c [ 0x8 ];

	D3DMATRIX to_matrix_with_scale ( )
	{
		D3DMATRIX m {};

		const Vector3 Scale
		(
			( scale3d.x == 0.0 ) ? 1.0 : scale3d.x ,
			( scale3d.y == 0.0 ) ? 1.0 : scale3d.y ,
			( scale3d.z == 0.0 ) ? 1.0 : scale3d.z
		);

		const double x2 = rotation.x + rotation.x;
		const double y2 = rotation.y + rotation.y;
		const double z2 = rotation.z + rotation.z;
		const double xx2 = rotation.x * x2;
		const double yy2 = rotation.y * y2;
		const double zz2 = rotation.z * z2;
		const double yz2 = rotation.y * z2;
		const double wx2 = rotation.w * x2;
		const double xy2 = rotation.x * y2;
		const double wz2 = rotation.w * z2;
		const double xz2 = rotation.x * z2;
		const double wy2 = rotation.w * y2;

		m._41 = translation.x;
		m._42 = translation.y;
		m._43 = translation.z;
		m._11 = ( 1.0f - ( yy2 + zz2 ) ) * Scale.x;
		m._22 = ( 1.0f - ( xx2 + zz2 ) ) * Scale.y;
		m._33 = ( 1.0f - ( xx2 + yy2 ) ) * Scale.z;
		m._32 = ( yz2 - wx2 ) * Scale.z;
		m._23 = ( yz2 + wx2 ) * Scale.y;
		m._21 = ( xy2 - wz2 ) * Scale.y;
		m._12 = ( xy2 + wz2 ) * Scale.x;
		m._31 = ( xz2 + wy2 ) * Scale.z;
		m._13 = ( xz2 - wy2 ) * Scale.x;
		m._14 = 0.0f;
		m._24 = 0.0f;
		m._34 = 0.0f;
		m._44 = 1.0f;

		return m;
	}
};

D3DMATRIX matrix_multiplication ( D3DMATRIX pm1 , D3DMATRIX pm2 )
{
	D3DMATRIX pout {};
	pout._11 = pm1._11 * pm2._11 + pm1._12 * pm2._21 + pm1._13 * pm2._31 + pm1._14 * pm2._41;
	pout._12 = pm1._11 * pm2._12 + pm1._12 * pm2._22 + pm1._13 * pm2._32 + pm1._14 * pm2._42;
	pout._13 = pm1._11 * pm2._13 + pm1._12 * pm2._23 + pm1._13 * pm2._33 + pm1._14 * pm2._43;
	pout._14 = pm1._11 * pm2._14 + pm1._12 * pm2._24 + pm1._13 * pm2._34 + pm1._14 * pm2._44;
	pout._21 = pm1._21 * pm2._11 + pm1._22 * pm2._21 + pm1._23 * pm2._31 + pm1._24 * pm2._41;
	pout._22 = pm1._21 * pm2._12 + pm1._22 * pm2._22 + pm1._23 * pm2._32 + pm1._24 * pm2._42;
	pout._23 = pm1._21 * pm2._13 + pm1._22 * pm2._23 + pm1._23 * pm2._33 + pm1._24 * pm2._43;
	pout._24 = pm1._21 * pm2._14 + pm1._22 * pm2._24 + pm1._23 * pm2._34 + pm1._24 * pm2._44;
	pout._31 = pm1._31 * pm2._11 + pm1._32 * pm2._21 + pm1._33 * pm2._31 + pm1._34 * pm2._41;
	pout._32 = pm1._31 * pm2._12 + pm1._32 * pm2._22 + pm1._33 * pm2._32 + pm1._34 * pm2._42;
	pout._33 = pm1._31 * pm2._13 + pm1._32 * pm2._23 + pm1._33 * pm2._33 + pm1._34 * pm2._43;
	pout._34 = pm1._31 * pm2._14 + pm1._32 * pm2._24 + pm1._33 * pm2._34 + pm1._34 * pm2._44;
	pout._41 = pm1._41 * pm2._11 + pm1._42 * pm2._21 + pm1._43 * pm2._31 + pm1._44 * pm2._41;
	pout._42 = pm1._41 * pm2._12 + pm1._42 * pm2._22 + pm1._43 * pm2._32 + pm1._44 * pm2._42;
	pout._43 = pm1._41 * pm2._13 + pm1._42 * pm2._23 + pm1._43 * pm2._33 + pm1._44 * pm2._43;
	pout._44 = pm1._41 * pm2._14 + pm1._42 * pm2._24 + pm1._43 * pm2._34 + pm1._44 * pm2._44;
	return pout;
}

D3DMATRIX to_matrix ( Vector3 rot , Vector3 origin = Vector3 ( 0 , 0 , 0 ) )
{
	float radpitch = ( rot.x * M_PI / 180 );
	float radyaw = ( rot.y * M_PI / 180 );
	float radroll = ( rot.z * M_PI / 180 );
	float sp = sinf ( radpitch );
	float cp = cosf ( radpitch );
	float sy = sinf ( radyaw );
	float cy = cosf ( radyaw );
	float sr = sinf ( radroll );
	float cr = cosf ( radroll );
	D3DMATRIX matrix {};
	matrix.m [ 0 ][ 0 ] = cp * cy;
	matrix.m [ 0 ][ 1 ] = cp * sy;
	matrix.m [ 0 ][ 2 ] = sp;
	matrix.m [ 0 ][ 3 ] = 0.f;
	matrix.m [ 1 ][ 0 ] = sr * sp * cy - cr * sy;
	matrix.m [ 1 ][ 1 ] = sr * sp * sy + cr * cy;
	matrix.m [ 1 ][ 2 ] = -sr * cp;
	matrix.m [ 1 ][ 3 ] = 0.f;
	matrix.m [ 2 ][ 0 ] = -( cr * sp * cy + sr * sy );
	matrix.m [ 2 ][ 1 ] = cy * sr - cr * sp * sy;
	matrix.m [ 2 ][ 2 ] = cr * cp;
	matrix.m [ 2 ][ 3 ] = 0.f;
	matrix.m [ 3 ][ 0 ] = origin.x;
	matrix.m [ 3 ][ 1 ] = origin.y;
	matrix.m [ 3 ][ 2 ] = origin.z;
	matrix.m [ 3 ][ 3 ] = 1.f;
	return matrix;
}

struct Camera
{
	Vector3 location;
	Vector3 rotation;
	float fov;
};

struct FNRot
{
	double a;
	char pad_0008 [ 24 ];
	double b;
	char pad_0028 [ 424 ];
	double c;
};



struct Pointers {

	uintptr_t uworld;
	uintptr_t bIsDying;
	uintptr_t pawn_private;
	uintptr_t targeted_fort_pawn;
	uintptr_t current_weapon;
	uintptr_t weapon_rarity;
	uintptr_t weapon_data;

	float server_time;
	uintptr_t game_instance;
	uintptr_t local_players;
	uintptr_t player_controller;
	uintptr_t local_pawn;
	uintptr_t root_component;
	uintptr_t player_state;
	Vector3 relative_location;
	int my_team_id;
	uintptr_t game_state;
	uintptr_t player_array;
	int player_count;
	float closest_distance;
	uintptr_t closest_mesh;
	FMatrix matrix;

};

std::unique_ptr<Pointers> g_pointers = std::make_unique<Pointers> ( );



inline Vector3 get_bone_position_from_mesh ( uintptr_t mesh , int id ) {
	if ( !mesh ) return Vector3 ( 0 , 0 , 0 );
	uintptr_t bone_array = bypass::read<uintptr_t> ( mesh + Offsets::Bone_Array );
	if ( !bone_array ) bone_array = bypass::read<uintptr_t> ( mesh + Offsets::Bone_Array + 0x10 );
	if ( !bone_array ) return Vector3 ( 0 , 0 , 0 );
	auto BoneTransform = bypass::read<FTransform> ( bone_array + ( id * 0x60 ) );
	FTransform ComponentToWorld = bypass::read<FTransform> ( mesh + Offsets::Component_To_World );
	D3DMATRIX Matrix = matrix_multiplication ( BoneTransform.to_matrix_with_scale ( ) , ComponentToWorld.to_matrix_with_scale ( ) );
	return Vector3 ( Matrix._41 , Matrix._42 , Matrix._43 );
}

struct player_bounds {
	double min_x , max_x , min_y , max_y;
	float width ( ) const { return ( float ) ( max_x - min_x ); }
	float height ( ) const { return ( float ) ( max_y - min_y ); }
};

namespace Custom {
	Vector2 K2_Project ( Vector3 worldlocation );
}

inline player_bounds get_player_bounds ( uintptr_t mesh ) {
	try {
		const std::array<int , 11> bone_indices = { 110 , 71 , 78 , 72 , 79 , 83 , 76 , 9 , 38 , 33 , 62 };
		std::array<Vector2 , 11> bone_screen {};

		for ( size_t i = 0; i < bone_indices.size ( ); ++i ) {
			const Vector3 bone_world = get_bone_position_from_mesh ( mesh , bone_indices [ i ] );
			bone_screen [ i ] = Custom::K2_Project ( bone_world );
		}

		auto [min_x_it , max_x_it] = std::minmax_element (
			bone_screen.begin ( ) , bone_screen.end ( ) ,
			[ ] ( const Vector2& a , const Vector2& b ) { return a.x < b.x; }
		);

		auto [min_y_it , max_y_it] = std::minmax_element (
			bone_screen.begin ( ) , bone_screen.end ( ) ,
			[ ] ( const Vector2& a , const Vector2& b ) { return a.y < b.y; }
		);

		const double box_height = max_y_it->y - min_y_it->y;
		const double box_width = max_x_it->x - min_x_it->x;

		const double width_offset = box_width * 0.175;
		const double height_offset_top = box_height * 0.125;
		const double height_offset_bottom = box_height * 0.05;

		return { min_x_it->x - width_offset , max_x_it->x + width_offset , min_y_it->y - height_offset_top , max_y_it->y + height_offset_bottom };
	}
	catch ( ... ) {
		return {};
	}
}

namespace ViewPoint {

	inline uintptr_t view_state = 0;
	inline Vector3 Location {};
	inline FRotator Rotation {};
	inline float FieldOfView = 0.0f;

	inline void setup_camera ( )
	{
		uintptr_t view_matrix = bypass::read<uintptr_t> ( g_pointers->local_players + 0xd0 );

		view_state = bypass::read<uintptr_t> ( view_matrix + 0x8 );
	}

	inline void update_camera ( )
	{
		g_pointers->matrix = bypass::read<FMatrix> ( ViewPoint::view_state + 0x940 );
		auto projection = g_pointers->matrix;

		Rotation.Pitch = asin ( projection.z_plane.W ) * 180.0f / M_PI;
		Rotation.Yaw = atan2 ( projection.y_plane.W , projection.x_plane.W ) * 180.0f / M_PI;
		Rotation.Roll = 0.0f;

		Location.x = projection.m [ 3 ][ 0 ];
		Location.y = projection.m [ 3 ][ 1 ];
		Location.z = projection.m [ 3 ][ 2 ];

		auto fov_radians = 2.0f * atanf ( 1.0f / static_cast< float >( bypass::read<double> ( view_state + 0x740 ) ) );
		FieldOfView = fov_radians * 180.0f / M_PI;
	}


}



//bool is_visible ( uintptr_t Mesh ) {
//	auto Seconds = bypass::read<double> ( g_pointers->uworld + Offsets::CameraRotation + 0x10 );
//	auto LastRenderTime = bypass::read<float> ( Mesh + 0x32C );
//	return Seconds - LastRenderTime <= 0.06f;
//}


std::string Name ( uintptr_t pawn ) {
	uintptr_t weapon = bypass::read<uintptr_t> ( pawn + Offsets::CurrentWeapon );
	uintptr_t data = bypass::read<uintptr_t> ( weapon + Offsets::WeaponData );
	auto ftext_ptr = bypass::read<uint64_t> ( ( data + 0x40 ) );

	if ( !bypass::target::is_valid ( ftext_ptr ) ) {
		return "None";
	}
	uint64_t ftext_data = bypass::read<uint64_t> ( ftext_ptr + 0x20 );
	if ( !bypass::target::is_valid ( ftext_data ) ) {
		return "None";
	}

	int ftext_length = bypass::read<int> ( ftext_ptr + 0x28 );
	if ( ftext_length <= 0 || ftext_length >= 50 ) {
		return "None";
	}

	std::unique_ptr<wchar_t [ ]> ftext_buf ( new wchar_t [ ftext_length + 1 ] );
	bypass::target::read_memory ( ( ftext_data ) , ftext_buf.get ( ) , ftext_length * sizeof ( wchar_t ) );

	if ( ftext_buf [ 0 ] != L'\0' ) {
		std::wstring wstr_buf ( ftext_buf.get ( ) , ftext_length );
		std::string str_buf ( wstr_buf.begin ( ) , wstr_buf.end ( ) );

		return str_buf;
	}
	else {
		return "None";
	}
}

namespace Custom {
	inline D3DMATRIX rotation_matrix;
	inline float inv_fov;

	inline Vector2 K2_Project ( Vector3 worldlocation ) {

		D3DMATRIX tempMatrix = Matrix ( ViewPoint::Rotation );

		Vector3 vAxisX = Vector3 ( tempMatrix.m [ 0 ][ 0 ] , tempMatrix.m [ 0 ][ 1 ] , tempMatrix.m [ 0 ][ 2 ] );
		Vector3 vAxisY = Vector3 ( tempMatrix.m [ 1 ][ 0 ] , tempMatrix.m [ 1 ][ 1 ] , tempMatrix.m [ 1 ][ 2 ] );
		Vector3 vAxisZ = Vector3 ( tempMatrix.m [ 2 ][ 0 ] , tempMatrix.m [ 2 ][ 1 ] , tempMatrix.m [ 2 ][ 2 ] );

		Vector3 vDelta = worldlocation - ViewPoint::Location;
		Vector3 vTransformed = Vector3 ( vDelta.Dot ( vAxisY ) , vDelta.Dot ( vAxisZ ) , vDelta.Dot ( vAxisX ) );

		if ( vTransformed.z < 1.f )
			vTransformed.z = 1.f;
		return Vector2 ( ( width_sdk / 2.0f ) + vTransformed.x * ( ( ( width_sdk / 2.0f ) / tanf ( ViewPoint::FieldOfView * ( float ) M_PI / 360.f ) ) ) / vTransformed.z , ( height_sdk / 2.0f ) - vTransformed.y * ( ( ( width_sdk / 2.0f ) / tanf ( ViewPoint::FieldOfView * ( float ) M_PI / 360.f ) ) ) / vTransformed.z );
	}



}

ImColor get_platform_color ( std::string platform )
{
	if ( strstr ( platform.c_str ( ) , "WIN" ) ) // Windows / PC
	{
		return ImColor ( 0 , 200 , 255 , 255 );      // bright cyan
	}
	else if ( strstr ( platform.c_str ( ) , "XBL" ) || strstr ( platform.c_str ( ) , "XSX" ) ) // Xbox
	{
		return ImColor ( 16 , 185 , 129 , 255 );     // vibrant Xbox green
	}
	else if ( strstr ( platform.c_str ( ) , "PSN" ) || strstr ( platform.c_str ( ) , "PS5" ) ) // PlayStation
	{
		return ImColor ( 0 , 120 , 255 , 255 );      // bright PlayStation blue
	}
	else if ( strstr ( platform.c_str ( ) , "SWT" ) ) // Switch
	{
		return ImColor ( 255 , 60 , 60 , 255 );      // Nintendo red
	}
	else if ( strstr ( platform.c_str ( ) , "AND" ) ) // Android
	{
		return ImColor ( 80 , 220 , 100 , 255 );     // Android green
	}
	else if ( strstr ( platform.c_str ( ) , "IOS" ) ) // iOS
	{
		return ImColor ( 180 , 180 , 255 , 255 );    // soft Apple-style blue
	}

	return ImColor ( 200 , 200 , 200 , 255 );        // unknown / default
}


std::string get_platform_name_1 ( std::string platform ) {
	if ( strstr ( platform.c_str ( ) , "WIN" ) ) {
		return "Windows";
	}
	else if ( strstr ( platform.c_str ( ) , ( "XBL" ) ) || strstr ( platform.c_str ( ) , ( "XSX" ) ) ) {
		return "Xbox";
	}
	else if ( strstr ( platform.c_str ( ) , ( "PSN" ) ) || strstr ( platform.c_str ( ) , ( "PS5" ) ) ) {
		return "Playstation";
	}
	else if ( strstr ( platform.c_str ( ) , ( "SWT" ) ) ) {
		return "Nintendo";
	}
	else if ( strstr ( platform.c_str ( ) , ( "AND" ) ) || strstr ( platform.c_str ( ) , ( "IOS" ) ) ) {
		return "Mobile";
	}

	return platform;
}

std::string get_platform_name ( uintptr_t player_state ) {
	const auto& state = player_state;
	if ( !state ) return "";

	int size = bypass::read<int> ( player_state + 0x440 + 0x8 );
	if ( size <= 0 || size > 50 ) return "";

	std::uint64_t src = bypass::read<std::uint64_t> ( player_state + 0x440 );
	if ( !src ) return "";

	if ( size <= 32 ) {
		wchar_t stack_buffer [ 33 ] = { 0 };
		if ( !bypass::target::read_memory ( src , stack_buffer , size * sizeof ( wchar_t ) ) ) {
			return "";
		}

		try {
			std::wstring conversion ( stack_buffer );
			return std::string ( conversion.begin ( ) , conversion.end ( ) );
		}
		catch ( ... ) {
			return "";
		}
	}
}

std::string GetPlatformText ( uintptr_t player_state )
{
	uintptr_t platform_ptr = bypass::read<uintptr_t> ( player_state + Offsets::Platform );
	if ( !bypass::target::is_valid ( platform_ptr ) )
		return std::string {};

	wchar_t platform_char [ 64 ] = { 0 };
	bypass::target::read_memory ( ( platform_ptr ) , reinterpret_cast< uint8_t* >( platform_char ) , sizeof ( platform_char ) );
	std::wstring platform_wide_str ( platform_char );

	static const std::unordered_map<std::wstring , std::pair<std::string , std::string>> platform_map = {
		{L"XBL", {"xbox", "G"}},
		{L"XSX", {"xbox", "G"}},
		{L"PSN", {"ps4", "H"}},
		{L"PS5", {"ps5", "H"}},
		{L"WIN", {"windows", "A"}},
		{L"SWT", {"nintendo", "F"}},
		{L"AND", {"android", "E"}},
		{L"MAC", {"apple", "C"}},
		{L"LNX", {"linux", "B"}},
		{L"IOS", {"ios", "D"}}
	};

	for ( const auto& [key , val] : platform_map ) {
		if ( platform_wide_str.find ( key ) != std::wstring::npos ) {
			return val.first;
		}
	}

	return std::string {};
}



struct fbox_sphere_bounds final {
public:
	Vector3 orgin;
	Vector3 box_extent;
	double sphere_radius;
};

inline bool is_visible ( uintptr_t world , uintptr_t mesh )
{
	if ( !world || !mesh )
		return false;

	const float world_delta_time = bypass::read<float> ( world + 0x88CULL );
	const double world_time_seconds = bypass::read<double> ( world + 0x868ULL );
	const float mesh_last_render_time = bypass::read<float> ( mesh + 0x330ULL );

	const double time_since_render = world_time_seconds - static_cast< double >( mesh_last_render_time );
	const float visible_threshold = ( std::max ) ( 0.06f , world_delta_time + 0.0001f );
	return time_since_render <= static_cast< double >( visible_threshold );
}


struct colors_t {
	ImColor m_main_color;
	ImColor m_outline_color;
	ImColor m_text_color;
};

colors_t get_theme_color ( bool isdowned , bool isteamate , bool isvisible ) {
	colors_t theme_color;

	if ( isdowned ) {
		theme_color.m_main_color = ImColor ( 75 , 87 , 219 , 255 );
		theme_color.m_outline_color = ImColor ( 0 , 0 , 0 , 255 );
		theme_color.m_text_color = ImColor ( 75 , 87 , 219 , 255 );
	}
	else if ( isteamate ) {
		theme_color.m_main_color = ImColor ( 93 , 225 , 255 );
		theme_color.m_outline_color = ImColor ( 0 , 0 , 0 , 255 );
		theme_color.m_text_color = ImColor ( 93 , 225 , 255 );
	}
	else if ( isvisible ) {
		theme_color.m_main_color = ImColor ( 140 , 168 , 255 );
		theme_color.m_outline_color = ImColor ( 0 , 0 , 0 , 255 );
		theme_color.m_text_color = ImColor ( 140 , 168 , 255 );
	}
	else {
		theme_color.m_main_color = ImColor ( 255 , 138 , 93 );
		theme_color.m_outline_color = ImColor ( 0 , 0 , 0 , 255 );
		theme_color.m_text_color = ImColor ( 255 , 138 , 93 );
	}

	return theme_color;
}



enum class e_fort_rarity : std::uint8_t {
	common = 0 ,
	uncommon = 1 ,
	rare = 2 ,
	epic = 3 ,
	legendary = 4 ,
	mythic = 5 ,
	transcendent = 6 ,
	unattainable = 7 ,
	num_rarity_values = 8
};


FMatrix to_rotation_matrix ( FRotator& rotation ) {
	FMatrix matrix = {};

	auto rad_pitch = ( rotation.Pitch * M_PI / 180.f );
	auto rad_yaw = ( rotation.Yaw * M_PI / 180.f );
	auto rad_roll = ( rotation.Roll * M_PI / 180.f );

	auto sin_pitch = sin ( rad_pitch );
	auto cos_pitch = cos ( rad_pitch );

	auto sin_yaw = sin ( rad_yaw );
	auto cos_yaw = cos ( rad_yaw );

	auto sin_roll = sin ( rad_roll );
	auto cos_roll = cos ( rad_roll );

	matrix.x_plane.X = cos_pitch * cos_yaw;
	matrix.x_plane.Y = cos_pitch * sin_yaw;
	matrix.x_plane.Z = sin_pitch;
	matrix.x_plane.W = 0.f;

	matrix.y_plane.X = sin_roll * sin_pitch * cos_yaw - cos_roll * sin_yaw;
	matrix.y_plane.Y = sin_roll * sin_pitch * sin_yaw + cos_roll * cos_yaw;
	matrix.y_plane.Z = -sin_roll * cos_pitch;
	matrix.y_plane.W = 0.f;

	matrix.z_plane.X = -( cos_roll * sin_pitch * cos_yaw + sin_roll * sin_yaw );
	matrix.z_plane.Y = cos_yaw * sin_roll - cos_roll * sin_pitch * sin_yaw;
	matrix.z_plane.Z = cos_roll * cos_pitch;
	matrix.z_plane.W = 0.f;

	matrix.w_plane.W = 1.f;
	return matrix;
}


ImColor Weapon ( int32_t Tier )
{

	if ( Tier == 0 || Tier == 7 )
		return ImColor ( 255 , 255 , 255 ); // NOne common
	else if ( Tier == 1 )
		return ImColor ( 0 , 221 , 26 ); // Uncommon
	else if ( Tier == 2 )
		return ImColor ( 0 , 112 , 221 ); // Rare
	else if ( Tier == 3 )
		return ImColor ( 163 , 53 , 238 ); // Epic
	else if ( Tier == 4 )
		return ImColor ( 255 , 128 , 0 ); // Legendary
	else if ( Tier == 5 )
		return ImColor ( 255 , 255 , 0 ); // Mythic
	else
		return ImColor ( 255 , 255 , 255 ); // none
}


ImColor get_tier_color ( e_fort_rarity tier ) {
	switch ( tier ) {
	case e_fort_rarity::common:
		return ImColor ( 102 , 102 , 102 , 255 );
	case e_fort_rarity::uncommon:
		return ImColor ( 138 , 154 , 91 , 255 );
	case e_fort_rarity::rare:
		return ImColor ( 93 , 137 , 186 , 255 );
	case e_fort_rarity::epic:
		return ImColor ( 141 , 78 , 133 , 255 );
	case e_fort_rarity::legendary:
		return ImColor ( 211 , 175 , 55 , 255 );
	case e_fort_rarity::mythic:
		return ImColor ( 255 , 207 , 64 , 255 );
	default:
		return ImColor ( 102 , 102 , 102 , 255 );
	}
}

std::string get_rank_name ( int32_t rank ) {
	if ( rank == 0 )
		return ( "Bronze I" );
	else if ( rank == 1 )
		return ( "Bronze II" );
	else if ( rank == 2 )
		return ( "Bronze III" );
	else if ( rank == 3 )
		return ( "Silver I" );
	else if ( rank == 4 )
		return ( "Silver II" );
	else if ( rank == 5 )
		return ( "Silver III" );
	else if ( rank == 6 )
		return ( "Gold I" );
	else if ( rank == 7 )
		return ( "Gold II" );
	else if ( rank == 8 )
		return ( "Gold III" );
	else if ( rank == 9 )
		return ( "Platinum I" );
	else if ( rank == 10 )
		return ( "Platinum II" );
	else if ( rank == 11 )
		return ( "Platinum III" );
	else if ( rank == 12 )
		return ( "Diamond I" );
	else if ( rank == 13 )
		return ( "Diamond II" );
	else if ( rank == 14 )
		return ( "Diamond III" );
	else if ( rank == 15 )
		return ( "Elite" );
	else if ( rank == 16 )
		return ( "Champion" );
	else if ( rank == 17 )
		return ( "Unreal" );
	return ( "Unranked" );
}


ImColor get_rank_color ( int32_t rank )
{
	// Bronze
	if ( rank >= 0 && rank <= 2 )
		return ImColor ( 255 , 140 , 60 , 255 );      // vibrant bronze orange

	// Silver
	else if ( rank >= 3 && rank <= 5 )
		return ImColor ( 180 , 210 , 255 , 255 );     // bright icy silver

	// Gold
	else if ( rank >= 6 && rank <= 8 )
		return ImColor ( 255 , 215 , 0 , 255 );       // rich gold

	// Platinum
	else if ( rank >= 9 && rank <= 11 )
		return ImColor ( 0 , 220 , 255 , 255 );       // neon cyan

	// Diamond
	else if ( rank >= 12 && rank <= 14 )
		return ImColor ( 70 , 130 , 255 , 255 );      // bright blue

	// Elite
	else if ( rank == 15 )
		return ImColor ( 255 , 90 , 120 , 255 );      // vibrant pink/red

	// Champion
	else if ( rank == 16 )
		return ImColor ( 255 , 120 , 0 , 255 );       // glowing orange

	// Unreal
	else if ( rank == 17 )
		return ImColor ( 180 , 60 , 255 , 255 );      // neon purple

	return ImColor ( 170 , 180 , 190 , 255 );
}

std::string get_user ( uintptr_t player_state , float ServerWorldTimeSecondsDelta ) {

	bool is_lobby = ServerWorldTimeSecondsDelta ? false : true;

	int v25;
	char v21;
	int v22;
	__int64 v6;

	std::uint64_t src;
	int size;

	const auto& state = player_state;
	if ( state ) {
		if ( is_lobby ) {
			size = bypass::read<int> ( player_state + 0x348 + 0x8 );
			if ( !size || size > 100 )
				return "bot";

			src = bypass::read<std::uint64_t> ( player_state + 0x348 );
			if ( !src )
				return "bot";
		}
		else {
			const auto& __routine = bypass::read<std::uint64_t> ( player_state + Offsets::Username );
			if ( !__routine )
				return "bot";

			size = bypass::read<int> ( __routine + 0x10 );
			if ( !size || size > 100 )
				return "bot";

			src = bypass::read<std::uint64_t> ( __routine + 0x8 );
			if ( !src )
				return "bot";
		}

		if ( size > 0 && size < 100 ) {
			std::unique_ptr<wchar_t [ ]> dst ( new ( std::nothrow ) wchar_t [ size ] );

			bypass::target::read_memory (

				src ,
				dst.get ( ) ,
				size * sizeof ( wchar_t )
			);

			v6 = size;
			v21 = v6 - 1;
			v22 = 0;

			if ( v6 == 0 )
				v21 = 0;

			unsigned short* v23 = ( unsigned short* ) ( dst.get ( ) );

			for ( int i = v21 & 3;; *v23++ += i & 7 ) {
				v25 = v6 - 1;

				if ( v6 == 0 )
					v25 = 0;

				if ( v22 >= v25 )
					break;

				i += 3;
				++v22;
			}

			std::wstring conversion ( dst.get ( ) );
			std::string result ( conversion.begin ( ) , conversion.end ( ) );

			std::transform ( result.begin ( ) , result.end ( ) , result.begin ( ) ,
				[ ] ( unsigned char c ) { return std::tolower ( c ); } );

			return result;
		}
	}

	return "AI/NPC";
}

struct Character {


	Vector3 Bone ( int id ) {
		uintptr_t bone_array = bypass::read<uintptr_t> ( ( uintptr_t ) this + Offsets::Bone_Array );
		if ( bone_array == NULL ) {
			bone_array = bypass::read<uintptr_t> ( ( uintptr_t ) this + Offsets::Bone_Array + 0x10 );
		}
		auto BoneTransform = bypass::read<FTransform> ( bone_array + ( id * 0x60 ) );
		FTransform ComponentToWorld = bypass::read<FTransform> ( ( uintptr_t ) this + Offsets::Component_To_World );
		D3DMATRIX Matrix = matrix_multiplication ( BoneTransform.to_matrix_with_scale ( ) , ComponentToWorld.to_matrix_with_scale ( ) );
		return Vector3 ( Matrix._41 , Matrix._42 , Matrix._43 );
	}


};

