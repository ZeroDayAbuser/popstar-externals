#pragma once
#include "../window/window.hpp"

#include "../base/object/object.hpp"
#include "../utils/color.hpp"
#include "../utils/style.hpp"
#include "../render/render.hpp"
#include "../render/texture/texture.hpp"
#include "../controls/button/button.hpp"
#include "../controls/checkbox/checkbox.hpp"
#include "../controls/color_picker/color_picker.hpp"
#include "../controls/container/container.hpp"
#include "../controls/dropdown/dropdown.hpp"
#include "../controls/label/label.hpp"
#include "../controls/listbox/listbox.hpp"
#include "../controls/multi_dropdown/multi_dropdown.hpp"
#include "../controls/popup/popup.hpp"
#include "../controls/slider/slider.hpp"
#include "../controls/subtab_bar/subtab_bar.hpp"
#include "../controls/tab/tab.hpp"
#include "../controls/tab_control/tab_control.hpp"
#include "../controls/text_input/text_input.hpp"

#include <array>
#include <functional>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>

namespace app
{
    inline std::vector<std::shared_ptr<gui::Window>> windows;
    inline ID3D11Device* device = nullptr;

    inline std::recursive_mutex gui_mutex;

    void setup();
    void render();
    void handle_debug_hotkey();
    bool on_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    gui::Object* find_widget_recursive(gui::Object* current, std::span<const std::string_view> paths, std::size_t depth);

    template <typename T, typename... Args>
    T* get_widget(const std::string_view& window_id, const Args &...path_ids)
    {
        if constexpr (sizeof...(Args) == 0)
        {
            return nullptr;
        }
        else
        {
            std::lock_guard lock(gui_mutex);

            std::array<std::string_view, sizeof...(Args)> paths = { std::string_view(path_ids)... };
            for (auto& window : windows)
            {
                if (window->m_name == window_id)
                {
                    gui::Object* found = find_widget_recursive(window.get(), paths, 0);
                    return dynamic_cast<T*>(found);
                }
            }

            return nullptr;
        }
    }

    template <typename T, typename... Args>
    auto& get_value(std::string_view window_id, Args... path_ids)
    {
        if (T* widget = get_widget<T>(window_id, path_ids...))
            return widget->value;

        struct Slot { decltype(T::value) val{}; };
        thread_local std::unordered_map<std::size_t, Slot> miss_slots;
        std::size_t h = std::hash<std::string_view>{}(window_id);
        ((h ^= std::hash<std::string_view>{}(std::string_view(path_ids))
              + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2)), ...);
        return miss_slots[h].val;
    }
}
