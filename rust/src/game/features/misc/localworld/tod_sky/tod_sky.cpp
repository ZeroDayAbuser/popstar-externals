#include "tod_sky.hpp"
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <settings/settings.hpp>
#include <chrono>

namespace features::misc::tod_sky {

    struct ColorState {
        f32 sun[4]{};
        f32 light[4]{};
        f32 ray[4]{};
        f32 sky[4]{};
        f32 cloud[4]{};
        f32 fog[4]{};
        f32 ambient[4]{};
    };

    static ColorState original_day{};
    static ColorState original_night{};
    static bool colors_captured = false;
    static uptr last_tod_sky = 0;

    struct ModulationState {
        bool enabled{};
        Color color{};

        bool operator!=(const ModulationState& other) const {
            return enabled != other.enabled || color != other.color;
        }
    };

    struct AllModulationState {
        ModulationState sun, light, ray, sky, cloud, fog, ambient;

        bool operator!=(const AllModulationState& other) const {
            return sun != other.sun || light != other.light || ray != other.ray ||
                sky != other.sky || cloud != other.cloud || fog != other.fog ||
                ambient != other.ambient;
        }
    };

    static AllModulationState last_day_applied{};
    static AllModulationState last_night_applied{};

    static bool captured_day[7]{};
    static bool captured_night[7]{};

    static bool is_plausible_color(const float* v)
    {
        auto ok = [](float f) {
            return std::isfinite(f) && f >= -0.05f && f <= 1.05f;
        };
        return ok(v[0]) && ok(v[1]) && ok(v[2]) && ok(v[3]);
    }

    static bool capture_color(uptr param_ptr, u32 offset, float* out)
    {
        uptr gradient = memory::read(param_ptr + offset);
        if (!gradient) return false;
        uptr color_base = memory::read(gradient + 0x10);
        if (!color_base) return false;
        if (!memory::read_memory_raw(color_base, out, sizeof(float) * 4)) return false;
        return is_plausible_color(out);
    }

    static void apply_color(uptr param_ptr, u32 offset, const ModulationState& state, float* original)
    {
        uptr gradient = memory::read(param_ptr + offset);
        if (!gradient) return;
        uptr color_base = memory::read(gradient + 0x10);
        if (!color_base) return;

        f32 current_here[4]{};
        if (!memory::read_memory_raw(color_base, current_here, sizeof(f32) * 4))
            return;
        if (!is_plausible_color(current_here))
            return;
        if (!is_plausible_color(original))
            return;

        if (state.enabled) {
            f32 color[4] = {
                static_cast<f32>(state.color.r) / 255.0f,
                static_cast<f32>(state.color.g) / 255.0f,
                static_cast<f32>(state.color.b) / 255.0f,
                1.0f
            };
            memory::write_memory_raw(color_base, color, sizeof(f32) * 4);
        }
        else {
            memory::write_memory_raw(color_base, original, sizeof(f32) * 4);
        }
    }

