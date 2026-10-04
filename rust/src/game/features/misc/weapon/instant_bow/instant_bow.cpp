#include "instant_bow.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <game/features/feature_util.hpp>
#include <string_encryption.hpp>
#include <string>
#include <Windows.h>
#include <chrono>

namespace features::misc::instant_bow {

    static constexpr auto kMinDraw = std::chrono::milliseconds(200);

    void tick()
    {
        const bool enabled = settings.aimbot.weapons.instant_bow;

        static bool last_rmb = false;
        static std::chrono::steady_clock::time_point rmb_start{};
        const bool rmb_held = (::GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        if (rmb_held && !last_rmb)
            rmb_start = std::chrono::steady_clock::now();
        last_rmb = rmb_held;

        if (!enabled) return;
        if (!rmb_held) return;                       // only during actual aim
        if (!game::impl::local_player) return;
        if (game::impl::local_player->item_shortname.find(xs("bow")) == std::string::npos)
            return;

        const uptr base_projectile = features::held_weapon_or_zero();
        if (!base_projectile) return;

        if (std::chrono::steady_clock::now() - rmb_start < kMinDraw)
            return;

        memory::write<bool>(base_projectile + offsets::bow_weapon::attackReady, true);
    }
}
