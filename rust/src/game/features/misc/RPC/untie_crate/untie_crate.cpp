#include "untie_crate.hpp"
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <game/cache/cache.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <sdk/rust/entity/entity.hpp>
#include <game/features/feature_util.hpp>
#include <string_encryption.hpp>
#include <glm/vec3.hpp>
#include <string>

namespace features::misc::untie_crate {
    void tick()
    {
        bool enabled = settings.misc.general.instant_untie_crate;
        if (!enabled)
            return;

        if (!features::local_base_or_zero())
            return;

        constexpr f32 kMaxRangeSq = 5.0f * 5.0f;
        const glm::vec3 me = game::impl::local_player->origin;

        cache::try_for_each_entity<rust::Entity>([&](rust::Entity* e) {
            if (!e || !e->base_address) return;

            const glm::vec3 d = e->origin - me;
            if (d.x * d.x + d.y * d.y + d.z * d.z > kMaxRangeSq) return;

            if (e->type == EntityType::LockedCrate)
            {
                memory::write<f32>(e->base_address + offsets::HackableLockedCrate::hackSeconds, 9999.0f);
                return;
            }

            const bool name_match =
                e->type == EntityType::UnderwaterCrate ||
                e->prefab_name.find("freeable") != std::string::npos ||
                e->prefab_name.find("tied")     != std::string::npos ||
                e->prefab_name.find("underwater_labs/crate") != std::string::npos;
            if (!name_match) return;

            (void)e;
        });
    }
}