    void tick()
    {
        {
            static auto s_last_print = std::chrono::steady_clock::time_point{};
            const auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - s_last_print).count() >= 2) {
                s_last_print = now;
                uptr ts = game::impl::tod_sky;
                uptr d = ts ? memory::read(ts + offsets::TOD_Sky::day) : 0;
                uptr n = ts ? memory::read(ts + offsets::TOD_Sky::night) : 0;
                uptr c = ts ? memory::read(ts + offsets::TOD_Sky::cycle) : 0;
                (void)ts; (void)d; (void)n; (void)c;
            }
        }

        if (!game::impl::tod_sky) {
            colors_captured = false;
            return;
        }

        const auto& world = settings.visuals.world;
        const bool time_changer = world.time_changer;
        const bool any_modulate =
            world.modulates[0].enabled ||
            world.modulates[1].enabled ||
            world.modulates[2].enabled ||
            world.modulates[3].enabled ||
            world.modulates[4].enabled ||
            world.modulates[5].enabled ||
            world.modulates[6].enabled;
        if (!time_changer && !any_modulate) return;

        if (game::impl::tod_sky != last_tod_sky) {
            colors_captured = false;
            for (int i = 0; i < 7; ++i) { captured_day[i] = false; captured_night[i] = false; }
            last_tod_sky = game::impl::tod_sky;
        }

        uptr day = memory::read(game::impl::tod_sky + offsets::TOD_Sky::day);
        uptr night = memory::read(game::impl::tod_sky + offsets::TOD_Sky::night);
        if (!day || !night) return;

        if (!colors_captured) {
            captured_day[0] = capture_color(day, 0x10, original_day.sun);
            captured_day[1] = capture_color(day, 0x18, original_day.light);
            captured_day[2] = capture_color(day, 0x20, original_day.ray);
            captured_day[3] = capture_color(day, 0x28, original_day.sky);
            captured_day[4] = capture_color(day, 0x30, original_day.cloud);
            captured_day[5] = capture_color(day, 0x38, original_day.fog);
            captured_day[6] = capture_color(day, 0x40, original_day.ambient);

            captured_night[0] = capture_color(night, 0x10, original_night.sun);
            captured_night[1] = capture_color(night, 0x18, original_night.light);
            captured_night[2] = capture_color(night, 0x20, original_night.ray);
            captured_night[3] = capture_color(night, 0x28, original_night.sky);
            captured_night[4] = capture_color(night, 0x30, original_night.cloud);
            captured_night[5] = capture_color(night, 0x38, original_night.fog);
            captured_night[6] = capture_color(night, 0x40, original_night.ambient);

            bool all = true;
            for (int i = 0; i < 7; ++i)
                if (!captured_day[i] || !captured_night[i]) { all = false; break; }
            colors_captured = all;
            if (!all) return;  // don't attempt writes this frame — chain is untrusted
        }

        if (time_changer)
        {
            const float time = world.time;
            const uptr cycle = memory::read(game::impl::tod_sky + offsets::TOD_Sky::cycle);
            if (cycle)
                memory::write<float>(cycle + 0x10, time);
        }

        AllModulationState mods{};
        mods.sun     = { world.modulates[0].enabled, world.modulates[0].color };
        mods.light   = { world.modulates[1].enabled, world.modulates[1].color };
        mods.ray     = { world.modulates[2].enabled, world.modulates[2].color };
        mods.sky     = { world.modulates[3].enabled, world.modulates[3].color };
        mods.cloud   = { world.modulates[4].enabled, world.modulates[4].color };
        mods.fog     = { world.modulates[5].enabled, world.modulates[5].color };
        mods.ambient = { world.modulates[6].enabled, world.modulates[6].color };

        if (mods != last_day_applied) {
            if (captured_day[0]) apply_color(day, 0x10, mods.sun,     original_day.sun);
            if (captured_day[1]) apply_color(day, 0x18, mods.light,   original_day.light);
            if (captured_day[2]) apply_color(day, 0x20, mods.ray,     original_day.ray);
            if (captured_day[3]) apply_color(day, 0x28, mods.sky,     original_day.sky);
            if (captured_day[4]) apply_color(day, 0x30, mods.cloud,   original_day.cloud);
            if (captured_day[5]) apply_color(day, 0x38, mods.fog,     original_day.fog);
            if (captured_day[6]) apply_color(day, 0x40, mods.ambient, original_day.ambient);
            last_day_applied = mods;
        }

        if (mods != last_night_applied) {
            if (captured_night[0]) apply_color(night, 0x10, mods.sun,     original_night.sun);
            if (captured_night[1]) apply_color(night, 0x18, mods.light,   original_night.light);
            if (captured_night[2]) apply_color(night, 0x20, mods.ray,     original_night.ray);
            if (captured_night[3]) apply_color(night, 0x28, mods.sky,     original_night.sky);
            if (captured_night[4]) apply_color(night, 0x30, mods.cloud,   original_night.cloud);
            if (captured_night[5]) apply_color(night, 0x38, mods.fog,     original_night.fog);
            if (captured_night[6]) apply_color(night, 0x40, mods.ambient, original_night.ambient);
            last_night_applied = mods;
        }
    }
}
