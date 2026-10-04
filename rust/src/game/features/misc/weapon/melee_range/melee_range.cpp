#include "melee_range.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <game/features/feature_util.hpp>
#include <unordered_map>

namespace features::misc::melee_range {
    void tick()
    {
        struct MeleeCache {
            f32 max_distance;
            f32 attack_radius;
        };
        static std::unordered_map<uptr, MeleeCache>* melee_map{ nullptr };
        if (!melee_map)
            melee_map = new std::unordered_map<uptr, MeleeCache>;

        bool melee_enabled = settings.aimbot.weapons.override_melee_range;
        i32 wanted_range_perc = settings.aimbot.weapons.override_melee_range_amount;

        uptr base_melee = features::held_weapon_or_zero();
        if (!base_melee && game::impl::local_player && game::impl::local_player->active_item)
            base_melee = memory::read(game::impl::local_player->active_item + offsets::Item::heldEntity);
        if (!base_melee)
            return;
        const uptr active_item = base_melee;

        if (melee_enabled) {
            if (melee_map->find(active_item) == melee_map->end()) {
                (*melee_map)[active_item] = {
                    memory::read<f32>(base_melee + offsets::BaseMelee::maxDistance),
                    memory::read<f32>(base_melee + offsets::BaseMelee::attackRadius)
                };
            }

            const auto& original = (*melee_map)[active_item];
            const f32 multiplier = static_cast<f32>(wanted_range_perc) / 100.0f;

            if (original.max_distance > 0.0f && original.max_distance < 10.0f) {
                memory::write<f32>(base_melee + offsets::BaseMelee::maxDistance, original.max_distance * multiplier);
                memory::write<f32>(base_melee + offsets::BaseMelee::attackRadius, original.attack_radius * multiplier);
            }
        }
        else {
            auto it = melee_map->find(active_item);
            if (it != melee_map->end()) {
                memory::write<f32>(base_melee + offsets::BaseMelee::maxDistance, it->second.max_distance);
                memory::write<f32>(base_melee + offsets::BaseMelee::attackRadius, it->second.attack_radius);
                melee_map->erase(it);
            }
        }
    }
}
