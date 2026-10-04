#pragma once

namespace aimbot
{
	auto tick( ) -> void;
	auto draw( ) -> void;

	auto get_enemy( ) -> classes::c_entity*;
	auto get_bone( classes::c_entity* enemy ) -> data::_vector3;
	auto view_angles( data::_vector3 world , data::_vector3 origin ) -> void;
}
