#include "build_obfuscation.hpp"

namespace build_obf {

    volatile std::uint32_t runtime_seed = 0;

    __declspec(noinline) static std::uint32_t opaque_seed() {
        std::uint32_t s = BUILD_SEED;
        s ^= runtime_seed;
        s ^= runtime_seed;
        return s;
    }

    void init_runtime_seed() {
        runtime_seed = opaque_seed();
    }

}
