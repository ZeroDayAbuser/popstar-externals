#include "render.hpp"
#include <cmath>

namespace render {

ImDrawList* draw_list = nullptr;

static ImFont* s_font_arial13 = nullptr;
static ImFont* s_font_arial14 = nullptr;
static ImFont* s_font_arial16 = nullptr;
static ImFont* s_font_noto14  = nullptr;
static ImFont* s_font_noto16  = nullptr;
static ImFont* s_font_pixel10 = nullptr;
static ImFont* s_font_fa14    = nullptr;

void setup() {
    auto& io = ImGui::GetIO();
    if (io.Fonts && io.Fonts->Fonts.Size > 0)
        s_font_arial14 = io.Fonts->Fonts[0];
    s_font_arial13 = s_font_arial14;
    s_font_arial16 = s_font_arial14;
    s_font_noto14  = s_font_arial14;
    s_font_noto16  = s_font_arial14;
    s_font_pixel10 = s_font_arial14;
    s_font_fa14    = s_font_arial14;
}

void set_to_background() { draw_list = ImGui::GetBackgroundDrawList(); }
void set_to_foreground() { draw_list = ImGui::GetForegroundDrawList(); }

ImFont* get_imfont(Fonts font) {
    switch (font) {
        case Fonts::Arial13px:      return s_font_arial13;
        case Fonts::Arial14px:      return s_font_arial14;
        case Fonts::Arial16px:      return s_font_arial16;
        case Fonts::NotoSans14px:   return s_font_noto14;
        case Fonts::NotoSans16px:   return s_font_noto16;
        case Fonts::Pixelmix10px:   return s_font_pixel10;
        case Fonts::FontAwesome14px:return s_font_fa14;
    }
    return s_font_arial14;
}

static float font_size_hint(Fonts f) {
    switch (f) {
        case Fonts::Arial13px:    return 13.0f;
        case Fonts::Arial14px:    return 14.0f;
        case Fonts::Arial16px:    return 16.0f;
        case Fonts::NotoSans14px: return 14.0f;
        case Fonts::NotoSans16px: return 16.0f;
        case Fonts::Pixelmix10px: return 10.0f;
        case Fonts::FontAwesome14px: return 14.0f;
    }
    return 14.0f;
}

void add_text(Fonts font, std::string_view text, glm::vec2 pos, Color color,
              int flags, glm::vec2 alignment)
{
    if (!draw_list || text.empty()) return;
    ImFont* f = get_imfont(font);
    const float sz = font_size_hint(font);
    const std::string s(text);
    const ImVec2 measured = f
        ? f->CalcTextSizeA(sz, FLT_MAX, 0.0f, s.c_str())
        : ImGui::CalcTextSize(s.c_str());

    const float x = pos.x - measured.x * alignment.x;
    const float y = pos.y - measured.y * alignment.y;
    const ImVec2 anchor{x, y};

    if (flags & TextFlagsDropShadow) {
        const Color shadow{0, 0, 0, color.a};
        if (f) draw_list->AddText(f, sz, {anchor.x + 1, anchor.y + 1}, shadow.to_imgui(), s.c_str());
        else   draw_list->AddText({anchor.x + 1, anchor.y + 1}, shadow.to_imgui(), s.c_str());
    }
    if (flags & TextFlagsOutline) {
        const Color outline{0, 0, 0, color.a};
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx)
                if (dx || dy) {
                    if (f) draw_list->AddText(f, sz, {anchor.x + dx, anchor.y + dy}, outline.to_imgui(), s.c_str());
                    else   draw_list->AddText({anchor.x + dx, anchor.y + dy}, outline.to_imgui(), s.c_str());
                }
    }
    if (f) draw_list->AddText(f, sz, anchor, color.to_imgui(), s.c_str());
    else   draw_list->AddText(anchor, color.to_imgui(), s.c_str());
}

glm::vec2 get_text_size(Fonts font, std::string_view text) {
    ImFont* f = get_imfont(font);
    const float sz = font_size_hint(font);
    const std::string s(text);
    ImVec2 m = f
        ? f->CalcTextSizeA(sz, FLT_MAX, 0.0f, s.c_str())
        : ImGui::CalcTextSize(s.c_str());
    return {m.x, m.y};
}

