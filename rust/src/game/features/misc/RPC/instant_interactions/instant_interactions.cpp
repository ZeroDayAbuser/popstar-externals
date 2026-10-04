#include "instant_interactions.hpp"
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <string_encryption.hpp>

namespace features::misc::instant_interactions {
    void tick()
    {
        bool enabled = settings.misc.general.instant_interactions;
        if (!enabled) return;

        const uptr klass = memory::read<uptr>(
            game::impl::game_assembly + offsets::ProgressBar::typeinfo);
        if (!klass) return;
        const uptr static_fields = memory::read<uptr>(
            klass + offsets::ProgressBar::static_fields);
        if (!static_fields) return;
        const uptr instance = memory::read<uptr>(
            static_fields + offsets::ProgressBar::instance);
        if (!instance) return;

        memory::write<f32>(instance + offsets::ProgressBar::timeCounter, 9999.0f);
    }
}
