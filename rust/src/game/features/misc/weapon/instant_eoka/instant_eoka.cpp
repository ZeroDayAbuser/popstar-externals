#include "instant_eoka.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <game/features/feature_util.hpp>
#include <string_encryption.hpp>
#include <cstdlib>
#include <string>

namespace features::misc::instant_eoka {
    void tick()
    {
        bool enabled = settings.aimbot.weapons.instant_eoka;
        if (!enabled)
            return;

        if (!game::impl::local_player)
            return;

        if (game::impl::local_player->item_shortname.find(xs("eoka")) == std::string::npos)
            return;

        uptr base_projectile = features::held_weapon_or_zero();
        if (!base_projectile)
            return;

        const int chance = settings.aimbot.weapons.instant_eoka_strikechance;
        const f32 fraction = chance >= 100 ? 1.0f : (static_cast<f32>(chance) / 100.0f);
        memory::write<f32>(base_projectile + offsets::FlintStrikeWeapon::successFraction, fraction);
        if (chance >= 100 || (rand() % 100) < chance) {
            if (!memory::read<bool>(base_projectile + offsets::FlintStrikeWeapon::didSparkThisFrame))
                memory::write<bool>(base_projectile + offsets::FlintStrikeWeapon::didSparkThisFrame, true);
        }
    }
}