void add_rect(glm::vec2 pos, glm::vec2 size, Color color, float rounding, float thickness) {
    if (!draw_list) return;
    draw_list->AddRect({pos.x, pos.y}, {pos.x + size.x, pos.y + size.y},
                       color.to_imgui(), rounding, 0, thickness);
}
void add_rect_filled(glm::vec2 pos, glm::vec2 size, Color color, float rounding) {
    if (!draw_list) return;
    draw_list->AddRectFilled({pos.x, pos.y}, {pos.x + size.x, pos.y + size.y},
                             color.to_imgui(), rounding);
}
void add_rect_gradient(glm::vec2 pos, glm::vec2 size, GradientType type,
                       Color c1, Color c2, float /*rounding*/) {
    if (!draw_list) return;
    const ImVec2 a{pos.x, pos.y};
    const ImVec2 b{pos.x + size.x, pos.y + size.y};
    if (type == GradientType::Vertical)
        draw_list->AddRectFilledMultiColor(a, b, c1.to_imgui(), c1.to_imgui(),
                                           c2.to_imgui(), c2.to_imgui());
    else
        draw_list->AddRectFilledMultiColor(a, b, c1.to_imgui(), c2.to_imgui(),
                                           c2.to_imgui(), c1.to_imgui());
}
void add_shadow_rect(glm::vec2 pos, glm::vec2 size, Color color,
                     float blur_amount, float offset, float rounding) {
    if (!draw_list) return;
    const int steps = 6;
    const float a_step = (color.scalable_alpha() / float(steps)) * 0.6f;
    for (int i = 0; i < steps; ++i) {
        const float t = (float(i) + 1.0f) / float(steps);
        const float spread = blur_amount * t;
        Color c{color.r, color.g, color.b, int(a_step * 255.0f)};
        draw_list->AddRectFilled(
            {pos.x - spread, pos.y - spread + offset * 0.5f},
            {pos.x + size.x + spread, pos.y + size.y + spread + offset * 0.5f},
            c.to_imgui(), rounding + spread);
    }
}

void add_circle(glm::vec2 c, float r, Color color, float thickness, int segments) {
    if (!draw_list) return;
    draw_list->AddCircle({c.x, c.y}, r, color.to_imgui(), segments, thickness);
}
void add_circle_filled(glm::vec2 c, float r, Color color, int segments) {
    if (!draw_list) return;
    draw_list->AddCircleFilled({c.x, c.y}, r, color.to_imgui(), segments);
}
void add_circle_shadow(glm::vec2 c, float r, Color color, float blur_amount) {
    if (!draw_list) return;
    const int steps = 6;
    const float a_step = (color.scalable_alpha() / float(steps)) * 0.6f;
    for (int i = 0; i < steps; ++i) {
        const float t = (float(i) + 1.0f) / float(steps);
        const float radius = r + blur_amount * t;
        Color cc{color.r, color.g, color.b, int(a_step * 255.0f)};
        draw_list->AddCircleFilled({c.x, c.y}, radius, cc.to_imgui());
    }
}

void add_line(glm::vec2 p1, glm::vec2 p2, Color color, float thickness) {
    if (!draw_list) return;
    draw_list->AddLine({p1.x, p1.y}, {p2.x, p2.y}, color.to_imgui(), thickness);
}
void add_polyline(const std::vector<glm::vec2>& points, Color color,
                  float thickness, bool closed) {
    if (!draw_list || points.size() < 2) return;
    std::vector<ImVec2> pts;
    pts.reserve(points.size());
    for (auto& p : points) pts.push_back({p.x, p.y});
    draw_list->AddPolyline(pts.data(), int(pts.size()), color.to_imgui(),
                           closed ? ImDrawFlags_Closed : ImDrawFlags_None,
                           thickness);
}
void add_shadow_poly(const std::vector<glm::vec2>& points, Color color, float blur_amount) {
    if (!draw_list || points.size() < 2) return;
    const int steps = 4;
    const float a_step = (color.scalable_alpha() / float(steps)) * 0.6f;
    for (int i = 0; i < steps; ++i) {
        const float t = (float(i) + 1.0f) / float(steps);
        Color c{color.r, color.g, color.b, int(a_step * 255.0f)};
        add_polyline(points, c, blur_amount * t);
    }
}
void add_triangle_filled(glm::vec2 a, glm::vec2 b, glm::vec2 c, Color color) {
    if (!draw_list) return;
    draw_list->AddTriangleFilled({a.x, a.y}, {b.x, b.y}, {c.x, c.y}, color.to_imgui());
}

