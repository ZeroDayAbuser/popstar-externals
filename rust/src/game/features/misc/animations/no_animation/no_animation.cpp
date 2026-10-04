#include "no_animation.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <game/features/feature_util.hpp>

namespace features::misc::no_animation {
    void tick()
    {
        bool enabled = settings.aimbot.weapons.no_animation;
        if (!enabled) return;

        // Do NOT null BasePlayer.currentGesture — that pointer wipe has
        // hard-crashed the client on the next animation tick (especially
        // around respawn). Only shorten deploy/animation delays on the held
        // weapon.
        if (const uptr weapon = features::held_weapon_or_zero()) {
            memory::write<f32>(weapon + offsets::AttackEntity::deployDelay,     0.0f);
            memory::write<f32>(weapon + offsets::AttackEntity::animationDelay,  0.0f);
            memory::write<f32>(weapon + offsets::AttackEntity::timeSinceDeploy, 9999.0f);
        }
    }
}
