#include "automatic_weapons.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <game/features/feature_util.hpp>
#include <string_encryption.hpp>
#include <string>

namespace features::misc::automatic_weapons {
    void tick()
    {
        bool enabled = settings.aimbot.weapons.automatic_weapons;
        if (!enabled)
            return;

        if (!game::impl::local_player)
            return;

        if (game::impl::local_player->item_shortname.find(xs("bow")) != std::string::npos)
            return;

        uptr base_projectile = features::held_weapon_or_zero();
        if (!base_projectile)
            return;

        memory::write<bool>(base_projectile + offsets::BaseProjectile::automatic, true);
    }
}
