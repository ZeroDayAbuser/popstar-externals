#pragma once


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <timeapi.h>
#include <dwmapi.h>
#include <winnt.h>
#include <winternl.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <numbers>
#include <print>
#include <ranges>
#include <shared_mutex>
#include <span>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#ifndef LOG
#define LOG(...) do { try { std::println(__VA_ARGS__); } catch (...) {} } while (0)
#endif

#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>

#include <glm/glm.hpp>

struct Vector2 : glm::vec2
{
    using glm::vec2::vec2;
    Vector2() = default;
    Vector2(const glm::vec2 &v) : glm::vec2(v) {}

    Vector2 operator+(const Vector2 &o) const { return Vector2(x + o.x, y + o.y); }
    Vector2 operator-(const Vector2 &o) const { return Vector2(x - o.x, y - o.y); }
    Vector2 operator*(float s) const           { return Vector2(x * s, y * s); }
    Vector2 operator/(float s) const           { return Vector2(x / s, y / s); }
    Vector2 &operator+=(const Vector2 &o)      { x += o.x; y += o.y; return *this; }
    Vector2 &operator-=(const Vector2 &o)      { x -= o.x; y -= o.y; return *this; }

    Vector2 Floored() const { return Vector2(std::floor(x), std::floor(y)); }
    Vector2 Ceiled()  const { return Vector2(std::ceil(x),  std::ceil(y));  }
    Vector2 Rounded() const { return Vector2(std::round(x), std::round(y)); }
};
using Vector3 = glm::vec3;
using Vector4 = glm::vec4;

struct Rect
{
    Vector2 position = Vector2(), size = Vector2();

    Vector2 Min() const { return position; }
    Vector2 Max() const { return position + size; }

    bool IsValid() const
    {
        return position.x > 0.0f && position.y > 0.0f && size.x > 0.0f && size.y > 0.0f;
    }

    bool Intersects(const Vector2 &p, const Vector2 &s) const
    {
        Vector2 _min = p;
        Vector2 _max = p + s;
        return _min.y < Max().y && _max.y > Min().y && _min.x < Max().x && _max.x > Min().x;
    }
};

#include <string_encryption.hpp>

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#ifndef IM_VEC2_CLASS_EXTRA
#define IM_VEC2_CLASS_EXTRA \
    ImVec2(const glm::vec2 &v) : x(v.x), y(v.y) {} \
    operator glm::vec2() const { return glm::vec2(x, y); } \
    operator Vector2() const { return Vector2(x, y); }
#endif
#include <menu/framework/render/imgui/imgui.h>
#include <menu/framework/render/imgui/imgui_internal.h>
#include <menu/framework/render/imgui/imgui_impl_dx11.h>
#include <menu/framework/render/imgui/imgui_impl_win32.h>

#include <menu/overlay_shim.hpp>

#include <menu/framework/color.hpp>
#include <menu/framework/input/input.hpp>
#include <menu/framework/render/render.hpp>
#include <menu/framework/render/assets/font_awesome.hpp>
#include <menu/framework/style/style.hpp>
#include <menu/framework/object/object.hpp>
#include <menu/framework/window/window.hpp>
#include <menu/framework/container/container.hpp>
#include <menu/framework/tab/tab.hpp>
#include <menu/framework/tab/subtab.hpp>

#include <menu/framework/widgets/popup/popup.hpp>
#include <menu/framework/widgets/label/label.hpp>
#include <menu/framework/widgets/button/button.hpp>
#include <menu/framework/widgets/checkbox/checkbox.hpp>
#include <menu/framework/widgets/slider/slider.hpp>
#include <menu/framework/widgets/dropdown/dropdown.hpp>
#include <menu/framework/widgets/multi_dropdown/multi_dropdown.hpp>
#include <menu/framework/widgets/color_picker/color_picker.hpp>
#include <menu/framework/widgets/listbox/listbox.hpp>
#include <menu/framework/widgets/text_input/text_input.hpp>
#include <menu/framework/widgets/confirmation_button/confirmation_button.hpp>
#include <menu/framework/widgets/seperator/seperator.hpp>
#define PAWJOB_MENU_KEYBIND_WIDGET_ENABLED
#include <menu/framework/widgets/keybind/keybind.hpp>

#include <menu/framework/ui/ui.hpp>
