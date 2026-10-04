#include "viewmodel_swap_or_hide.hpp"
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <sdk/rust/entity/entity.hpp>
#include <game/features/feature_util.hpp>
#include <string_encryption.hpp>

namespace features::misc::viewmodel_swap_or_hide {
    void tick()
    {
        bool enabled = settings.aimbot.weapons.hide_viewmodel;
        if (!enabled) return;

        uptr held_entity = features::held_weapon_or_zero();
        if (!held_entity) return;

        uptr view_model = memory::read(held_entity + offsets::HeldEntity::view_model);
        if (!view_model) return;

        memory::write<uptr>(view_model + offsets::BaseViewModel::model, 0);
    }
}
