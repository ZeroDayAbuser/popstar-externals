#include "fov.hpp"
#include <game/game.hpp>
#include <game/features/feature_thread.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <settings/settings.hpp>
#include <atomic>
#include <chrono>
#include <cmath>
#include <thread>
#include <Windows.h>

namespace features::misc::fov {

    static std::atomic<bool> g_fov_started{ false };

    static void fov_worker()
    {
        bool enabled  = false;
        f32 base_fov = 90.0f, zoom_fov = 35.0f;
        int zoom_vk = 0;
        auto last_read = std::chrono::steady_clock::now();
        auto last_step = std::chrono::steady_clock::now();
        f32 current = -1.0f;

        while (true)
        {
            if (!game::is_in_game())
            {
                current = -1.0f;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            const uptr cam = game::impl::camera_object;
            if (!cam)
            {
                current = -1.0f;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            const auto now = std::chrono::steady_clock::now();
            if (now - last_read >= std::chrono::milliseconds(5))
            {
                enabled  = settings.visuals.local.custom_fov;
                base_fov = settings.visuals.local.field_of_view;
                zoom_fov = settings.visuals.local.zoom_fov;
                zoom_vk  = settings.visuals.local.zoom_key.key;
                last_read = now;
            }

            if (!enabled)
            {
                current = -1.0f;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            const bool zoom_held =
                (zoom_vk > 0 && zoom_vk < 256 && (GetAsyncKeyState(zoom_vk) & 0x8000));
            const bool rmb_held = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
            const f32 target = zoom_held ? zoom_fov : base_fov;
            if (target < 1.0f || target > 180.0f)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }

            const f32 dt = std::chrono::duration<f32>(now - last_step).count();
            last_step = now;
            constexpr f32 kRate     = 750.0f;
            constexpr f32 kEaseDist = 4.0f;
            constexpr f32 kMinScale = 0.15f;

            if (current < 0.0f) current = target;
            const f32 delta     = target - current;
            const f32 abs_delta = std::abs(delta);
            const f32 scale =
                abs_delta >= kEaseDist
                    ? 1.0f
                    : kMinScale + (1.0f - kMinScale) * (abs_delta / kEaseDist);
            const f32 step = kRate * scale * (dt > 0.05f ? 0.05f : dt);

            if (abs_delta <= step) current = target;
            else                   current += (delta > 0.0f ? step : -step);

            const uptr addr = cam + offsets::main_camera::field_of_view;
            static int steady_ticks = 0;
            if (game::impl::camera_object == cam) {
                const f32 game_fov = memory::read<f32>(addr);
                if (std::abs(game_fov - current) > 0.05f) {
                    memory::write<f32>(addr, current);
                    steady_ticks = 0;
                }
                else if (steady_ticks < 500) {
                    steady_ticks++;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(steady_ticks >= 200 ? 20 : 1));
        }
    }

    void tick()
    {
        if (!g_fov_started.exchange(true))
            features::run_feature_thread(fov_worker);
    }
}
