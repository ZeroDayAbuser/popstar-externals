#pragma once

static CachedPlayerData current_locked_cache {};
static bool has_current_locked_cache = false;
static double locked_fov_distance = 0.0;

Vector3 aim_loc = Vector3 ( );
Vector2 aim_loc_2d = Vector2 ( );

bool is_in_screen ( const Vector3 screen , int width , int height )
{
	return ( screen.x > 0.f && screen.y > 0.f && screen.x < static_cast< float > ( width ) && screen.y < static_cast< float > ( height ) );
}

double Vector_Distance2D ( const Vector2& a , const Vector2& b ) {
	double dx = static_cast< double > ( a.x ) - static_cast< double > ( b.x );
	double dy = static_cast< double > ( a.y ) - static_cast< double > ( b.y );
	return std::sqrt ( dx * dx + dy * dy );
}

bool InRect ( double Radius , Vector2 ScreenLocation )
{
	return ScreenLocation.x >= ( width_sdk / 2 - Radius ) &&
		ScreenLocation.x <= ( width_sdk / 2 + Radius ) &&
		ScreenLocation.y >= ( height_sdk / 2 - Radius ) &&
		ScreenLocation.y <= ( height_sdk / 2 + Radius );
}

bool InCircle ( double Radius , Vector2  ScreenLocation )
{
	if ( InRect ( Radius , ScreenLocation ) )
	{
		double dx = ( width_sdk / 2 ) - ScreenLocation.x; dx *= dx;
		double dy = ( height_sdk / 2 ) - ScreenLocation.y; dy *= dy;
		return dx + dy <= Radius * Radius;
	} return false;
}


void UpdateAimLocation ( const CachedPlayerData* cached )
{
	if ( !cached )
		return;

	Vector3 hitbox = Vector3 ( 0 , 0 , 0 );

	switch ( g_aimbot::hitbox_type )
	{
	case 0:
		hitbox = cached->head_3d;
		break;

	case 1:
		hitbox = cached->aim_neck_3d;
		break;

	case 2:
		hitbox = cached->aim_chest_3d;
		break;

	case 3:
		hitbox = cached->aim_feet_3d;
		break;

	case 4:
	{
		Vector2 HeadWorld = cached->head_2d;
		Vector2 NeckWorld = Custom::K2_Project ( cached->aim_neck_3d );
		Vector2 ChestWorld = Custom::K2_Project ( cached->aim_chest_3d );
		Vector2 PelvisWorld = Custom::K2_Project ( cached->aim_pelvis_3d );
		Vector2 FeetWorld = Custom::K2_Project ( cached->aim_feet_3d );

		Vector2 center ( width_sdk / 2.0f , height_sdk / 2.0f );
		double HeadDistance = Vector_Distance2D ( center , HeadWorld );
		double NeckDistance = Vector_Distance2D ( center , NeckWorld );
		double ChestDistance = Vector_Distance2D ( center , ChestWorld );
		double PelvisDistance = Vector_Distance2D ( center , PelvisWorld );
		double FeetDistance = Vector_Distance2D ( center , FeetWorld );

		hitbox = cached->head_3d;
		double minDistance = HeadDistance;

		if ( NeckDistance < minDistance ) { hitbox = cached->aim_neck_3d; minDistance = NeckDistance; }
		if ( ChestDistance < minDistance ) { hitbox = cached->aim_chest_3d; minDistance = ChestDistance; }
		if ( PelvisDistance < minDistance ) { hitbox = cached->aim_pelvis_3d; minDistance = PelvisDistance; }
		if ( FeetDistance < minDistance ) { hitbox = cached->aim_feet_3d; minDistance = FeetDistance; }
		break;
	}

	case 5:
	{
		static uintptr_t last_random_mesh = 0;
		static Vector3 last_random_hitbox = Vector3 ( 0 , 0 , 0 );

		if ( cached->mesh != last_random_mesh )
		{
			std::random_device rd;
			std::mt19937 gen ( rd ( ) );
			std::uniform_int_distribution<> dis ( 0 , 4 );
			int random_number = dis ( gen );

			if ( random_number == 0 ) hitbox = cached->head_3d;
			else if ( random_number == 1 ) hitbox = cached->aim_neck_3d;
			else if ( random_number == 2 ) hitbox = cached->aim_chest_3d;
			else if ( random_number == 3 ) hitbox = cached->aim_feet_3d;
			else hitbox = cached->aim_pelvis_3d;

			last_random_mesh = cached->mesh;
			last_random_hitbox = hitbox;
		}
		else
		{
			hitbox = last_random_hitbox;
		}
		break;
	}

	default:
		hitbox = cached->head_3d;
		break;
	}

	aim_loc = hitbox;
	aim_loc_2d = Custom::K2_Project ( hitbox );
}

