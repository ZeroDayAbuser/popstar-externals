#pragma once

#include <cstdint>
#include <cstddef>

namespace camouflage {

    void notify_dormant(bool dormant);

    void set_throttle_cap(uint32_t reads_per_sec);

    bool should_suppress_read();

    uint32_t reads_in_current_window();
}
