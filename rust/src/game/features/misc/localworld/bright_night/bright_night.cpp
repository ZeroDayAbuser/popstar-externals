#include "bright_night.hpp"
#include <game/game.hpp>
#include <game/features/feature_thread.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <settings/settings.hpp>
#include <atomic>
#include <chrono>
#include <thread>

namespace features::misc::bright_night {

    static std::atomic<bool> g_bright_night_started{ false };

    static void bright_night_worker()
    {
        constexpr float kBrightnessMultiplier = 20.0f;
        bool  enabled    = false;
        auto  last_read  = std::chrono::steady_clock::now();

        while (true)
        {
            if (!game::is_in_game() || !game::impl::tod_sky) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            const auto now = std::chrono::steady_clock::now();
            if (now - last_read >= std::chrono::milliseconds(5))
            {
                enabled = settings.visuals.world.bright_night;
                last_read = now;
            }

            if (!enabled) {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }

            const uptr tod_sky = game::impl::tod_sky;
            if (!tod_sky) {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }

            const uptr night_params   = memory::read<uptr>(tod_sky + offsets::TOD_Sky::night);
            const uptr ambient_params = memory::read<uptr>(tod_sky + offsets::TOD_Sky::ambient);

            if (night_params) {
                memory::write<float>(
                    night_params + offsets::TOD_NightParameters::ambient_multiplier,
                    kBrightnessMultiplier);
            }
            if (ambient_params) {
                memory::write<float>(
                    ambient_params + offsets::TOD_AmbientParameters::saturation,
                    0.0f);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    void tick()
    {
        if (!g_bright_night_started.exchange(true))
            features::run_feature_thread(bright_night_worker);
    }
}
