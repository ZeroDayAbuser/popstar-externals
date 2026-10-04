#pragma once

#include <cstdint>
#include <string>

namespace loot
{
	enum class rarity_t : std::uint8_t
	{
		common = 0 ,
		rare = 1 ,
		epic = 2 ,
		legendary = 3 ,
		heirloom = 4 ,
	};

	enum class category_t : std::uint8_t
	{
		weapon = 0 ,
		ammo = 1 ,
		heal = 2 ,
		gear = 3 ,
		attachment = 4 ,
		grenade = 5 ,
		deathbox = 6 ,
		misc = 7 ,
		unknown = 8 ,
	};

	struct entity_t
	{
		std::uint64_t address = 0;
		float x = 0.f;
		float y = 0.f;
		float z = 0.f;
		std::string name;
		rarity_t rarity = rarity_t::common;
		category_t category = category_t::unknown;
	};
}
