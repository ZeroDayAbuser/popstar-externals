#include "anti_aim.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <sdk/decryptions.hpp>
#include <game/features/feature_util.hpp>

namespace features::misc::anti_aim {
    void tick()
    {
        bool enabled = settings.misc.movement.anti_aim;
        if (!enabled) return;

        const uptr base = features::local_base_or_zero();
        if (!base) return;
        const uptr wrapper = memory::read(base + offsets::BasePlayer::playerEyes);
        if (!wrapper) return;
        uptr eyes = decryption::player_eyes(wrapper);
        if (!eyes && wrapper > 0x100000000ULL && (wrapper & 1) == 0)
            eyes = wrapper;
        if (!eyes) return;

        struct Quat { float x, y, z, w; };
        Quat eye = memory::read<Quat>(eyes + offsets::PlayerEyes::bodyRotation);
        if (eye.x == 0.0f && eye.y == 0.0f && eye.z == 0.0f && eye.w == 0.0f)
            return;
        Quat body;
        body.x = -eye.z;
        body.y =  eye.w;
        body.z =  eye.x;
        body.w = -eye.y;
        memory::write<Quat>(eyes + offsets::PlayerEyes::bodyRotation, body);
    }
}
