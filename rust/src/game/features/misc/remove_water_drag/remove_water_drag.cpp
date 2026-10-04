#include "remove_water_drag.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <sdk/movement_crypt.hpp>
#include <game/features/feature_util.hpp>

namespace features::misc::remove_water_drag {
    void tick()
    {
        bool remove_water_drag = settings.misc.movement.remove_water_drag;
        if (!remove_water_drag)
            return;

        const uptr base = features::local_base_or_zero();
        if (!base) return;
        uptr water_body = memory::read(base + offsets::BasePlayer::waterBody);
        if (!water_body)
            return;

        memory::write<f32>(water_body + 0x20, 0.0f);
        const uptr bm = memory::read(base + offsets::BasePlayer::baseMovement);
        if (bm)
            mvcrypt::write_swimming(bm, false);
    }
}
