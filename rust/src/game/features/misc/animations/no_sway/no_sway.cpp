#include "no_sway.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <game/features/feature_util.hpp>

namespace features::misc::no_sway {
    void tick()
    {
        bool enabled = settings.aimbot.weapons.no_sway;
        if (!enabled) return;

        const uptr weapon = features::held_weapon_or_zero();
        if (!weapon) return;

        memory::write<f32>(weapon + offsets::BaseProjectile::aimSway,      0.0f);
        memory::write<f32>(weapon + offsets::BaseProjectile::aimSwaySpeed, 0.0f);
    }
}
