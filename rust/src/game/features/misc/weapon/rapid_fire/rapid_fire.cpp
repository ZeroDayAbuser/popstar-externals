#include "rapid_fire.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <game/features/feature_util.hpp>
#include <unordered_map>

namespace features::misc::rapid_fire {
    void tick()
    {
        const bool rapid = settings.aimbot.weapons.rapid_fire;
        const bool fast  = settings.aimbot.weapons.fast_shoot;
        const bool enabled = rapid || fast;

        static std::unordered_map<uptr, f32>* s_original = nullptr;
        if (!s_original) s_original = new std::unordered_map<uptr, f32>;

        static bool s_was_enabled = false;
        const bool just_turned_off = s_was_enabled && !enabled;
        s_was_enabled = enabled;

        if (!enabled && !just_turned_off) return;

        const uptr weapon = features::held_weapon_or_zero();
        if (!weapon) return;

        const uptr delay_addr = weapon + offsets::AttackEntity::repeatDelay;

        if (enabled) {
            if (s_original->find(weapon) == s_original->end())
                (*s_original)[weapon] = memory::read<f32>(delay_addr);
            if (rapid)
                memory::write<f32>(delay_addr, fast ? 0.01f : 0.09f);
            if (fast)
                memory::write<f32>(weapon + offsets::AttackEntity::nextAttackTime, 0.0f);
        } else {
            for (const auto& kv : *s_original)
                memory::write<f32>(kv.first + offsets::AttackEntity::repeatDelay, kv.second);
            s_original->clear();
        }
    }
}
