#include "omni_sprint.hpp"
#include <sdk/rust/entity/entity.hpp>
#include "../fly/fly.hpp"
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <game/cache/cache.hpp>
#include <game/features/feature_thread.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <sdk/movement_crypt.hpp>
#include <atomic>
#include <chrono>
#include <mutex>
#include <Windows.h>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <print>
#include <utils/debug.hpp>

namespace features::misc::omni_sprint {

    static void set_model_flag(uptr state, int flag, bool value)
    {
        if (!state) return;
        const uptr addr = state + offsets::ModelState::flags;
        int flags = memory::read<int>(addr);
        if (value) flags |= flag;
        else       flags &= ~flag;
        memory::write<int>(addr, flags);
    }

    static std::atomic<bool> g_omni_sprint_started{ false };

    static void omni_sprint_worker()
    {
        static int s_diag = 0;
        while (true)
        {
            uptr base = 0;
            {
                std::unique_lock<std::recursive_mutex> lk(cache::cache_mut);
                if (game::impl::local_player)
                    base = game::impl::local_player->base_address;
            }
            if (!base)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            const bool enabled = settings.misc.movement.omni_sprint;
            if (!enabled || features::misc::fly::g_fly_active.load(std::memory_order_relaxed))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }

            {
                i32 pf = memory::read<i32>(base + offsets::BasePlayer::playerFlags);
                if (pf & PlayerFlags::NoSprint)
                    memory::write<i32>(base + offsets::BasePlayer::playerFlags,
                                       pf & ~PlayerFlags::NoSprint);
            }

            const uptr movement = memory::read(base + offsets::BasePlayer::baseMovement);
            if (!movement)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }

            const uptr model_state = memory::read(base + offsets::BasePlayer::modelState);

            bool blocked = false;
            if (model_state) {
                const int packet_flags = memory::read<int>(model_state + offsets::ModelState::flags);
                blocked = (packet_flags & (int)offsets::ModelState::Ducked) != 0
                       || (packet_flags & (int)offsets::ModelState::Crawling) != 0;
            }

            if (!blocked)
            {
                glm::vec3 vel = memory::read<glm::vec3>(
                    movement + offsets::BaseMovement::TargetMovement);

                // If plaintext TargetMovement looks dead, pull encrypted velocity.
                if (glm::length(vel) <= 0.01f) {
                    float x = 0, y = 0, z = 0;
                    mvcrypt::read_vec3_0x128(movement, x, y, z);
                    vel = { x, 0.0f, z }; // horizontal only
                }

                if (glm::length(vel) > 0.01f)
                {
                    constexpr float kSprintSpeed = 5.5f;
                    const glm::vec3 out = glm::normalize(vel) * kSprintSpeed;
                    memory::write<glm::vec3>(
                        movement + offsets::BaseMovement::TargetMovement, out);
                    mvcrypt::write_vec3_0x128(movement, out.x, out.y, out.z);
                    mvcrypt::write_sprintForced(movement, 1.0f);
                    if (model_state)
                        set_model_flag(model_state, (int)offsets::ModelState::Sprinting, true);

                    if ((++s_diag % 250) == 1) {
                        DBG("[omni] applied sprint vel=({:.1f},{:.1f},{:.1f}) ms=0x{:x}",
                            out.x, out.y, out.z, model_state);
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }

    void tick()
    {
        if (!g_omni_sprint_started.exchange(true))
            features::run_feature_thread(omni_sprint_worker);
    }
}
