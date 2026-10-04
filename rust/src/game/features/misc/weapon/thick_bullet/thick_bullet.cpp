#include "thick_bullet.hpp"
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <settings/settings.hpp>
#include <print>
#include <utils/debug.hpp>

namespace features::misc::thick_bullet {

    static constexpr f32 kThickness = 1.8f;

    static bool looks_ptr(uptr p)
    {
        return p > 0x10000ULL && p < 0x00007FFFFFFFFFFFULL;
    }

    static bool try_hashset(uptr hashset, uptr& out_vals, i32& out_count)
    {
        if (!looks_ptr(hashset)) return false;

        // Common: values@+0x10 count@+0x18
        const uptr layouts[][2] = {
            { 0x10, 0x18 },
            { 0x18, 0x10 },
            { 0x18, 0x20 },
            { 0x10, 0x20 },
        };

        for (const auto& lay : layouts) {
            const uptr vals = memory::read<uptr>(hashset + lay[0]);
            const i32 count = memory::read<i32>(hashset + lay[1]);
            if (!looks_ptr(vals) || count <= 0 || count > 65536) continue;
            // Managed array: first element ptr at +0x20
            if (!looks_ptr(memory::read<uptr>(vals + 0x20))) continue;
            out_vals = vals;
            out_count = count;
            return true;
        }
        return false;
    }

    static bool resolve_projectile_list(uptr& out_vals, i32& out_count)
    {
        namespace lc = offsets::ListComponent_Projectile;
        const uptr klass_rva = lc::ListComponent_C ? lc::ListComponent_C : lc::list_component_c;
        if (!klass_rva) return false;

        const uptr klass = memory::read<uptr>(game::impl::game_assembly + klass_rva);
        if (!looks_ptr(klass)) return false;

        const uptr sf = memory::read<uptr>(klass + lc::static_fields);
        if (!looks_ptr(sf)) return false;

        // Try several static-field layouts used across builds.
        const uptr parent_offs[] = { lc::parent_static, (uptr)0x10, (uptr)0x18, (uptr)0x8 };
        const uptr inst_offs[] = { lc::instance, (uptr)0x8, (uptr)0x0 };

        for (uptr po : parent_offs) {
            for (uptr io : inst_offs) {
                uptr instance = memory::read<uptr>(sf + io);
                if (!looks_ptr(instance) && po != io) {
                    const uptr parent = memory::read<uptr>(sf + po);
                    if (looks_ptr(parent))
                        instance = memory::read<uptr>(parent + io);
                }
                if (!looks_ptr(instance)) continue;

                const uptr buf_offs[] = { lc::buffer, (uptr)0x10, (uptr)0x18, (uptr)0x8 };
                for (uptr bo : buf_offs) {
                    uptr hashset = memory::read<uptr>(instance + bo);
                    if (try_hashset(hashset, out_vals, out_count))
                        return true;
                    // instance itself may be the buffer wrapper
                    if (try_hashset(instance, out_vals, out_count))
                        return true;
                }
            }
        }
        return false;
    }

    void tick()
    {
        const bool enabled = settings.aimbot.weapons.thick_bullet;

        static bool s_was_enabled = false;
        const bool just_turned_off = s_was_enabled && !enabled;
        s_was_enabled = enabled;

        if (!enabled && !just_turned_off) return;
        if (!game::impl::game_assembly) return;

        uptr vals = 0;
        i32 count = 0;
        if (!resolve_projectile_list(vals, count)) {
            static int s_fail = 0;
            if (enabled && (++s_fail % 120) == 1)
                DBG("[thick] resolve failed (ListComponent_C=0x{:x})",
                    (uptr)offsets::ListComponent_Projectile::ListComponent_C);
            return;
        }

        static int s_ok = 0;
        if (enabled && (++s_ok % 120) == 1)
            DBG("[thick] list ok count={} vals=0x{:x}", count, vals);

        i32 written = 0;
        for (i32 i = 0; i < count; i++) {
            const uptr proj = memory::read<uptr>(vals + 0x20 + (i * 0x8));
            if (!looks_ptr(proj)) continue;

            if (enabled) {
                memory::write<f32>(proj + offsets::Projectile::currentThickness, kThickness);
                ++written;
            } else {
                const uptr prefab = memory::read<uptr>(proj + offsets::Projectile::sourceProjectilePrefab);
                if (!looks_ptr(prefab)) continue;
                const f32 original = memory::read<f32>(prefab + offsets::Projectile::currentThickness);
                if (original > 0.0005f && original < 100.0f)
                    memory::write<f32>(proj + offsets::Projectile::currentThickness, original);
            }
        }

        if (enabled && written > 0 && (s_ok % 120) == 1)
            DBG("[thick] wrote thickness on {} projectiles", written);
    }
}
