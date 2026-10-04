#pragma once
#include <atomic>
#include <sdk/globals.hpp>

namespace features::misc::remote_call {

    // Queues managed calls for execution on the game thread.
    enum class State { Idle, Installed, PendingClear };
    extern std::atomic<State> g_state;
    extern uptr               g_action_slot;

    void Run();

    void Call(uptr function, uptr rcx = 0, uptr rdx = 0, uptr r8 = 0);

    uptr CreateString(const wchar_t* string, int index = 1);

    void tick();
    bool call_one_arg(uptr fn, uptr rcx);
}
