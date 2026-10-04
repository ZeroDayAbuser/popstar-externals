#include "misc.hpp"

#include "player_flags/player_flags.hpp"
#include "remove_water_drag/remove_water_drag.hpp"
#include "remote_call/remote_call.hpp"

#include "localworld/tod_sky/tod_sky.hpp"
#include "localworld/bright_night/bright_night.hpp"
#include "localworld/layer_toggle/layer_toggle.hpp"

#include "movement/walk_on_water/walk_on_water.hpp"
#include "movement/anti_aim/anti_aim.hpp"
#include "movement/spider_man/spider_man.hpp"
#include "movement/fly/fly.hpp"
#include "movement/silent_walk/silent_walk.hpp"
#include "movement/no_fall/no_fall.hpp"
#include "movement/speed_hack/speed_hack.hpp"
#include "movement/omni_sprint/omni_sprint.hpp"

#include "weapon/weapon_spread/weapon_spread.hpp"
#include "weapon/weapon_recoil/weapon_recoil.hpp"
#include "weapon/instant_bow/instant_bow.hpp"
#include "weapon/instant_eoka/instant_eoka.hpp"
#include "weapon/automatic_weapons/automatic_weapons.hpp"
#include "weapon/melee_range/melee_range.hpp"
#include "weapon/rapid_fire/rapid_fire.hpp"
#include "weapon/hitbox_override/hitbox_override.hpp"
#include "weapon/thick_bullet/thick_bullet.hpp"

#include "animations/in_gesture/in_gesture.hpp"
#include "animations/no_animation/no_animation.hpp"
#include "animations/no_sway/no_sway.hpp"
#include "animations/viewmodel_no_lower/viewmodel_no_lower.hpp"
#include "animations/viewmodel_swap_or_hide/viewmodel_swap_or_hide.hpp"
#include "animations/debug_camera/debug_camera.hpp"
#include "animations/fov/fov.hpp"
#include "animations/local_chams/local_chams.hpp"

#include "RPC/instant_revive/instant_revive.hpp"
#include "RPC/untie_crate/untie_crate.hpp"
#include "RPC/instant_interactions/instant_interactions.hpp"
#include "RPC/fast_loot/fast_loot.hpp"

#include <game/cache/cache.hpp>
#include <game/game.hpp>
#include <sdk/rust/entity/entity.hpp>
#include <mutex>

namespace features::misc {
    using cache::g_cache_epoch;

    void on_tick()
    {
        {
            std::unique_lock<std::recursive_mutex> lock(cache::cache_mut, std::try_to_lock);
            if (!lock.owns_lock()) return;
            if (!game::impl::local_player || !game::impl::local_player->base_address)
                return;
        }

        player_flags::tick();
        tod_sky::tick();
        bright_night::tick();
        remove_water_drag::tick();
        walk_on_water::tick();
        anti_aim::tick();
        spider_man::tick();
        fly::tick();
        silent_walk::tick();
        no_fall::tick();
        speed_hack::tick();
        omni_sprint::tick();
        weapon_spread::tick();
        weapon_recoil::tick();
        instant_bow::tick();
        in_gesture::tick();
        instant_revive::tick();
        instant_eoka::tick();
        untie_crate::tick();
        automatic_weapons::tick();
        melee_range::tick();
        rapid_fire::tick();
        no_sway::tick();
        viewmodel_no_lower::tick();
        viewmodel_swap_or_hide::tick();
        no_animation::tick();
        debug_camera::tick();
        hitbox_override::tick();
        thick_bullet::tick();
        fov::tick();
        local_chams::tick();
        fast_loot::tick();
        instant_interactions::tick();
        layer_toggle::tick();
    }

    void reset_caches()
    {
        g_cache_epoch.fetch_add(1, std::memory_order_release);
    }

    void on_shutdown()
    {
        weapon_recoil::restore_all();
    }
}
