#include "instant_revive.hpp"
#include <sdk/rust/entity/entity.hpp>
#include "../../remote_call/remote_call.hpp"
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <game/features/feature_util.hpp>
#include <string_encryption.hpp>
#include <chrono>
#include <cstdint>
#include <string>
#include <Windows.h>

namespace features::misc::instant_revive {
    void tick()
    {
        remote_call::tick();

        bool enabled = settings.misc.general.instant_revive;
        if (!enabled) return;

        const int vk = settings.misc.general.instant_revive_key.key;
        if (!vk) return;

        static bool prev = false;
        const bool now = (::GetAsyncKeyState(vk) & 0x8000) != 0;
        const bool just_pressed = now && !prev;
        prev = now;
        if (!just_pressed) return;

        const uptr local = features::local_base_or_zero();
        if (!local) return;
        const uptr target = memory::read<uptr>(local + offsets::BasePlayer::_lookingAtEntity);
        if (!target || target == local) return;

        static uptr s_baseplayer_klass = 0;
        if (!s_baseplayer_klass)
            s_baseplayer_klass = memory::read<uptr>(game::impl::game_assembly + offsets::BasePlayer_Class::typeinfo);
        const uptr target_klass = memory::read<uptr>(target);
        if (!s_baseplayer_klass || target_klass != s_baseplayer_klass) return;

        if (offsets::BasePlayer::Menu_AssistPlayer_rva == 0) return;

        remote_call::call_one_arg(
            game::impl::game_assembly + offsets::BasePlayer::Menu_AssistPlayer_rva,
            target);
    }
}
