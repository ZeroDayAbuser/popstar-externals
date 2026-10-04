#include "fast_loot.hpp"
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <string_encryption.hpp>
#include <cfloat>
#include <cstdint>

namespace features::misc::fast_loot {

    void tick()
    {
        if (!settings.misc.general.fast_loot)
            return;

        static uptr cached_dict_addr = 0;

        if (!cached_dict_addr) {
            uptr klass = memory::read<uptr>(game::impl::game_assembly + offsets::item_icon::typeinfo);
            if (!klass) return;

            uptr sf = memory::read<uptr>(klass + offsets::item_icon::static_fields);
            if (!sf) return;

            cached_dict_addr = sf + offsets::item_icon::containerLootStartTimes;
        }

        uptr dict = memory::read<uptr>(cached_dict_addr);
        if (!dict) return;

        uptr entries = memory::read<uptr>(dict + offsets::item_icon::dict_pointer);
        if (!entries) return;

        auto length = memory::read<std::int32_t>(entries + offsets::item_icon::dict_count);
        if (length <= 0 || length > 4096) return;

        for (std::int32_t i = 0; i < length; ++i)
        {
            uptr addr = entries
                + offsets::item_icon::dict_data
                + static_cast<uptr>(i) * offsets::item_icon::dict_stride
                + offsets::item_icon::entry_timer;
            memory::write<float>(addr, FLT_MIN);
        }
    }
}
