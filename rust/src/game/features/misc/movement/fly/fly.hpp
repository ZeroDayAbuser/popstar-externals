#pragma once
#include <atomic>
namespace features::misc::fly {
    extern std::atomic<bool> g_fly_active;
    void tick();
}
