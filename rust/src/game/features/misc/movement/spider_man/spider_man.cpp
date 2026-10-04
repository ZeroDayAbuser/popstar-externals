#include "spider_man.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <sdk/movement_crypt.hpp>
#include <game/features/feature_util.hpp>
#include <Windows.h>

namespace features::misc::spider_man {
    void tick()
    {
        bool spider_man = settings.misc.movement.spider_man;
        const int sm_vk = settings.misc.movement.spider_man_key.key;
        const bool key_held = sm_vk != 0 && (GetAsyncKeyState(sm_vk) & 0x8000) != 0;
        if (!spider_man && !key_held)
            return;

        const uptr base = features::local_base_or_zero();
        if (!base) return;
        uptr base_movement = memory::read(base + offsets::BasePlayer::baseMovement);
        if (!base_movement)
            return;

        mvcrypt::write_groundAngleNew(base_movement, 0.0f);
    }
}
