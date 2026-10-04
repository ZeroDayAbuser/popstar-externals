#include "speed_hack.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <sdk/movement_crypt.hpp>
#include <game/features/feature_util.hpp>
#include <cmath>
#include <glm/vec3.hpp>

namespace features::misc::speed_hack {
    void tick()
    {
        bool enabled = settings.misc.movement.speed_hack;
        if (!enabled) return;

        const uptr base = features::local_base_or_zero();
        if (!base) return;
        uptr base_movement = memory::read(base + offsets::BasePlayer::baseMovement);
        if (!base_movement) return;

        constexpr float kSpeedMul = 4.0f;
        constexpr float kMaxMag   = 50.0f;

        float x = 0.0f, y = 0.0f, z = 0.0f;
        mvcrypt::read_vec3_0x128(base_movement, x, y, z);
        glm::vec3 v{ x, y, z };

        v *= kSpeedMul;

        const float mag = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        if (mag > kMaxMag)
            v *= (kMaxMag / mag);

        mvcrypt::write_vec3_0x128(base_movement, v.x, v.y, v.z);
        mvcrypt::write_maxVelocity(base_movement, kMaxMag);
    }
}
