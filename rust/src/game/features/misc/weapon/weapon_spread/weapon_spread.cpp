#include "weapon_spread.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <game/cache/cache.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <game/features/feature_util.hpp>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace features::misc::weapon_spread {

    using cache::g_cache_epoch;

    static uptr find_component_by_name(uptr owning_component, const char* target_name)
    {
        const uptr game_object = memory::read<uptr>(owning_component + 0x30);
        if (!game_object) return 0;

        const uptr component_list = memory::read<uptr>(game_object + 0x20);
        if (!component_list) return 0;

        const i32 component_size = memory::read<i32>(game_object + 0x30);
        if (component_size <= 0 || component_size > 100) return 0;

        for (i32 idx = 0; idx < component_size; ++idx)
        {
            const uptr component = memory::read<uptr>(component_list + 0x10 * idx + 0x8);
            if (!component) continue;

            const uptr klass = memory::read<uptr>(component + 0x28);
            if (!klass) continue;

            const uptr name_ptr = memory::read<uptr>(klass + 0x00);
            if (!name_ptr) continue;

            const uptr name_str = memory::read<uptr>(name_ptr + 0x10);
            if (!name_str) continue;

            const std::string name = memory::read_string(name_str);
            if (name == target_name) return component;
        }
        return 0;
    }

    void tick()
    {
        if (!settings.settings.ui.enable_memory_writes)
            return;

        uptr base_projectile = features::held_weapon_or_zero();
        if (!base_projectile) return;

        uptr item_mod_projectile = 0;
        uptr magazine = memory::read(base_projectile + offsets::BaseProjectile::primaryMagazine);
        if (magazine)
        {
            uptr ammo_def = memory::read(magazine + offsets::Magazine::ammoType);
            if (ammo_def)
            {
                static std::unordered_map<uptr, uptr>* mod_for_ammo{ nullptr };
                if (!mod_for_ammo) mod_for_ammo = new std::unordered_map<uptr, uptr>;

                auto cached_mod = mod_for_ammo->find(ammo_def);
                if (cached_mod != mod_for_ammo->end())
                    item_mod_projectile = cached_mod->second;
                else {
                    item_mod_projectile = find_component_by_name(ammo_def, "ItemModProjectile");
                    (*mod_for_ammo)[ammo_def] = item_mod_projectile;
                }
            }
        }

        struct CurveOrig {
            uptr aimconeCurve;
            uint8_t useCurve;
            uint8_t overrideAimconeWithCurve;
            bool has_useCurve;
            bool has_recoilOverride;
        };
        static std::unordered_map<uptr, f32>* original_spread{ nullptr };
        static std::unordered_map<uptr, CurveOrig>* curve_orig{ nullptr };
        if (!original_spread) original_spread = new std::unordered_map<uptr, f32>;
        if (!curve_orig)      curve_orig      = new std::unordered_map<uptr, CurveOrig>;

        static std::uint32_t seen_epoch{ 0 };
        const std::uint32_t cur_epoch = g_cache_epoch.load(std::memory_order_acquire);
        if (cur_epoch != seen_epoch) {
            original_spread->clear();
            curve_orig->clear();
            seen_epoch = cur_epoch;
        }
        static uptr last_active_base{ 0 };
        static uptr last_active_imp { 0 };
        if ((last_active_base && last_active_base != base_projectile) ||
            (last_active_imp  && last_active_imp  != item_mod_projectile)) {
            original_spread->clear();
            if (last_active_base) curve_orig->erase(last_active_base);
        }
        last_active_base = base_projectile;
        last_active_imp  = item_mod_projectile;

        bool enabled    = settings.aimbot.weapons.override_weapon_spread;
        i32  amount_pct = settings.aimbot.weapons.override_weapon_spread_amount;

        const bool full_kill = (amount_pct == 0);
        const f32 scale = static_cast<f32>(amount_pct) / 100.0f;

        struct SpreadField { uptr base; uptr off; f32 kill_val; };
        std::vector<SpreadField> fields;
        fields.reserve(8);
        fields.push_back({ base_projectile, offsets::BaseProjectile::aimCone,           -10.0f });
        fields.push_back({ base_projectile, offsets::BaseProjectile::hipAimCone,        -10.0f });
        fields.push_back({ base_projectile, offsets::BaseProjectile::sightAimConeScale,   0.0f });
        fields.push_back({ base_projectile, offsets::BaseProjectile::sightAimConeOffset,  0.0f });
        fields.push_back({ base_projectile, offsets::BaseProjectile::hipAimConeScale,     0.0f });
        fields.push_back({ base_projectile, offsets::BaseProjectile::hipAimConeOffset,    0.0f });
        if (item_mod_projectile) {
            fields.push_back({ item_mod_projectile, offsets::ItemModProjectile::projectileSpread, -10.0f });
            fields.push_back({ item_mod_projectile, offsets::ItemModProjectile::spreadScalar,     -10.0f });
        }

        for (const auto& f : fields)
        {
            if (f.off == 0) continue;

            const uptr addr = f.base + f.off;
            auto cached = original_spread->find(addr);

            if (enabled)
            {
                if (cached == original_spread->end())
                {
                    (*original_spread)[addr] = memory::read<f32>(addr);
                    cached = original_spread->find(addr);
                }
                const f32 orig = cached->second;
                const f32 desired = full_kill ? f.kill_val : orig * scale;
                memory::write<f32>(addr, desired);
            }
            else if (cached != original_spread->end())
            {
                memory::write<f32>(addr, cached->second);
                original_spread->erase(cached);
            }
        }

        const uptr recoil_prop = memory::read<uptr>(base_projectile + offsets::BaseProjectile::recoilProp);

        if (enabled && full_kill)
        {
            auto it = curve_orig->find(base_projectile);
            if (it == curve_orig->end())
            {
                CurveOrig o{};
                o.aimconeCurve = memory::read<uptr>(base_projectile + offsets::BaseProjectile::aimconeCurve);
                if (item_mod_projectile) {
                    o.useCurve = memory::read<uint8_t>(item_mod_projectile + offsets::ItemModProjectile::useCurve);
                    o.has_useCurve = true;
                }
                if (recoil_prop) {
                    o.overrideAimconeWithCurve = memory::read<uint8_t>(recoil_prop + offsets::RecoilProperties::overrideAimconeWithCurve);
                    o.has_recoilOverride = true;
                }
                (*curve_orig)[base_projectile] = o;
            }

            memory::write<uptr>(base_projectile + offsets::BaseProjectile::aimconeCurve, 0);
            if (item_mod_projectile)
                memory::write<uint8_t>(item_mod_projectile + offsets::ItemModProjectile::useCurve, 0);
            if (recoil_prop)
                memory::write<uint8_t>(recoil_prop + offsets::RecoilProperties::overrideAimconeWithCurve, 0);
        }
        else
        {
            auto it = curve_orig->find(base_projectile);
            if (it != curve_orig->end())
            {
                memory::write<uptr>(base_projectile + offsets::BaseProjectile::aimconeCurve, it->second.aimconeCurve);
                if (it->second.has_useCurve && item_mod_projectile)
                    memory::write<uint8_t>(item_mod_projectile + offsets::ItemModProjectile::useCurve, it->second.useCurve);
                if (it->second.has_recoilOverride && recoil_prop)
                    memory::write<uint8_t>(recoil_prop + offsets::RecoilProperties::overrideAimconeWithCurve, it->second.overrideAimconeWithCurve);
                curve_orig->erase(it);
            }
        }
    }
}
