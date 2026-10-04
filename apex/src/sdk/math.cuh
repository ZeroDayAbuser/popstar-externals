#pragma once

namespace math
{
	constexpr float k_deg2rad = 0.01745329251f;
	constexpr float k_rad2deg = 57.295779513082f;

	inline auto screen_center( ) -> data::_vector2
	{
		return { data::width_ * 0.5f , data::height_ * 0.5f };
	}

	inline auto length_2d( float dx , float dy ) -> float
	{
		return sqrtf( dx * dx + dy * dy );
	}

	inline auto lerp_angle( float from , float to , float t ) -> float
	{
		auto d = to - from;
		while ( d > 180.f ) d -= 360.f;
		while ( d < -180.f ) d += 360.f;
		return from + d * t;
	}

	inline auto calc_angle( data::_vector3 from , data::_vector3 to ) -> data::_vector3
	{
		const auto dx = to.x - from.x;
		const auto dy = to.y - from.y;
		const auto dz = to.z - from.z;
		const auto hyp = length_2d( dx , dy );
		if ( hyp < 0.001f )
			return { };

		return {
			-atanf( dz / hyp ) * k_rad2deg ,
			atan2f( dy , dx ) * k_rad2deg ,
			0.f
		};
	}

}
