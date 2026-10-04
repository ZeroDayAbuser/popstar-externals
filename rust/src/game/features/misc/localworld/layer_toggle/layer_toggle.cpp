#include "layer_toggle.hpp"

#include <game/game.hpp>
#include <memory/memory.hpp>
#include <settings/settings.hpp>
#include <cstdint>
#include <Windows.h>
#include <sdk/offsets.hpp>

namespace features::misc::layer_toggle {

    static constexpr std::int32_t kDefaultMask = -537953485;

    static constexpr std::int32_t kBitConstruction = 2097152;
    static constexpr std::int32_t kBitTransparent  = 16777216;
    static constexpr std::int32_t kBitDebris       = 67108864;
    static constexpr std::int32_t kBitDefault      = 1;
    static constexpr std::int32_t kBitDeployed     = 256;
    static constexpr std::int32_t kBitRagdoll      = 512;
    static constexpr std::int32_t kBitTerrain      = 8388608;
    static constexpr std::int32_t kBitTree         = 1073741824;
    static constexpr std::int32_t kBitWorld        = 65536;
    static constexpr std::int32_t kBitWater        = 16;
    static constexpr std::int32_t kBitClutter      = 33554432;

    static uptr s_culling_mask_off = 0;

    static uptr scan_culling_mask_offset(uptr native_camera)
    {
        for (uptr off = 0x100; off <= 0x300; off += 4) {
            const std::int32_t v = memory::read<std::int32_t>(native_camera + off);
            if (v == kDefaultMask) return off;
        }
        return 0;
    }

    void tick()
    {
        auto& s = settings.misc.layer_toggle;
        if (!s.enabled) return;

        const int vk = s.key.key;
        if (!vk) return;

        static bool prev_down = false;
        const bool now_down = (::GetAsyncKeyState(vk) & 0x8000) != 0;
        const bool just_pressed = now_down && !prev_down;
        prev_down = now_down;
        if (!just_pressed) return;

        const uptr native_cam = game::impl::camera_object;
        if (!native_cam) return;

        if (!s_culling_mask_off) {
            s_culling_mask_off = scan_culling_mask_offset(native_cam);
            if (!s_culling_mask_off)
                s_culling_mask_off = offsets::camera::culling_mask;
            if (!s_culling_mask_off) return;
        }

        s.active = !s.active;

        std::int32_t mask = kDefaultMask;
        if (s.active) {
            if (s.hide_construction) mask &= ~kBitConstruction;
            if (s.hide_transparent)  mask &= ~kBitTransparent;
            if (s.hide_debris)       mask &= ~kBitDebris;
            if (s.hide_default)      mask &= ~kBitDefault;
            if (s.hide_deployed)     mask &= ~kBitDeployed;
            if (s.hide_ragdoll)      mask &= ~kBitRagdoll;
            if (s.hide_terrain)      mask &= ~kBitTerrain;
            if (s.hide_tree)         mask &= ~kBitTree;
            if (s.hide_world)        mask &= ~kBitWorld;
            if (s.hide_water)        mask &= ~kBitWater;
            if (s.hide_clutter)      mask &= ~kBitClutter;
        }

        memory::write<std::int32_t>(native_cam + s_culling_mask_off, mask);
    }
}
