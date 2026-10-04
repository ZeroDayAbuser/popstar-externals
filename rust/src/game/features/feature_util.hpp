#pragma once


#include <cstdint>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/decryptions.hpp>
#include <sdk/offsets.hpp>
#include <sdk/rust/entity/entity.hpp>

namespace features {

	inline std::uintptr_t local_base_or_zero() noexcept
	{
		auto* lp = game::impl::local_player;
		return (lp && lp->base_address) ? lp->base_address : 0;
	}

	inline std::uintptr_t item_held_entity(std::uintptr_t item) noexcept
	{
		if (!item) return 0;
		std::uintptr_t held = memory::read<std::uintptr_t>(item + offsets::Item::heldEntity);
		if (!held && offsets::Item::held_entity_alt)
			held = memory::read<std::uintptr_t>(item + offsets::Item::held_entity_alt);
		if (!held) return 0;
		if (held < 0x100000000ULL || held > 0x00007FFFFFFFFFFFULL) return 0;
		return held;
	}

	inline bool looks_like_projectile(std::uintptr_t held) noexcept
	{
		if (!held || !offsets::BaseProjectile::recoilProp) return false;
		const std::uintptr_t prop = memory::read<std::uintptr_t>(held + offsets::BaseProjectile::recoilProp);
		return prop > 0x100000000ULL && prop < 0x00007FFFFFFFFFFFULL;
	}

	inline std::uintptr_t held_weapon_or_zero() noexcept
	{
		auto* lp = game::impl::local_player;
		if (!lp || !lp->base_address) return 0;

		auto accept = [](std::uintptr_t held) -> std::uintptr_t {
			return (held && looks_like_projectile(held)) ? held : 0;
		};

		if (std::uintptr_t h = accept(item_held_entity(lp->active_item)))
			return h;

		constexpr std::uint32_t kHeld = 0x400;
		for (std::int32_t i = 0; i < 6; ++i) {
			const std::uintptr_t item = lp->held_items[i];
			const std::uintptr_t held = item_held_entity(item);
			if (!held) continue;
			const std::uint32_t flags = memory::read<std::uint32_t>(held + offsets::BaseEntity::flags);
			//printf("[held] item=0x%d held=0x%d flags=0x%i\n", item, held, flags);
			if ((flags & kHeld) && accept(held))
				return held;
		}

		const std::uint64_t enc = memory::read<std::uint64_t>(lp->base_address + offsets::BasePlayer::clActiveItem);
		if (enc) {
			const std::uint64_t uid = decryption::cl_active_item(enc);
			if (uid) {
				for (std::int32_t i = 0; i < 6; ++i) {
					const std::uintptr_t item = lp->held_items[i];
					if (!item) continue;
					const std::uint64_t item_uid = memory::read<std::uint64_t>(item + offsets::Item::uid);
					if (item_uid == uid) {
						if (std::uintptr_t h = accept(item_held_entity(item)))
							return h;
					}
				}
			}
		}

		for (std::int32_t i = 0; i < 6; ++i) {
			if (std::uintptr_t h = accept(item_held_entity(lp->held_items[i])))
				return h;
		}
		return 0;
	}

}
