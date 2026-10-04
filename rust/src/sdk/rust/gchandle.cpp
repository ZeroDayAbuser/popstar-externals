#include "gchandle.hpp"

#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>

namespace gchandle {

uptr get_target(uintptr_t encrypted) {
    if (!encrypted || !game::impl::game_assembly)
        return 0;

    if ((encrypted & 1) == 0)
        return memory::read<uptr>(encrypted);

    const u32 handle = static_cast<u32>(encrypted);
    const u32 slot = get_handle_slot(handle);
    const u32 type = static_cast<u32>(get_handle_type(handle));
    if (type > 3 || slot > 0x100000u)
        return 0;

    const uptr table = game::impl::game_assembly + offsets::il2cpp::get_handle +
                       static_cast<uptr>(type) * offsets::il2cpp::gchandle_stride;

    const handle_data gc = memory::read<handle_data>(table);
    if (!gc.entries || gc.size == 0 || slot >= gc.size)
        return 0;

    const uptr entries = reinterpret_cast<uptr>(gc.entries);
    if (entries < 0x100000000ULL || entries > 0x00007FFFFFFFFFFFULL)
        return 0;

    uptr obj = memory::read<uptr>(entries + static_cast<uptr>(slot) * sizeof(void*));
    if (gc.type <= HANDLE_WEAK_TRACK)
        obj = ~obj;

    if (obj < 0x100000000ULL || obj > 0x00007FFFFFFFFFFFULL)
        return 0;
    return obj;
}

}
