#include "no_fall.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <sdk/movement_crypt.hpp>
#include <game/features/feature_util.hpp>
#include <glm/vec3.hpp>

namespace features::misc::no_fall {
    void tick()
    {
        bool enabled = settings.misc.movement.no_fall;
        if (!enabled) return;

        const uptr base = features::local_base_or_zero();
        if (!base) return;
        uptr base_movement = memory::read(base + offsets::BasePlayer::baseMovement);
        if (!base_movement) return;

        mvcrypt::write_falling(base_movement, false);
        mvcrypt::write_wasFalling(base_movement, false);
        mvcrypt::write_grounded(base_movement, true);
        float x = 0.0f, y = 0.0f, z = 0.0f;
        mvcrypt::read_vec3_0x128(base_movement, x, y, z);
        if (y < 0.0f)
            mvcrypt::write_vec3_0x128(base_movement, x, 0.0f, z);
    }
}
