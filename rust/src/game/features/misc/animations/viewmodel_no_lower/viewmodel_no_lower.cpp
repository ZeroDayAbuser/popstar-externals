#include "viewmodel_no_lower.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <game/features/feature_util.hpp>
#include <string_encryption.hpp>

namespace features::misc::viewmodel_no_lower {
    void tick()
    {
        bool enabled = settings.aimbot.weapons.no_viewmodel_lower;
        if (!enabled) return;

        uptr held_entity = features::held_weapon_or_zero();
        if (!held_entity) return;

        uptr view_model = memory::read(held_entity + offsets::HeldEntity::view_model);
        if (!view_model) return;

        uptr lower = memory::read(view_model + offsets::BaseViewModel::lower);
        if (!lower) return;

        memory::write<bool>(lower + offsets::viewmodel_lower::lowerOnSprint,       false); // 0x20
        memory::write<bool>(lower + offsets::viewmodel_lower::lowerWhenCantAttack, false); // 0x21
        memory::write<bool>(lower + offsets::viewmodel_lower::shouldLower,         false); // 0x28
        memory::write<f32> (lower + offsets::viewmodel_lower::rotateAngle,         0.0f);  // 0x2c
    }
}
