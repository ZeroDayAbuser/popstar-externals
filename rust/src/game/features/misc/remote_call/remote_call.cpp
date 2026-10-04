#include "remote_call.hpp"

#include <cstddef>
#include <cstring>
#include <deque>
#include <mutex>

#include <game/game.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>


namespace features::misc::remote_call {

    struct alignas(8) IlAction {
        uptr invoke_impl;
        uptr method_code;
        uptr method;
        char pad[0x50 - 3 * sizeof(uptr)];
    };
    static_assert(sizeof(IlAction) == 0x50,
                  "IlAction must be exactly 0x50 bytes");

    struct RemoteCall {
        IlAction mA;
        IlAction mB;
    };

    static constexpr uptr        kAllocationOffsets[4] = { 0x500, 0x700, 0x900, 0xB00 };
    static constexpr std::size_t kAllocationSlotCount  =
        sizeof(kAllocationOffsets) / sizeof(kAllocationOffsets[0]);
    static constexpr std::size_t kQueueCapacity        = 64;

    std::atomic<State> g_state{ State::Idle };
    uptr               g_action_slot = 0;

    static std::mutex             s_queue_lock;
    static std::deque<RemoteCall> s_queue;
    static bool                   s_restore = false;

    static uptr resolve_static_fields(uptr typeinfo_rva, uptr static_fields_offset)
    {
        if (game::impl::game_assembly == 0) return 0;
        if (typeinfo_rva == 0)              return 0;

        const uptr klass = memory::read<uptr>(game::impl::game_assembly + typeinfo_rva);
        if (!klass) return 0;

        return memory::read<uptr>(klass + static_fields_offset);
    }

    static uptr resolve_server_static_fields()
    {
        return resolve_static_fields(
            offsets::convar_server_static::typeinfo,
            offsets::convar_server_static::static_fields);
    }

    static uptr get_allocation(uptr server_sf)
    {
        return server_sf + kAllocationOffsets[0];
    }

    static bool resolve_all(uptr& server_sf,
                            uptr& camera_hook_action_slot,
                            uptr& received_data_from_server_addr)
    {
        server_sf                       = 0;
        camera_hook_action_slot         = 0;
        received_data_from_server_addr  = 0;

        if (game::impl::game_assembly == 0) return false;

        server_sf = resolve_server_static_fields();
        if (!server_sf) return false;

        const uptr cam_sf = resolve_static_fields(
            offsets::camera_update_hook_static::typeinfo,
            offsets::camera_update_hook_static::static_fields);
        if (!cam_sf) return false;
        camera_hook_action_slot = cam_sf + offsets::camera_update_hook_static::action;

        if (offsets::server_admin_ugc_entry::ReceivedDataFromServer_rva == 0) return false;
        received_data_from_server_addr =
            game::impl::game_assembly
            + offsets::server_admin_ugc_entry::ReceivedDataFromServer_rva;

        return true;
    }

    void Run()
    {
        std::lock_guard<std::mutex> lock(s_queue_lock);

        uptr server_sf        = 0;
        uptr action_slot      = 0;
        uptr received_data_fn = 0;
        if (!resolve_all(server_sf, action_slot, received_data_fn)) {
            g_state.store(State::Idle, std::memory_order_release);
            return;
        }

        if (!s_queue.empty()) {
            RemoteCall call = std::move(s_queue.front());
            s_queue.pop_front();

            const uptr allocation = get_allocation(server_sf);

            memory::write<IlAction>(allocation + 0x00, call.mA);
            memory::write<IlAction>(allocation + 0x50, call.mB);
            memory::write<uptr>    (allocation + 0xA0, allocation);

            memory::write<uptr>(action_slot, allocation + 0x50);

            g_action_slot = action_slot;
            s_restore     = true;
            g_state.store(State::Installed, std::memory_order_release);
        }
        else if (s_restore) {
            memory::write<uptr>(action_slot, 0);
            g_action_slot = 0;
            s_restore     = false;
            g_state.store(State::PendingClear, std::memory_order_release);
        }
        else {
            g_state.store(State::Idle, std::memory_order_release);
        }
    }

    void Call(uptr function, uptr rcx, uptr rdx, uptr r8)
    {
        if (!function) return;

        std::lock_guard<std::mutex> lock(s_queue_lock);

        if (s_queue.size() >= kQueueCapacity) return;

        if (game::impl::game_assembly == 0) return;
        if (offsets::server_admin_ugc_entry::ReceivedDataFromServer_rva == 0) return;

        const uptr server_sf = resolve_server_static_fields();
        if (!server_sf) return;

        RemoteCall call{};   // zero-init pad bytes so we don't smuggle stack garbage
        call.mA.invoke_impl = function;
        call.mA.method_code = rcx;
        call.mA.method      = r8;

        call.mB.invoke_impl = game::impl::game_assembly
                              + offsets::server_admin_ugc_entry::ReceivedDataFromServer_rva;
        call.mB.method_code = get_allocation(server_sf) + 0x50;  // &mB
        call.mB.method      = rdx;

        s_queue.push_back(call);
    }

    uptr CreateString(const wchar_t* string, int index)
    {
        if (!string) return 0;
        if (index < 0 || static_cast<std::size_t>(index) >= kAllocationSlotCount) return 0;
        (void)string;
        return 0;
    }

    void tick() { Run(); }

    bool call_one_arg(uptr fn, uptr rcx)
    {
        if (!fn) return false;

        std::size_t before = 0;
        {
            std::lock_guard<std::mutex> lock(s_queue_lock);
            before = s_queue.size();
        }
        Call(fn, rcx, 0, 0);
        std::lock_guard<std::mutex> lock(s_queue_lock);
        return s_queue.size() > before;
    }
}
