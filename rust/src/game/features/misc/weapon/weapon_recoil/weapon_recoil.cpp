#include "weapon_recoil.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <game/cache/cache.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <game/features/feature_util.hpp>
#include <cmath>
#include <cstdint>
#include <print>
#include <utils/debug.hpp>
#include <unordered_map>

namespace features::misc::weapon_recoil {

    using cache::g_cache_epoch;

    struct RecoilCache {
        f32 yaw_min, yaw_max, pitch_min, pitch_max;
    };

    static std::unordered_map<uptr, RecoilCache>* g_recoil_map      = nullptr;
    static uptr                                    g_last_active_prop     = 0;
    static uptr                                    g_last_active_override = 0;
    static std::uint32_t                           g_seen_epoch           = 0;

    static bool looks_like_our_zero_write(const RecoilCache& c)
    {
        auto near_zero = [](f32 v) { return std::abs(v) < 0.0005f; };
        return near_zero(c.yaw_min) && near_zero(c.yaw_max)
            && near_zero(c.pitch_min) && near_zero(c.pitch_max);
    }

    static bool sane_recoil(const RecoilCache& c)
    {
        auto ok = [](f32 v) { return std::isfinite(v) && v > -10000.0f && v < 10000.0f; };
        return ok(c.yaw_min) && ok(c.yaw_max) && ok(c.pitch_min) && ok(c.pitch_max);
    }

    static void write_recoil(uptr prop, const RecoilCache& c)
    {
        memory::write<f32>(prop + offsets::RecoilProperties::recoilYawMin,   c.yaw_min);
        memory::write<f32>(prop + offsets::RecoilProperties::recoilYawMax,   c.yaw_max);
        memory::write<f32>(prop + offsets::RecoilProperties::recoilPitchMin, c.pitch_min);
        memory::write<f32>(prop + offsets::RecoilProperties::recoilPitchMax, c.pitch_max);
    }

    void tick()
    {
        if (!g_recoil_map)
            g_recoil_map = new std::unordered_map<uptr, RecoilCache>;

        const std::uint32_t cur_epoch = g_cache_epoch.load(std::memory_order_acquire);
        if (cur_epoch != g_seen_epoch) {
            g_recoil_map->clear();
            g_last_active_prop = 0;
            g_last_active_override = 0;
            g_seen_epoch = cur_epoch;
        }

        const bool recoil_enabled       = settings.aimbot.weapons.override_weapon_recoil;
        const i32  wanted_recoil_perc   = settings.aimbot.weapons.override_weapon_recoil_amount;

        auto process_recoil = [&](uptr prop, f32 mult, bool enabled) {
            if (!prop) return;
            auto it = g_recoil_map->find(prop);
            if (enabled) {
                if (it == g_recoil_map->end()) {
                    RecoilCache c{
                        memory::read<f32>(prop + offsets::RecoilProperties::recoilYawMin),
                        memory::read<f32>(prop + offsets::RecoilProperties::recoilYawMax),
                        memory::read<f32>(prop + offsets::RecoilProperties::recoilPitchMin),
                        memory::read<f32>(prop + offsets::RecoilProperties::recoilPitchMax)
                    };
                    if (!sane_recoil(c)) return;
                    if (looks_like_our_zero_write(c)) return;
                    (*g_recoil_map)[prop] = c;
                    it = g_recoil_map->find(prop);
                }
                const auto& original = it->second;
                memory::write<f32>(prop + offsets::RecoilProperties::recoilYawMin,   original.yaw_min   * mult);
                memory::write<f32>(prop + offsets::RecoilProperties::recoilYawMax,   original.yaw_max   * mult);
                memory::write<f32>(prop + offsets::RecoilProperties::recoilPitchMin, original.pitch_min * mult);
                memory::write<f32>(prop + offsets::RecoilProperties::recoilPitchMax, original.pitch_max * mult);
            }
            else {
                if (it != g_recoil_map->end()) {
                    write_recoil(prop, it->second);
                    g_recoil_map->erase(it);
                }
            }
        };

        if (!recoil_enabled) {
            for (auto it = g_recoil_map->begin(); it != g_recoil_map->end(); ) {
                uptr prop = it->first;
                if (prop == g_last_active_prop || prop == g_last_active_override)
                    write_recoil(prop, it->second);
                it = g_recoil_map->erase(it);
            }
            g_last_active_prop     = 0;
            g_last_active_override = 0;
            return;
        }

        if (!settings.settings.ui.enable_memory_writes) {
            static int s_w = 0;
            if ((++s_w % 300) == 1)
                DBG("[recoil] blocked — enable Settings → Enable memory writes");
            return;
        }

        uptr base_projectile = features::held_weapon_or_zero();
        if (!base_projectile) {
            static int s_h = 0;
            if ((++s_h % 300) == 1)
                DBG("[recoil] no held weapon (active_item=0x{:x})",
                    game::impl::local_player ? game::impl::local_player->active_item : 0);
            return;
        }

        uptr recoil_prop = memory::read(base_projectile + offsets::BaseProjectile::recoilProp);
        if (!recoil_prop) {
            static int s_p = 0;
            if ((++s_p % 300) == 1)
                DBG("[recoil] held=0x{:x} but recoilProp@+0x{:x} = 0",
                    base_projectile, (uptr)offsets::BaseProjectile::recoilProp);
            return;
        }

        uptr override_prop = memory::read(recoil_prop + offsets::RecoilProperties::newRecoilOverride);

        if (g_last_active_prop     && g_last_active_prop     != recoil_prop)
            g_recoil_map->erase(g_last_active_prop);
        if (g_last_active_override && g_last_active_override != override_prop)
            g_recoil_map->erase(g_last_active_override);

        g_last_active_prop     = recoil_prop;
        g_last_active_override = override_prop;

        const f32 multiplier = static_cast<f32>(wanted_recoil_perc) / 100.0f;
        process_recoil(recoil_prop, multiplier, true);
        process_recoil(override_prop, multiplier, true);

        if (offsets::BaseProjectile::aimSway) {
            memory::write<f32>(base_projectile + offsets::BaseProjectile::aimSway, 0.0f);
            memory::write<f32>(base_projectile + offsets::BaseProjectile::aimSwaySpeed, 0.0f);
        }

        {
            static int s_ok = 0;
            if ((++s_ok % 600) == 1)
                DBG("[recoil] writing held=0x{:x} prop=0x{:x} override=0x{:x} mult={:.2f}",
                    base_projectile, recoil_prop, override_prop, multiplier);
        }
    }

    void restore_all()
    {
        if (!g_recoil_map) return;
        for (auto& [prop, cache] : *g_recoil_map) {
            if (prop) write_recoil(prop, cache);
        }
        g_recoil_map->clear();
        g_last_active_prop     = 0;
        g_last_active_override = 0;
    }
}
