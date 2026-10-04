#pragma once

namespace heirloom
{
	auto tick( ) -> void;
	auto catalog_count( ) -> int;
	auto catalog_label( int index ) -> const char*;
}
