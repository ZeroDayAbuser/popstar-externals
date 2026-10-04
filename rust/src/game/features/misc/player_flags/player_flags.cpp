#include "player_flags.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <settings/settings.hpp>
#include <game/features/feature_util.hpp>

namespace features::misc::player_flags {
    void tick()
    {
        const bool tp_enabled = settings.visuals.local.third_person;

        const uptr base = features::local_base_or_zero();
        if (!base) return;
        i32 original_flags = memory::read<i32>(base + offsets::BasePlayer::playerFlags);
        i32 new_flags = original_flags;

        if (!(original_flags & PlayerFlags::Wounded) && !(original_flags & PlayerFlags::Sleeping))
        {
            if (tp_enabled)
                new_flags |= PlayerFlags::ThirdPersonViewmode;
            else
                new_flags &= ~PlayerFlags::ThirdPersonViewmode;
        }

        if (new_flags != original_flags)
        {
            memory::write<int>(base + offsets::BasePlayer::playerFlags, new_flags);
        }
    }
}
