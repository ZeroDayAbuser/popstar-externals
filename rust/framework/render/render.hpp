#pragma once
#include "../utils/color.hpp"
#include "texture/texture.hpp"
#include <d3d11.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace render {

enum class Fonts {
    Arial13px,
    Arial14px,
    Arial16px,
    NotoSans14px,
    NotoSans16px,
    Pixelmix10px,
    FontAwesome14px,
};

enum class TextFlag : int {
    None       = 0,
    DropShadow = 1 << 0,
    Outline    = 1 << 1,
};
inline constexpr int TextFlagsDropShadow = static_cast<int>(TextFlag::DropShadow);
inline constexpr int TextFlagsOutline    = static_cast<int>(TextFlag::Outline);

enum class GradientType { Vertical, Horizontal };

extern ImDrawList* draw_list;

inline Texture steam_avatar;

void setup();
void set_to_background();
void set_to_foreground();

void add_text(Fonts font, std::string_view text, glm::vec2 pos,
              Color color, int flags = 0, glm::vec2 alignment = {0.0f, 0.0f});
glm::vec2 get_text_size(Fonts font, std::string_view text);
ImFont* get_imfont(Fonts font);

void add_rect(glm::vec2 pos, glm::vec2 size, Color color,
              float rounding = 0.0f, float thickness = 1.0f);
void add_rect_filled(glm::vec2 pos, glm::vec2 size, Color color,
                     float rounding = 0.0f);
void add_rect_gradient(glm::vec2 pos, glm::vec2 size, GradientType type,
                       Color c1, Color c2, float rounding = 0.0f);
void add_shadow_rect(glm::vec2 pos, glm::vec2 size, Color color,
                     float blur_amount = 25.0f, float offset = 10.0f,
                     float rounding = 0.0f);

void add_circle(glm::vec2 center, float radius, Color color,
                float thickness = 1.0f, int segments = 0);
void add_circle_filled(glm::vec2 center, float radius, Color color,
                       int segments = 0);
void add_circle_shadow(glm::vec2 center, float radius, Color color,
                       float blur_amount = 25.0f);

void add_line(glm::vec2 p1, glm::vec2 p2, Color color,
              float thickness = 1.0f);
void add_polyline(const std::vector<glm::vec2>& points, Color color,
                  float thickness = 1.0f, bool closed = false);
void add_shadow_poly(const std::vector<glm::vec2>& points, Color color,
                     float blur_amount = 25.0f);
void add_triangle_filled(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3,
                         Color color);

void add_image(ID3D11ShaderResourceView* srv, glm::vec2 pos, glm::vec2 size,
               float rotation = 0.0f, Color tint = Color::white(),
               float rounding = 0.0f);

void modify_alpha(int start_vtx, int end_vtx, float f);
void modulate_color(int start_vtx, int end_vtx, Color tint);

void gradient_items(glm::vec2 pos, glm::vec2 size, Color c1, Color c2,
                    const std::function<void()>& draw_func,
                    float frequency = 0.75f);

}
