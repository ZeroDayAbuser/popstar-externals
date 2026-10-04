#include "fly.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <game/cache/cache.hpp>
#include <game/features/feature_thread.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <sdk/decryptions.hpp>
#include <sdk/movement_crypt.hpp>
#include <sdk/unity/unity.hpp>
#include <atomic>
#include <chrono>
#include <mutex>
#include <print>
#include <utils/debug.hpp>
#include <thread>
#include <Windows.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>

namespace features::misc::fly {

    static std::atomic<bool> g_fly_started{ false };
    std::atomic<bool> g_fly_active{ false };

    static uptr resolve_eyes(uptr local)
    {
        const uptr wrapper = memory::read(local + offsets::BasePlayer::playerEyes);
        if (wrapper) {
            if (uptr eyes = decryption::player_eyes(wrapper))
                return eyes;
            if (wrapper > 0x100000000ULL && wrapper < 0x00007FFFFFFFFFFFULL && (wrapper & 1) == 0)
                return wrapper;
        }
        return unity::get_component_by_name(local, "PlayerEyes");
    }

    static void fly_worker()
    {
        bool gravity_zeroed = false;
        int restore_burst = 0;
        int diag = 0;

        auto write_gravity_and_state = [](uptr bm) {
            mvcrypt::write_gravityMultiplier(bm, 1.0f);
            mvcrypt::write_flying    (bm, false);
            mvcrypt::write_wasFlying (bm, false);
            mvcrypt::write_climbing  (bm, false);
            mvcrypt::write_swimming  (bm, false);
            mvcrypt::write_grounded  (bm, false);
            mvcrypt::write_falling   (bm, true);
            mvcrypt::write_wasFalling(bm, false);
        };

        auto restore_state = [&]() -> bool {
            uptr base = 0;
            {
                std::unique_lock<std::recursive_mutex> lk(cache::cache_mut);
                if (game::impl::local_player)
                    base = game::impl::local_player->base_address;
            }
            if (!base) return false;
            const uptr bm = memory::read(base + offsets::BasePlayer::baseMovement);
            if (!bm) return false;

            write_gravity_and_state(bm);
            memory::write<glm::vec3>(bm + offsets::BaseMovement::TargetMovement, {});
            memory::write<glm::vec3>(bm + offsets::BaseMovement::InheritedVelocity, {});
            mvcrypt::write_vec3_0x128(bm, 0.0f, 0.0f, 0.0f);
            return true;
        };

        auto maintain_state = [&]() {
            uptr base = 0;
            {
                std::unique_lock<std::recursive_mutex> lk(cache::cache_mut);
                if (game::impl::local_player)
                    base = game::impl::local_player->base_address;
            }
            if (!base) return;
            const uptr bm = memory::read(base + offsets::BasePlayer::baseMovement);
            if (!bm) return;
            write_gravity_and_state(bm);
        };

        while (true)
        {
            uptr local = 0;
            {
                std::unique_lock<std::recursive_mutex> lk(cache::cache_mut);
                if (game::impl::local_player)
                    local = game::impl::local_player->base_address;
            }
            if (!local)
            {
                if (gravity_zeroed && restore_state()) {
                    gravity_zeroed = false;
                    restore_burst = 50;
                }
                if (restore_burst > 0) { maintain_state(); --restore_burst; }
                g_fly_active.store(false, std::memory_order_relaxed);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            bool enabled = settings.misc.movement.fly;
            const int fly_vk = settings.misc.movement.fly_key.key;
            if (!enabled && fly_vk != 0 && (GetAsyncKeyState(fly_vk) & 0x8000))
                enabled = true;

            if (!enabled)
            {
                if (gravity_zeroed && restore_state()) {
                    gravity_zeroed = false;
                    restore_burst = 50;
                }
                if (restore_burst > 0) { maintain_state(); --restore_burst; }
                g_fly_active.store(false, std::memory_order_relaxed);
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }

            restore_burst = 0;
            g_fly_active.store(true, std::memory_order_relaxed);

            const uptr bm = memory::read(local + offsets::BasePlayer::baseMovement);
            if (!bm) { std::this_thread::yield(); continue; }

            static uptr cached_eyes = 0;
            static uptr cached_for_base = 0;
            if (cached_for_base != local) {
                cached_for_base = local;
                cached_eyes = resolve_eyes(local);
            }
            if (!cached_eyes)
                cached_eyes = resolve_eyes(local);

            mvcrypt::write_flying(bm, true);
            mvcrypt::write_grounded(bm, false);
            mvcrypt::write_falling(bm, false);
            mvcrypt::write_swimming(bm, false);
            mvcrypt::write_climbing(bm, false);
            mvcrypt::write_gravityMultiplier(bm, 0.0f);
            mvcrypt::write_groundAngleNew(bm, 0.0f);
            gravity_zeroed = true;

            glm::vec3 world_forward{ 0.0f, 0.0f, 1.0f };
            glm::vec3 world_right{ 1.0f, 0.0f, 0.0f };
            if (cached_eyes) {
                const glm::vec4 r = memory::read<glm::vec4>(cached_eyes + offsets::PlayerEyes::bodyRotation);
                const float qlen = r.x * r.x + r.y * r.y + r.z * r.z + r.w * r.w;
                if (qlen > 0.01f) {
                    const glm::quat rot(r.w, r.x, r.y, r.z);
                    world_forward = rot * glm::vec3(0.0f, 0.0f, 1.0f);
                    world_right   = rot * glm::vec3(1.0f, 0.0f, 0.0f);
                }
            }

            glm::vec3 target{ 0.0f, 0.0f, 0.0f };
            auto held = [](int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; };
            if (held('W')) target += world_forward;
            if (held('S')) target -= world_forward;
            if (held('A')) target -= world_right;
            if (held('D')) target += world_right;
            if (held(VK_SPACE)) target.y += 1.0f;
            if (held(VK_CONTROL)) target.y -= 1.0f;

            float speed = 5.0f;
            if (held(VK_SHIFT)) speed = 10.0f;

            const glm::vec3 scaled = target * speed;

            // Plaintext BaseMovement intent (0x3C / 0x30) — NOT crypt groundNormal slots.
            memory::write<glm::vec3>(bm + offsets::BaseMovement::TargetMovement, scaled);
            memory::write<glm::vec3>(bm + offsets::BaseMovement::InheritedVelocity, {});

            // Encrypted PlayerWalkMovement.velocity @ 0x128
            mvcrypt::write_vec3_0x128(bm, scaled.x, scaled.y, scaled.z);

            if ((++diag % 400) == 1) {
                DBG("[fly] bm=0x{:x} eyes=0x{:x} tgt=({:.1f},{:.1f},{:.1f}) TM@0x{:x}",
                    bm, cached_eyes, scaled.x, scaled.y, scaled.z,
                    (uptr)offsets::BaseMovement::TargetMovement);
            }

            std::this_thread::yield();
        }
    }

    void tick()
    {
        if (!g_fly_started.exchange(true))
            features::run_feature_thread(fly_worker);
    }
}
