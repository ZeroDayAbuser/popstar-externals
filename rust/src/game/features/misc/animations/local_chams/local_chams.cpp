#include "local_chams.hpp"
#include <game/game.hpp>
#include <game/features/feature_util.hpp>
#include <memory/memory.hpp>
#include <sdk/globals.hpp>
#include <sdk/offsets.hpp>
#include <sdk/unity/unity.hpp>
#include <settings/settings.hpp>
#include <string>
#include <vector>

namespace features::misc::local_chams {

    static void apply_material_to_render(uptr render, u32 material_id)
    {
        for (i32 idx = 0; idx < 2; idx++) {
            uptr render_entry = memory::read(render + 0x20 + (idx * 0x8));
            if (!render_entry) continue;

            uptr unity_object = memory::read(render_entry + 0x10);
            if (!unity_object) continue;

            const uptr mat_off = offsets::SkinnedMultiMesh::rendererMaterialArray;
            uptr material_list_base = memory::read(unity_object + mat_off);
            uptr material_list_size = memory::read(unity_object + mat_off + 0x10);

            if (!material_list_base || material_list_size < 1 || material_list_size > 8) continue;

            for (u32 i = 0; i < material_list_size; i++)
                memory::write<u32>(material_list_base + (i * 0x4), material_id);
        }
    }

    void tick()
    {
        if (!game::impl::local_player)
            return;

        const i32 weapon_chams = settings.visuals.local.weapon_chams;
        const i32 arm_chams    = settings.visuals.local.arm_chams;
        if (weapon_chams == 0 && arm_chams == 0)
            return;

        static uptr last_active_item_ptr = 0;
        static std::string last_active_item_name;
        static i32 last_weapon_chams = -1;
        static i32 last_arm_chams = -1;

        const uptr held = features::held_weapon_or_zero();
        const std::string item_name = game::impl::local_player->item_name;
        if (held == last_active_item_ptr &&
            item_name == last_active_item_name &&
            weapon_chams == last_weapon_chams &&
            arm_chams == last_arm_chams)
            return;

        last_active_item_ptr = 0;
        last_active_item_name.clear();
        last_weapon_chams = -1;
        last_arm_chams = -1;

        if (!held) return;

        uptr viewmodel = memory::read(held + offsets::HeldEntity::view_model);
        if (!viewmodel) return;

        uptr viewmodel_instance = memory::read(viewmodel + offsets::view_model::instance);
        if (!viewmodel_instance) return;

        uptr base_viewmodel = memory::read(viewmodel_instance + 0x10);
        if (!base_viewmodel) return;

        memory::write<bool>(viewmodel_instance + offsets::BaseViewModel::useViewModelCamera, false);

        uptr viewmodel_object = memory::read(base_viewmodel + 0x30);
        if (!viewmodel_object) return;

        std::vector<uptr> render_weapon;
        std::vector<uptr> render_hand;
        unity::get_components_in_children(viewmodel_object, render_weapon, render_hand);

        const i32 mat_count = static_cast<i32>(g_materials.size());
        if (weapon_chams > 0 && weapon_chams < mat_count) {
            for (auto render : render_weapon)
                apply_material_to_render(render, g_materials[weapon_chams]);
        }
        if (arm_chams > 0 && arm_chams < mat_count) {
            for (auto render : render_hand)
                apply_material_to_render(render, g_materials[arm_chams]);
        }

        last_active_item_ptr = held;
        last_active_item_name = item_name;
        last_weapon_chams = weapon_chams;
        last_arm_chams = arm_chams;
    }
}
