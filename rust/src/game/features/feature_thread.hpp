#pragma once


#include <memory/memory.hpp>
#include <thread>
#include <utility>
#include <Windows.h>

namespace features {

    template <typename Fn>
    inline void run_feature_thread(Fn&& worker)
    {
        std::thread([worker = std::forward<Fn>(worker)]() mutable {
            ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
            memory::pin_thread_stack();
            worker();
        }).detach();
    }

}
