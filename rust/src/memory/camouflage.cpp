#include "camouflage.hpp"

#include <atomic>
#include <chrono>

namespace camouflage {

namespace {

    using clock = std::chrono::steady_clock;

    inline uint64_t now_us() {
        return (uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(
            clock::now().time_since_epoch()).count();
    }

    constexpr uint32_t kDefaultThrottleCap = 1500;       // reads/sec
    constexpr uint64_t kWindowUs           = 1'000'000;  // 1s

    std::atomic<uint32_t> g_cap{ kDefaultThrottleCap };

    std::atomic<uint64_t> g_window_start_us{ 0 };
    std::atomic<uint32_t> g_window_count{ 0 };

    constexpr uint64_t kKeepaliveUs = 30'000'000;  // 30s

    std::atomic<bool>     g_dormant{ false };
    std::atomic<uint64_t> g_last_keepalive_us{ 0 };
}

void notify_dormant(bool dormant) {
    g_dormant.store(dormant, std::memory_order_release);
    if (!dormant) {
        g_last_keepalive_us.store(now_us(), std::memory_order_release);
    }
}

void set_throttle_cap(uint32_t reads_per_sec) {
    g_cap.store(reads_per_sec, std::memory_order_release);
}

bool should_suppress_read() {
    const uint64_t now = now_us();

    if (g_dormant.load(std::memory_order_acquire)) {
        uint64_t last = g_last_keepalive_us.load(std::memory_order_acquire);
        if (now - last >= kKeepaliveUs) {
            if (g_last_keepalive_us.compare_exchange_strong(
                    last, now,
                    std::memory_order_acq_rel,
                    std::memory_order_acquire))
            {
                return false;  // let this one through
            }
        }
        return true;  // suppress
    }

    const uint32_t cap = g_cap.load(std::memory_order_acquire);
    if (cap == 0) return false;  // throttle disabled

    uint64_t window_start = g_window_start_us.load(std::memory_order_acquire);
    if (now - window_start >= kWindowUs) {
        if (g_window_start_us.compare_exchange_strong(
                window_start, now,
                std::memory_order_acq_rel,
                std::memory_order_acquire))
        {
            g_window_count.store(0, std::memory_order_release);
        }
    }

    const uint32_t n = g_window_count.fetch_add(1, std::memory_order_acq_rel);
    return n >= cap;
}

uint32_t reads_in_current_window() {
    return g_window_count.load(std::memory_order_acquire);
}

} // namespace camouflage
