#include "in_gesture.hpp"

namespace features::misc::in_gesture {
    void tick()
    {
        // Intentionally disabled. Forcing PlayerModel.InGesture every tick
        // (and related gesture pointer nulling) has crashed the game on
        // respawn / animation ticks when the offset or local detection is
        // wrong. Do not re-enable without a dedicated setting and a verified
        // InGesture offset for the current build.
    }
}