void add_image(ID3D11ShaderResourceView* srv, glm::vec2 pos, glm::vec2 size,
               float rotation, Color tint, float /*rounding*/) {
    if (!draw_list || !srv) return;
    if (rotation == 0.0f) {
        draw_list->AddImage(reinterpret_cast<ImTextureID>(srv),
                            {pos.x, pos.y}, {pos.x + size.x, pos.y + size.y},
                            {0, 0}, {1, 1}, tint.to_imgui());
    } else {
        const float cx = pos.x + size.x * 0.5f;
        const float cy = pos.y + size.y * 0.5f;
        const float cs = std::cos(rotation);
        const float sn = std::sin(rotation);
        auto rot = [&](float dx, float dy) -> ImVec2 {
            return { cx + dx * cs - dy * sn, cy + dx * sn + dy * cs };
        };
        const float hx = size.x * 0.5f, hy = size.y * 0.5f;
        const ImVec2 p1 = rot(-hx, -hy);
        const ImVec2 p2 = rot( hx, -hy);
        const ImVec2 p3 = rot( hx,  hy);
        const ImVec2 p4 = rot(-hx,  hy);
        draw_list->AddImageQuad(reinterpret_cast<ImTextureID>(srv),
                                p1, p2, p3, p4,
                                {0, 0}, {1, 0}, {1, 1}, {0, 1},
                                tint.to_imgui());
    }
}

void modify_alpha(int start_vtx, int end_vtx, float f) {
    if (!draw_list) return;
    if (start_vtx < 0 || end_vtx <= start_vtx) return;
    if (end_vtx > draw_list->VtxBuffer.Size) end_vtx = draw_list->VtxBuffer.Size;
    for (int i = start_vtx; i < end_vtx; ++i) {
        ImDrawVert& v = draw_list->VtxBuffer.Data[i];
        const unsigned int a = (v.col >> 24) & 0xFFu;
        const unsigned int na = unsigned(float(a) * f) & 0xFFu;
        v.col = (v.col & 0x00FFFFFFu) | (na << 24);
    }
}

// multiplies vertex RGB(A) by tint/255. white tint = no-op.
void modulate_color(int start_vtx, int end_vtx, Color tint) {
    if (!draw_list) return;
    if (start_vtx < 0 || end_vtx <= start_vtx) return;
    if (end_vtx > draw_list->VtxBuffer.Size) end_vtx = draw_list->VtxBuffer.Size;
    const float tr = tint.r / 255.0f;
    const float tg = tint.g / 255.0f;
    const float tb = tint.b / 255.0f;
    const float ta = tint.a / 255.0f;
    for (int i = start_vtx; i < end_vtx; ++i) {
        ImDrawVert& v = draw_list->VtxBuffer.Data[i];
        const unsigned int r =  v.col        & 0xFFu;
        const unsigned int g = (v.col >>  8) & 0xFFu;
        const unsigned int b = (v.col >> 16) & 0xFFu;
        const unsigned int a = (v.col >> 24) & 0xFFu;
        const unsigned int nr = unsigned(float(r) * tr) & 0xFFu;
        const unsigned int ng = unsigned(float(g) * tg) & 0xFFu;
        const unsigned int nb = unsigned(float(b) * tb) & 0xFFu;
        const unsigned int na = unsigned(float(a) * ta) & 0xFFu;
        v.col = nr | (ng << 8) | (nb << 16) | (na << 24);
    }
}

void gradient_items(glm::vec2 pos, glm::vec2 size, Color c1, Color c2,
                    const std::function<void()>& draw_func, float /*frequency*/) {
    const int v_start = draw_list ? draw_list->VtxBuffer.Size : 0;
    if (draw_func) draw_func();
    if (!draw_list) return;
    const int v_end = draw_list->VtxBuffer.Size;
    if (v_end <= v_start) return;
    const ImU32 col1 = c1.to_imgui();
    const ImU32 col2 = c2.to_imgui();
    (void)col1; (void)col2;
    for (int i = v_start; i < v_end; ++i) {
        ImDrawVert& v = draw_list->VtxBuffer.Data[i];
        const float t = (v.pos.y - pos.y) / (size.y > 0.0f ? size.y : 1.0f);
        const float tt = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
        const int r = int(c1.r + (c2.r - c1.r) * tt);
        const int g = int(c1.g + (c2.g - c1.g) * tt);
        const int b = int(c1.b + (c2.b - c1.b) * tt);
        v.col = (v.col & 0xFF000000u) | (unsigned(b) << 16) | (unsigned(g) << 8) | unsigned(r);
    }
}

}
