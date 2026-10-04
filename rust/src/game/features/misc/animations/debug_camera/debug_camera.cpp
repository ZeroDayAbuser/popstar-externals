#include "debug_camera.hpp"
#include <game/game.hpp>
#include <game/features/feature_thread.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <settings/settings.hpp>
#include <atomic>
#include <chrono>
#include <cmath>
#include <thread>
#include <Windows.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/geometric.hpp>

namespace features::misc::debug_camera {

    static std::atomic<bool> g_debug_cam_started{ false };

    static void debug_camera_worker()
    {
        bool   enabled   = false;
        f32    speed     = 10.0f;
        f32    sens_pct  = 40.0f;
        int    toggle_vk = 0;
        auto   last_read = std::chrono::steady_clock::now();
        auto   last_step = std::chrono::steady_clock::now();
        bool   prev_toggle_down = false;
        bool   user_active = false;
        bool   pos_initialized = false;

        glm::vec3 pos{};
        float yaw   = 0.0f;
        float pitch = 0.0f;

        POINT lastMouse{};
        ::GetCursorPos(&lastMouse);

        while (true)
        {
            if (!game::is_in_game())
            {
                pos_initialized = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            const uptr cam = game::impl::camera_object;
            if (!cam)
            {
                pos_initialized = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            const auto now = std::chrono::steady_clock::now();
            if (now - last_read >= std::chrono::milliseconds(100))
            {
                enabled   = settings.visuals.local.debug_camera;
                speed     = settings.visuals.local.debug_camera_speed;
                sens_pct  = settings.visuals.local.mouse_sensitivity;
                toggle_vk = settings.visuals.local.debug_camera_key.key;
                last_read = now;
            }

            if (toggle_vk != 0)
            {
                const bool down = (::GetAsyncKeyState(toggle_vk) & 0x8000) != 0;
                if (down && !prev_toggle_down) user_active = !user_active;
                prev_toggle_down = down;
            }
            else
            {
                prev_toggle_down = false;
            }

            const bool active = enabled && user_active;
            if (!active)
            {
                pos_initialized = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }

            if (!pos_initialized)
            {
                const glm::mat4 vm = game::impl::view_matrix;
                const glm::mat4 inv = glm::inverse(vm);
                pos.x = inv[3][0];
                pos.y = inv[3][1];
                pos.z = inv[3][2];

                const glm::vec3 fwd{ -inv[2][0], -inv[2][1], -inv[2][2] };
                const glm::vec3 fwd_n = glm::normalize(fwd);
                pitch = std::asin(fwd_n.y);
                yaw   = std::atan2(fwd_n.x, fwd_n.z);

                ::GetCursorPos(&lastMouse);
                last_step = now;
                pos_initialized = true;
            }

            const float dt = std::chrono::duration<float>(now - last_step).count();
            last_step = now;

            POINT mp{};
            ::GetCursorPos(&mp);
            const float dx = (float)(mp.x - lastMouse.x);
            const float dy = (float)(mp.y - lastMouse.y);
            lastMouse = mp;

            const float sens = (sens_pct * 0.01f) * 0.003f;
            yaw   -= dx * sens;
            pitch -= dy * sens;
            constexpr float kPitchMax = 1.5533f;
            if (pitch >  kPitchMax) pitch =  kPitchMax;
            if (pitch < -kPitchMax) pitch = -kPitchMax;

            const glm::vec3 fwd{
                std::sin(yaw) * std::cos(pitch),
                std::sin(pitch),
                std::cos(yaw) * std::cos(pitch)
            };
            const glm::vec3 world_up{ 0.0f, 1.0f, 0.0f };
            const glm::vec3 right = glm::normalize(glm::cross(fwd, world_up));
            const glm::vec3 up    = glm::cross(right, fwd);

            glm::vec3 mv{ 0.0f };
            const auto held = [](int vk) {
                return (::GetAsyncKeyState(vk) & 0x8000) != 0;
            };
            if (held('W'))         mv += fwd;
            if (held('S'))         mv -= fwd;
            if (held('D'))         mv += right;
            if (held('A'))         mv -= right;
            if (held(VK_SPACE))    mv += world_up;
            if (held(VK_CONTROL))  mv -= world_up;
            const float sprint_mul = held(VK_SHIFT) ? 2.5f : 1.0f;
            if (glm::dot(mv, mv) > 1e-6f)
                pos += glm::normalize(mv) * (speed * sprint_mul * dt);

            const glm::mat4 view = glm::lookAt(pos, pos + fwd, up);
            memory::write<glm::mat4>(cam + offsets::main_camera::view_matrix, view);

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void tick()
    {
        if (!g_debug_cam_started.exchange(true))
            features::run_feature_thread(debug_camera_worker);
    }
}
