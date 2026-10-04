#include "silent_walk.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <sdk/movement_crypt.hpp>
#include <game/features/feature_util.hpp>

namespace features::misc::silent_walk {
    void tick()
    {
        bool enabled = settings.misc.movement.silent_walk;
        if (!enabled) return;
        const uptr base = features::local_base_or_zero();
        if (!base) return;

        const uptr base_movement = memory::read(base + offsets::BasePlayer::baseMovement);
        if (!base_movement) return;

        mvcrypt::write_grounded(base_movement, true);
        mvcrypt::write_climbing(base_movement, true);
        mvcrypt::write_wasClimbing(base_movement, true);
    }
}
