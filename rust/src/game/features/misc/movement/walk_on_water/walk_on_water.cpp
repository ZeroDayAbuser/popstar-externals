#include "walk_on_water.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <sdk/movement_crypt.hpp>
#include <game/features/feature_util.hpp>

namespace features::misc::walk_on_water {
    void tick()
    {
        static bool gravity_zeroed = false;

        auto restore_state = []() {
            const uptr base = features::local_base_or_zero();
            if (!base) return;
            const uptr bm = memory::read(base + offsets::BasePlayer::baseMovement);
            if (bm) {
                mvcrypt::write_gravityMultiplier(bm, 1.0f);
                mvcrypt::write_flying           (bm, false);
            }
        };

        bool enabled = settings.misc.movement.walk_on_water;

        if (!enabled) {
            if (gravity_zeroed) { restore_state(); gravity_zeroed = false; }
            return;
        }

        const uptr base = features::local_base_or_zero();
        if (!base) {
            if (gravity_zeroed) { restore_state(); gravity_zeroed = false; }
            return;
        }
        const uptr water = memory::read(base + offsets::BasePlayer::waterBody);
        if (!water) {
            if (gravity_zeroed) { restore_state(); gravity_zeroed = false; }
            return;
        }

        const uptr bm = memory::read(base + offsets::BasePlayer::baseMovement);
        if (!bm) return;

        mvcrypt::write_swimming         (bm, false);
        mvcrypt::write_grounded         (bm, true);
        mvcrypt::write_falling          (bm, false);
        mvcrypt::write_wasFalling       (bm, false);
        mvcrypt::write_gravityMultiplier(bm, 0.0f);
        mvcrypt::write_flying           (bm, true);
        gravity_zeroed = true;
    }
}