FRotator find_look_at_rotation ( Vector3& start , Vector3& target ) {
	Vector3 direction = target - start;
	direction = direction / sqrt ( direction.length ( ) );

	auto yaw = atan2 ( direction.y , direction.x ) * ( 180.0 / 3.141592653589793 );
	auto pitch = atan2 ( direction.z , sqrt ( direction.x * direction.x + direction.y * direction.y ) ) * ( 180.0 / 3.141592653589793 );

	return FRotator ( pitch , yaw , 0.0 );
}

FRotator get_aim_rotation ( Vector3 aim_location ) {
	auto aim_rotation = find_look_at_rotation ( ViewPoint::Location , aim_location );
	aim_rotation -= ViewPoint::Rotation;
	aim_rotation.Normalize ( );

	return aim_rotation;
}


void update_aimbot_target ( )
{
	if ( !g_aimbot::enable )
		return;

	CachedPlayerData* best_new_target = nullptr;
	double best_distance_to_center = ( std::numeric_limits<double>::max )( );
	float best_actual_distance = ( std::numeric_limits<float>::max )( );

	auto render_cache = g_world->get_render_cache ( );
	if ( !render_cache || render_cache->empty ( ) ) {
		has_current_locked_cache = false;
		return;
	}

	for ( auto& cached : *render_cache )
	{
		if ( cached.is_partner )
			continue;

		if ( !cached.in_screen )
			continue;

		if ( g_aimbot::visible_check && !cached.is_visible )
			continue;

		if ( cached.distance > g_aimbot::max_distance )
			continue;

		if ( !InCircle ( g_aimbot::fov , cached.head_2d ) )
			continue;

		double distance_to_center = Vector_Distance2D (
			Vector2 ( width_sdk / 2.0 , height_sdk / 2.0 ) ,
			cached.head_2d
		);

		bool is_better_target = false;

		if ( best_new_target == nullptr )
		{
			is_better_target = true;
		}
		else if ( distance_to_center < best_distance_to_center )
		{
			is_better_target = true;
		}
		else if ( std::abs ( distance_to_center - best_distance_to_center ) < 0.5 )
		{
			if ( cached.distance < best_actual_distance )
				is_better_target = true;
		}

		if ( is_better_target )
		{
			best_new_target = &cached;
			best_distance_to_center = distance_to_center;
			best_actual_distance = cached.distance;
		}
	}

	if ( best_new_target != nullptr )
	{
		current_locked_cache = *best_new_target;
		has_current_locked_cache = true;
		locked_fov_distance = best_distance_to_center;
		UpdateAimLocation ( &current_locked_cache );
	}
	else
	{
		has_current_locked_cache = false;
		locked_fov_distance = ( std::numeric_limits<double>::max )( );
	}
}

struct _aimbot {

    float AngleDelta ( float a , float b )
    {
        float delta = fmodf ( b - a , 360.f );
        if ( delta > 180.f ) delta -= 360.f;
        if ( delta < -180.f ) delta += 360.f;
        return delta;
    }

    inline float clamp_shit ( float v , float min , float max )
    {
        return ( v < min ) ? min : ( v > max ) ? max : v;
    }

    void move_mouse ( FRotator NewRotation , int smooth )
    {
        static FRotator inertiaRot = { 0.f, 0.f, 0.f };
		if ( !bypass::target::is_valid ( g_pointers->player_controller ) || !bypass::target::is_valid ( g_pointers->local_pawn ) ) {
			inertiaRot = { 0.f, 0.f, 0.f };
			return;
		}

        NewRotation.Normalize ( );

        float deltaYaw = NewRotation.Yaw;
        float deltaPitch = NewRotation.Pitch;

        float distance = sqrtf ( deltaYaw * deltaYaw + deltaPitch * deltaPitch );

        float dynamicSmooth = smooth + powf ( distance , 0.85f ) * 0.6f;

        dynamicSmooth = max ( 2.f , dynamicSmooth );

        float t = 1.f / dynamicSmooth;
        float inertia = 0.65f;

        inertiaRot.Yaw = inertiaRot.Yaw * inertia + deltaYaw * ( 1.f - inertia );
        inertiaRot.Pitch = inertiaRot.Pitch * inertia + deltaPitch * ( 1.f - inertia );

        float maxStep = 2.5f;

        inertiaRot.Yaw = clamp_shit ( inertiaRot.Yaw , -maxStep , maxStep );
        inertiaRot.Pitch = clamp_shit ( inertiaRot.Pitch , -maxStep , maxStep );

        FRotator output_delta {};
        output_delta.Yaw = inertiaRot.Yaw * t;
        output_delta.Pitch = inertiaRot.Pitch * t;
        output_delta.Roll = 0.f;
        output_delta.Normalize ( );

		bypass::write<FRotator> ( g_pointers->player_controller + 0x530 , output_delta );
		bypass::write<FRotator> ( g_pointers->player_controller + 0x898 , output_delta );

    }
};

std::unique_ptr<_aimbot> g_memory = std::make_unique<_aimbot> ( );
