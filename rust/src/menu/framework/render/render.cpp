#include <Cheat.hpp>

#include "stb_image.hpp"

#include "assets/font_awesome_data.hpp"
#include "assets/montserrat.hpp"
#include "assets/montserrat_semibold.hpp"
#include "assets/pixelmix.hpp"

ImFont *create_font_from_data(const void *data, const int &data_size, const float &size, const int &font_loader_flags = 0,
                              const ImWchar *glyph_ranges = nullptr, bool merge = false)
{
    ImFontConfig font_config;
    font_config.FontData = std::malloc(data_size);
    font_config.FontDataSize = data_size;
    font_config.SizePixels = size;
    font_config.GlyphRanges = glyph_ranges;

    font_config.FontLoaderFlags = font_loader_flags;

    font_config.MergeMode = merge;
    font_config.PixelSnapH = true;

    if (font_config.FontData)
    {
        std::memcpy(font_config.FontData, data, data_size);
    }

    return ImGui::GetIO().Fonts->AddFont(&font_config);
}

void Render::Fonts::Init()
{
    if (didInit)
        return;

    static ImVector<ImWchar> ranges;
    if (ranges.empty()) {
        ImFontGlyphRangesBuilder builder;
        builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesDefault());  // Adds Degree, etc.
        builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesCyrillic()); // Adds Cyrillic
        builder.BuildRanges(&ranges);
    }

    static const ImWchar emoji_ranges[] = {0x1, 0x1FFFF, 0};

    ImFontConfig mergeConfig;
    mergeConfig.MergeMode = true;
    mergeConfig.PixelSnapH = true;

    ImFontConfig mergeEmojiConfig;
    mergeEmojiConfig.MergeMode = true;
    mergeEmojiConfig.OversampleH = 1;
    mergeEmojiConfig.OversampleV = 1;
    mergeEmojiConfig.GlyphOffset.y = 1.0f;

    Fonts::montserrat14px = create_font_from_data(montserrat_semibold_data, sizeof(montserrat_semibold_data), 14.0f, 0, ranges.Data);
    if (!Fonts::montserrat14px)
        Fonts::montserrat14px = ImGui::GetIO().Fonts->AddFontDefault();
    Fonts::montserrat16px = create_font_from_data(montserrat_semibold_data, sizeof(montserrat_semibold_data), 16.0f, 0, ranges.Data);
    if (!Fonts::montserrat16px)
        Fonts::montserrat16px = ImGui::GetIO().Fonts->AddFontDefault();
    Fonts::montserratBold16px = create_font_from_data(montserrat_bold_data, sizeof(montserrat_bold_data), 16.0f, 0, ranges.Data);
    if (!Fonts::montserratBold16px)
        Fonts::montserratBold16px = ImGui::GetIO().Fonts->AddFontDefault();

    Fonts::arial13px =
        ImGui::GetIO().Fonts->AddFontFromFileTTF(X("C:\\Windows\\Fonts\\verdana.ttf"), 13.0f, nullptr, ImGui::GetIO().Fonts->GetGlyphRangesDefault());
    if (!Fonts::arial13px)
        Fonts::arial13px = ImGui::GetIO().Fonts->AddFontDefault();

    ImGui::GetIO().Fonts->AddFontFromFileTTF(X("C:\\Windows\\Fonts\\malgun.ttf"), 14.0f, &mergeConfig,
                                             ImGui::GetIO().Fonts->GetGlyphRangesJapanese());
    ImGui::GetIO().Fonts->AddFontFromFileTTF(X("C:\\Windows\\Fonts\\seguiemj.ttf"), 11.0f, &mergeEmojiConfig, emoji_ranges);

    static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    Fonts::fontAwesome14px = create_font_from_data(font_awesome_data, sizeof(font_awesome_data), 14.0f, 0, icon_ranges);
    if (!Fonts::fontAwesome14px)
        Fonts::fontAwesome14px = ImGui::GetIO().Fonts->AddFontDefault();

    Fonts::pixelmix10px = create_font_from_data(pixelmix_ttf, sizeof(pixelmix_ttf), 10.0f);
    if (!Fonts::pixelmix10px)
        Fonts::pixelmix10px = ImGui::GetIO().Fonts->AddFontDefault();

    didInit = true;
}

void Render::StartDrawing(ImDrawList *draw_list)
{
    alpha = 1.0f;
    drawList = draw_list;

    ImGui::GetBackgroundDrawList()->ChannelsSplit(11);
    ImGui::GetForegroundDrawList()->ChannelsSplit(11);
}

void Render::SetDrawList(ImDrawList *draw_list)
{
    drawListStack.push_back(drawList);
    oldDrawList = drawList;
    drawList = draw_list;
}

ImFont *Render::GetFont(const int &font_index)
{
    switch (font_index)
    {
    case 0:
        return Fonts::montserrat16px;
        break;
    case 1:
        return Fonts::pixelmix10px;
        break;
    case 2:
        return Fonts::arial13px;
        break;
    }

    return Fonts::montserrat16px;
}

void Render::RevertDrawList()
{
    if (drawListStack.empty())
        return;
    drawList = drawListStack.back();
    drawListStack.pop_back();
    oldDrawList = drawListStack.empty() ? nullptr : drawListStack.back();
}

void Render::EndDrawing()
{
    ImGui::GetBackgroundDrawList()->ChannelsMerge();
    ImGui::GetForegroundDrawList()->ChannelsMerge();
}

int Render::GetCurrentLayer()
{
    return drawList->_Splitter._Current;
}

void Render::SetLayer(int layer)
{
    IM_ASSERT(layer <= 10);

    drawList->ChannelsSetCurrent(std::clamp(layer, 0, 10));
}

void Render::AddRectFilled(const Vector2 &position, const Vector2 &size, const Color &input_color, const float &rounding, int flags)
{
    if (!drawList)
        return;

    Color col = input_color.ScaleAlpha(alpha);
    drawList->AddRectFilled(position.Floored(), (position + size).Floored(), col, rounding, flags);
}

void Render::AddRect(const Vector2 &position, const Vector2 &size, const Color &input_color, const float &rounding, const float &thickness,
                      int flags)
{
    if (!drawList)
        return;

    Color col = input_color.ScaleAlpha(alpha);
    drawList->AddRect(position.Floored(), (position + size).Floored(), col, rounding, flags, thickness);
}

void Render::AddShadowRectFast(const Vector2 &position, const Vector2 &size, const Color &input_color, const float &shadow_thick,
                                  const float &rounding, int flags)
{
    if (!drawList)
        return;

    Color col = input_color.ScaleAlpha(alpha);
    drawList->AddShadowRect(position.Floored(), (position + size).Floored(), col, shadow_thick, {0.0f, 0.0f}, flags, rounding);
}

void Render::AddShadowRect(const Vector2 &position, const Vector2 &size, const Color &input_color, const float &shadow_thick, const float &rounding,
                             int flags)
{
    if (!drawList)
        return;

    Color col = input_color.ScaleAlpha(alpha);
    AddShadowPoly(GetPointsForBox(position.Floored(), size.Floored(), rounding), col, shadow_thick);
}

std::vector<Vector2> Render::GetPointsForBox(const Vector2 &position, const Vector2 &size, float rounding, const int &segments_per_corner)
{
    const float max_rounding = std::min(size.x * 0.25f, size.y * 0.25f);
    rounding = std::min(rounding, max_rounding);

    if (rounding <= 0.0f || segments_per_corner < 1)
    {
        return {{position.x, position.y},
                {position.x + size.x, position.y},
                {position.x + size.x, position.y + size.y},
                {position.x, position.y + size.y}};
    }

    const float x = position.x;
    const float y = position.y;
    const float w = size.x;
    const float h = size.y;

    const int total_points = (segments_per_corner + 1) * 4;
    std::vector<Vector2> points;
    points.reserve(total_points);

    const Vector2 centers[4] = {
        {x + w - rounding, y + rounding},     // top right
        {x + w - rounding, y + h - rounding}, // bottom right
        {x + rounding, y + h - rounding},     // bottom left
        {x + rounding, y + rounding}          // top left
    };

    constexpr float pi = std::numbers::pi_v<float>;
    constexpr float angle_starts[4] = {1.5f * pi, 0.0f, 0.5f * pi, pi};
    constexpr float angle_ends[4] = {2.0f * pi, 0.5f * pi, pi, 1.5f * pi};

    for (int c = 0; c < 4; c++)
    {
        const float a0 = angle_starts[c];
        const float a1 = angle_ends[c];
        const Vector2 center = centers[c];

        for (int i = 0; i <= segments_per_corner; i++)
        {
            const float t = static_cast<float>(i) / static_cast<float>(segments_per_corner);
            const float a = a0 + t * (a1 - a0);
            points.emplace_back(center.x + std::cos(a) * rounding, center.y + std::sin(a) * rounding);
        }
    }

    return points;
}

void Render::AddShadowPoly(const std::vector<Vector2> &points, const Color &input_color, const float &shadow_thick, bool cut_background)
{
    if (!drawList)
        return;

    Color col = input_color.ScaleAlpha(alpha);
    drawList->AddShadowConvexPoly((const ImVec2 *)points.data(), static_cast<int>(points.size()), col, shadow_thick, {0.0f, 0.0f},
                                     cut_background ? ImDrawFlags_ShadowCutOutShapeBackground : 0);
}

void Render::AddPoly(const std::vector<Vector2> &points, const Color &input_color)
{
    if (!drawList)
        return;

    Color col = input_color.ScaleAlpha(alpha);
    drawList->AddConvexPolyFilled((const ImVec2 *)points.data(), static_cast<int>(points.size()), col);
}

void Render::AddPolyLine(const std::vector<Vector2> &points, const Color &input_color, const float &thickness, const bool &closed)
{
    if (!drawList)
        return;

    Color col = input_color.ScaleAlpha(alpha);
    drawList->AddPolyline((ImVec2 *)points.data(), points.size(), col, closed ? ImDrawFlags_Closed : 0, thickness);
}

void Render::PushClipRect(const Vector2 &position, const Vector2 &size, bool intersect)
{
    if (!drawList)
        return;

    drawList->PushClipRect(position.Floored(), (position + size).Floored(), intersect);
}

void Render::PopClipRect()
{
    if (!drawList)
        return;

    drawList->PopClipRect();
}

void Render::AddDropshadow(const Vector2 &position, const float &width, const Color &input_color, const float &intensity)
{
    if (!drawList)
        return;

    Color col = input_color.ScaleAlpha(alpha);
    Render::PushClipRect(position, Vector2{width, intensity});
    Render::AddShadowRectFast(position - Vector2{0.0f, 1.0f}, Vector2{width, 1.0f}, col, intensity, 0.0f);
    Render::PopClipRect();
}

Vector2 Render::CalcTextSize(const std::string &text, ImFont *font)
{
    if (!font)
        return Vector2(0.0f, 0.0f);
    return font->CalcTextSizeA(font->LegacySize, FLT_MAX, 0.0f, text.data());
}

void Render::AddText(const std::string &text, const Vector2 &in_position, const Color &input_color, const e_text_flags &flags, const Vector2 &align,
                      ImFont *font)
{
    if (!font)
        return;

    Vector2 position = in_position;
    const float font_size = font->LegacySize;
    const Vector2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text.c_str());

    position -= Vector2(text_size.x * align.x, text_size.y * align.y);
    position.x = std::floorf(position.x);
    position.y = std::floorf(position.y);

    Color col = input_color.ScaleAlpha(alpha);

    if (flags & TEXT_FLAG_DROPSHADOW)
        drawList->AddText(font, font_size, position + Vector2{1.0f, 1.0f}, Color::Black().ScaleAlpha(0.5f * col.ScalableAlpha()), text.c_str());

    if (flags & TEXT_FLAG_OUTLINE)
    {
        drawList->AddText(font, font_size, position - Vector2(1.0f, 0.0f), Color::Black().ScaleAlpha(0.5f * col.ScalableAlpha()), text.c_str());
        drawList->AddText(font, font_size, position - Vector2(0.0f, 1.0f), Color::Black().ScaleAlpha(0.5f * col.ScalableAlpha()), text.c_str());
        drawList->AddText(font, font_size, position + Vector2(1.0f, 0.0f), Color::Black().ScaleAlpha(0.5f * col.ScalableAlpha()), text.c_str());
        drawList->AddText(font, font_size, position + Vector2(0.0f, 1.0f), Color::Black().ScaleAlpha(0.5f * col.ScalableAlpha()), text.c_str());
    }

    drawList->AddText(font, font_size, position, col, text.data());
}

void Render::AddLine(const Vector2 &from, const Vector2 &to, const Color &input_color, const float &thickness)
{
    Color col = input_color.ScaleAlpha(alpha);
    drawList->AddLine(from, to, col, thickness);
}

void Render::RenderCheckmark(Vector2 position, const Color &input_color, float size, const float &progression)
{
    const float thickness = std::max(size / 5.0f, 1.0f);
    const float cap_radius = thickness * 0.5f;
    Color col = input_color.ScaleAlpha(alpha);

    size -= thickness * 0.5f;
    position += Vector2(thickness * 0.25f, thickness * 0.25f);

    const float third = size / 3.0f;

    const Vector2 p0 = Vector2(position.x + third - third, position.y + size - third * 0.5f - third);         // start
    const Vector2 p1 = Vector2(position.x + third, position.y + size - third * 0.5f);                         // middle
    const Vector2 p2 = Vector2(position.x + third + third * 2, position.y + size - third * 0.5f - third * 2); // end

    if (progression <= 0.0f)
        return;

    drawList->PathClear();

    const float total_len = ImLength(p0 - p1, 0.0f) + ImLength(p1 - p2, 0.0f);
    const float draw_len = total_len * progression;

    const float first_seg_len = ImLength(p0 - p1, 0.0f);
    const float second_seg_len = ImLength(p1 - p2, 0.0f);

    Vector2 current_end;

    if (draw_len <= first_seg_len)
    {
        float t = draw_len / first_seg_len;
        current_end = ImLerp(p0, p1, t);
        drawList->PathLineTo(p0);
        drawList->PathLineTo(current_end);
    }
    else
    {
        drawList->PathLineTo(p0);
        drawList->PathLineTo(p1);

        float t = (draw_len - first_seg_len) / second_seg_len;
        current_end = ImLerp(p1, p2, std::clamp(t, 0.0f, 1.0f));
        drawList->PathLineTo(current_end);
    }

    drawList->PathStroke(col, false, thickness);

    if (draw_len <= first_seg_len)
    {
        float t = draw_len / first_seg_len;
        current_end = ImLerp(p0, p1, t);
        AddLine(p0, current_end, col, 1.0f);
    }
    else
    {
        AddLine(p0, p1, col, 1.0f);

        float t = (draw_len - first_seg_len) / second_seg_len;
        current_end = ImLerp(p1, p2, std::clamp(t, 0.0f, 1.0f));
        AddLine(p1, current_end, col, 1.0f);
    }

    if (progression > 0.01f)
    {
        drawList->AddCircleFilled(p0, cap_radius, col, 12);
    }
    if (progression >= 1.0f)
    {
        drawList->AddCircleFilled(p2, cap_radius, col, 12);
    }
    else
    {
        drawList->AddCircleFilled(current_end, cap_radius, col, 12);
    }
}

void Render::GradientItems(const Vector2 &position, const Vector2 &size, const Color &input_first_color, const Color &input_second_color,
                            const std::function<void()> &fn, const float &rotation)
{
    const int vtx_idx_0 = drawList->VtxBuffer.Size;

    fn();

    const int vtx_idx_1 = drawList->VtxBuffer.Size;

    Color first_color = input_first_color.ScaleAlpha(alpha);
    Color second_color = input_second_color.ScaleAlpha(alpha);

    const Vector2 bounds_pos = position.Floored();
    const Vector2 bounds_size = size.Floored();

    auto shade_verts_linear_color_gradient = [&](const int &vert_start_idx, const int &vert_end_idx, const Vector2 &pos, const Vector2 &size,
                                                 const Color &left_color, const Color &right_color)
    {
        auto clamp_vec = [](const Vector2 &to_clamp, float min, float max)
        {
            return Vector2((to_clamp.x < min)   ? min
                           : (to_clamp.x > max) ? max
                                                : to_clamp.x,
                           (to_clamp.y < min)   ? min
                           : (to_clamp.y > max) ? max
                                                : to_clamp.y);
        };

        const auto vert_start = drawList->VtxBuffer.Data + vert_start_idx;
        const auto vert_end = drawList->VtxBuffer.Data + vert_end_idx;

        const float angle = rotation * std::numbers::pi_v<float> * 2.0f;
        const float cos_a = std::cosf(angle);
        const float sin_a = std::sinf(angle);

        for (auto vert = vert_start; vert < vert_end; vert++)
        {
            const float vert_alpha = static_cast<ImColor>(vert->col).Value.w;

            Vector2 uv = (vert->pos - pos) / size;
            uv = clamp_vec(uv, 0.0f, 1.0f);

            uv.x -= 0.50f;
            uv.y -= 0.50f;

            Vector2 rot_uv = {uv.x * cos_a - uv.y * sin_a, uv.x * sin_a + uv.y * cos_a};

            rot_uv.x += 0.50f;
            rot_uv.y += 0.50f;
            rot_uv = clamp_vec(rot_uv, 0.0f, 1.0f);

            ImColor result_color{left_color.Lerp(right_color, rot_uv.x)};
            result_color.Value.w *= vert_alpha;

            vert->col = result_color;
        }
    };

    shade_verts_linear_color_gradient(vtx_idx_0, vtx_idx_1, bounds_pos, bounds_size, first_color, second_color);
}

void Render::AddRectGradient(const Vector2 &position, const Vector2 &size, e_gradient_type type, const Color &input_color1,
                               const Color &input_color2, float rounding, int flags)
{

    Color color1 = input_color1.ScaleAlpha(alpha);
    Color color2 = input_color2.ScaleAlpha(alpha);

    auto shade_verts_linear_color_gradient =
        [&](int vert_start_idx, int vert_end_idx, Vector2 pos, Vector2 size, Color top_left, Color top_right, Color bottom_right, Color bottom_left)
    {
        auto clamp_vec = [](const Vector2 &to_clamp, float min, float max)
        {
            return Vector2((to_clamp.x < min)   ? min
                           : (to_clamp.x > max) ? max
                                                : to_clamp.x,
                           (to_clamp.y < min)   ? min
                           : (to_clamp.y > max) ? max
                                                : to_clamp.y);
        };

        auto vert_start = drawList->VtxBuffer.Data + vert_start_idx;
        auto vert_end = drawList->VtxBuffer.Data + vert_end_idx;
        Vector2 gradient_extent = Vector2(size.x, size.y);

        for (auto vert = vert_start; vert < vert_end; vert++)
        {
            ImColor imcol_col = vert->col;
            float vert_alpha = imcol_col.Value.w;

            Vector2 dt = clamp_vec((vert->pos - pos) / gradient_extent, 0.f, 1.f);

            Color left_color = top_left.Lerp(bottom_left, dt.y);
            Color right_color = top_right.Lerp(bottom_right, dt.y);

            ImColor result_color = ImColor{left_color.Lerp(right_color, dt.x)};

            result_color.Value.w *= vert_alpha;

            vert->col = result_color;
        }
    };

    if (rounding == 0.f)
    {
        switch (type)
        {
        case e_gradient_type::horizontal:
            drawList->AddRectFilledMultiColor(position.Floored(), (position + size).Floored(), color1, color2, color2, color1);
            break;
        case e_gradient_type::vertical:
            drawList->AddRectFilledMultiColor(position.Floored(), (position + size).Floored(), color1, color1, color2, color2);
            break;
        }

        return;
    }

    const int vtx_idx_0 = drawList->VtxBuffer.Size;
    drawList->AddRectFilled(position.Floored(), (position + size).Floored(), Color::White(), rounding, flags);
    const int vtx_idx_1 = drawList->VtxBuffer.Size;

    switch (type)
    {
    case e_gradient_type::horizontal:
        shade_verts_linear_color_gradient(vtx_idx_0, vtx_idx_1, position.Floored(), size.Floored(), color1, color2, color2, color1);
        break;
    case e_gradient_type::vertical:
        shade_verts_linear_color_gradient(vtx_idx_0, vtx_idx_1, position.Floored(), size.Floored(), color1, color1, color2, color2);
        break;
    }
}

void Render::AddCircle(const Vector2 &center, const float &radius, const Color &input_color, const float &thickness)
{
    if (!drawList)
        return;

    Color col = input_color.ScaleAlpha(alpha);
    drawList->AddCircle(center, radius, col, 0, thickness);
}

void Render::AddCircleFilled(const Vector2 &center, const float &radius, const Color &input_color)
{
    if (!drawList)
        return;

    Color col = input_color.ScaleAlpha(alpha);
    drawList->AddCircleFilled(center, radius, col);
}

void Render::AddShadowCircle(const Vector2 &center, const float &radius, const Color &input_color, const float &shadow_thick)
{
    if (!drawList)
        return;

    Color col = input_color.ScaleAlpha(alpha);
    drawList->AddShadowCircle(center, radius, col, shadow_thick, {0.0f, 0.0f});
}

void Render::AddGradientConcavePoly(const std::vector<Vector2> &points, const Color &input_color_in, const Color &input_color_out,
                                       const float &fcentroid_shrink, bool force_centroid, Vector2 forced_centroid)
{
    if (!drawList)
        return;

    if (points.size() < 3)
        return;

    int segment_count = static_cast<int>(points.size()) - 1;

    Vector2 centroid = Vector2(0.0f, 0.0f);
    if (force_centroid)
    {
        centroid = forced_centroid;
    }
    else
    {
        for (const auto &point : points)
        {
            centroid.x += point.x;
            centroid.y += point.y;
        }

        centroid.x /= points.size();
        centroid.y /= points.size();
    }

    std::vector<Vector2> outer(segment_count);
    std::vector<Vector2> inner(segment_count);

    for (int i = 0; i < segment_count; ++i)
    {
        outer[i] = points[i];
        inner[i] = ImLerp(points[i], centroid, fcentroid_shrink);
    }

    const int vtx_needed = segment_count * 2;
    const int idx_needed = segment_count * 6;
    drawList->PrimReserve(idx_needed, vtx_needed);

    const ImVec2 uv = drawList->_Data->TexUvWhitePixel;
    const ImU32 outer_col = ImColor{input_color_out.ScaleAlpha(alpha)};
    const ImU32 inner_col = ImColor{input_color_in.ScaleAlpha(alpha)};

    const unsigned int vtx_base = drawList->_VtxCurrentIdx;

    for (int i = 0; i < segment_count; ++i)
    {
        drawList->PrimWriteVtx(ImVec2(outer[i].x, outer[i].y), uv, outer_col);
        drawList->PrimWriteVtx(ImVec2(inner[i].x, inner[i].y), uv, inner_col);
    }

    for (int i = 0; i < segment_count; ++i)
    {
        const int ni = (i + 1) % segment_count;
        const unsigned int o_i = vtx_base + (i * 2);
        const unsigned int i_i = vtx_base + (i * 2) + 1;
        const unsigned int o_n = vtx_base + (ni * 2);
        const unsigned int i_n = vtx_base + (ni * 2) + 1;

        drawList->PrimWriteIdx(o_i);
        drawList->PrimWriteIdx(i_i);
        drawList->PrimWriteIdx(o_n);

        drawList->PrimWriteIdx(i_i);
        drawList->PrimWriteIdx(i_n);
        drawList->PrimWriteIdx(o_n);
    }
}

void Render::AddGradientConvexPoly(const std::vector<Vector2> &points, const Color &input_color_in, const Color &input_color_out,
                                      bool force_centroid, Vector2 forced_centroid)
{
    Color col_in = input_color_in.ScaleAlpha(alpha);
    Color col_out = input_color_out.ScaleAlpha(alpha);

    if (points.size() < 3 || ((col_in | col_out) & IM_COL32_A_MASK) == 0)
        return;

    const int num_points = points.size();

    Vector2 centroid = {0, 0};
    if (force_centroid)
    {
        centroid = forced_centroid;
    }
    else
    {
        for (const auto &point : points)
        {
            centroid.x += point.x;
            centroid.y += point.y;
        }
        centroid.x /= num_points;
        centroid.y /= num_points;
    }

    unsigned int vtx_base = drawList->_VtxCurrentIdx;
    drawList->PrimReserve(num_points * 3, num_points + 1);

    const ImVec2 uv = drawList->_Data->TexUvWhitePixel;

    drawList->PrimWriteVtx(centroid, uv, col_in);

    for (const auto &point : points)
    {
        drawList->PrimWriteVtx(point, uv, col_out);
    }

    for (int i = 0; i < num_points; i++)
    {
        drawList->PrimWriteIdx((ImDrawIdx)(vtx_base));
        drawList->PrimWriteIdx((ImDrawIdx)(vtx_base + 1 + i));
        drawList->PrimWriteIdx((ImDrawIdx)(vtx_base + 1 + ((i + 1) % num_points)));
    }
}

void Render::AddShadowLine(const Vector2 &from, const Vector2 &to, const Color &input_color, float thickness)
{
    float dx = to.x - from.x;
    float dy = to.y - from.y;

    float length = std::sqrtf(dx * dx + dy * dy);
    if (length == 0.0f)
        return;

    float nx = (-dy / length);
    float ny = (dx / length);

    drawList->PathLineTo({from.x + nx, from.y + ny});
    drawList->PathLineTo({to.x + nx, to.y + ny});
    drawList->PathLineTo({to.x - nx, to.y - ny});
    drawList->PathLineTo({from.x - nx, from.y - ny});

    auto col = input_color.ScaleAlpha(alpha);

    drawList->AddShadowConvexPoly(drawList->_Path.Data, drawList->_Path.Size, col, thickness, Vector2{0.0f, 0.0f}, 0);
    drawList->_Path.Size = 0;
}

bool Render::LoadTextureFromMemory(const void *data, size_t data_size, ID3D11ShaderResourceView **out_shader, int *out_width, int *out_height,
                                      Color *out_color)
{
    int image_width = 0;
    int image_height = 0;
    unsigned char *image_data = stbi_load_from_memory((const unsigned char *)data, (int)data_size, &image_width, &image_height, NULL, 4);

    if (image_data == NULL)
        return false;

    if (out_color)
    {
        uint64_t r_sum = 0, g_sum = 0, b_sum = 0, a_sum = 0;
        int total_pixels = image_width * image_height;

        for (int i = 0; i < total_pixels * 4; i += 4)
        {
            r_sum += image_data[i];
            g_sum += image_data[i + 1];
            b_sum += image_data[i + 2];
            a_sum += image_data[i + 3];
        }

        if (total_pixels > 0)
        {
            float r = (r_sum / (float)total_pixels) / 255.0f;
            float g = (g_sum / (float)total_pixels) / 255.0f;
            float b = (b_sum / (float)total_pixels) / 255.0f;

            float max_val = std::max({r, g, b});
            float min_val = std::min({r, g, b});
            float l = (max_val + min_val) / 2.0f;

            if (max_val != min_val)
            {
                float d = max_val - min_val;
                float s = l > 0.5f ? d / (2.0f - max_val - min_val) : d / (max_val + min_val);
                float new_s = std::clamp(s * 1.5f, 0.0f, 1.0f);
                auto hue_to_rgb = [](float p, float q, float t)
                {
                    if (t < 0.0f)
                        t += 1.0f;
                    if (t > 1.0f)
                        t -= 1.0f;
                    if (t < 1.0f / 6.0f)
                        return p + (q - p) * 6.0f * t;
                    if (t < 1.0f / 2.0f)
                        return q;
                    if (t < 2.0f / 3.0f)
                        return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
                    return p;
                };

                float h;
                if (max_val == r)
                    h = (g - b) / d + (g < b ? 6.0f : 0.0f);
                else if (max_val == g)
                    h = (b - r) / d + 2.0f;
                else
                    h = (r - g) / d + 4.0f;
                h /= 6.0f;

                float q = l < 0.5f ? l * (1.0f + new_s) : l + new_s - l * new_s;
                float p = 2.0f * l - q;

                r = hue_to_rgb(p, q, h + 1.0f / 3.0f);
                g = hue_to_rgb(p, q, h);
                b = hue_to_rgb(p, q, h - 1.0f / 3.0f);
            }
            *out_color = Color((int)(r * 255), (int)(g * 255), (int)(b * 255), (int)(a_sum / total_pixels));
        }
    }

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;

    ID3D11Texture2D *pTexture = NULL;
    if (SUCCEEDED(Overlay::g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture)))
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        Overlay::g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_shader);
        pTexture->Release();
    }

    *out_width = image_width;
    *out_height = image_height;

    stbi_image_free(image_data);
    return true;
}

bool Render::LoadTextureFromFile(const std::string &path, ID3D11ShaderResourceView **out_shader, int *out_width, int *out_height, Color *out_color)
{
    if (!Overlay::g_pd3dDevice)
        return false;

    int image_width = 0;
    int image_height = 0;
    unsigned char *image_data = stbi_load(path.c_str(), &image_width, &image_height, NULL, 4);

    if (image_data == NULL)
        return false;

    if (out_color)
    {
        uint64_t r_sum = 0, g_sum = 0, b_sum = 0, a_sum = 0;
        int total_pixels = image_width * image_height;

        for (int i = 0; i < total_pixels * 4; i += 4)
        {
            r_sum += image_data[i];
            g_sum += image_data[i + 1];
            b_sum += image_data[i + 2];
            a_sum += image_data[i + 3];
        }

        if (total_pixels > 0)
        {
            float r = (r_sum / (float)total_pixels) / 255.0f;
            float g = (g_sum / (float)total_pixels) / 255.0f;
            float b = (b_sum / (float)total_pixels) / 255.0f;

            float max_val = std::max({r, g, b});
            float min_val = std::min({r, g, b});
            float l = (max_val + min_val) / 2.0f;

            if (max_val != min_val)
            {
                float d = max_val - min_val;
                float s = l > 0.5f ? d / (2.0f - max_val - min_val) : d / (max_val + min_val);
                float new_s = std::clamp(s * 1.5f, 0.0f, 1.0f);
                auto hue_to_rgb = [](float p, float q, float t)
                {
                    if (t < 0.0f)
                        t += 1.0f;
                    if (t > 1.0f)
                        t -= 1.0f;
                    if (t < 1.0f / 6.0f)
                        return p + (q - p) * 6.0f * t;
                    if (t < 1.0f / 2.0f)
                        return q;
                    if (t < 2.0f / 3.0f)
                        return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
                    return p;
                };

                float h;
                if (max_val == r)
                    h = (g - b) / d + (g < b ? 6.0f : 0.0f);
                else if (max_val == g)
                    h = (b - r) / d + 2.0f;
                else
                    h = (r - g) / d + 4.0f;
                h /= 6.0f;

                float q = l < 0.5f ? l * (1.0f + new_s) : l + new_s - l * new_s;
                float p = 2.0f * l - q;

                r = hue_to_rgb(p, q, h + 1.0f / 3.0f);
                g = hue_to_rgb(p, q, h);
                b = hue_to_rgb(p, q, h - 1.0f / 3.0f);
            }
            *out_color = Color((int)(r * 255), (int)(g * 255), (int)(b * 255), (int)(a_sum / total_pixels));
        }
    }

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;

    ID3D11Texture2D *pTexture = NULL;
    if (SUCCEEDED(Overlay::g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture)))
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        Overlay::g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_shader);
        pTexture->Release();
    }

    *out_width = image_width;
    *out_height = image_height;

    stbi_image_free(image_data);
    return true;
}

void Render::AddImage(ID3D11ShaderResourceView *texture, const Vector2 &pos, const Vector2 &size, const Color &clr)
{
    if (!texture)
        return;

    drawList->AddImage((ImTextureID)texture, pos.Floored(), (pos + size).Floored(), {0, 0}, {1, 1}, clr.ScaleAlpha(alpha));
}

void Render::AddProgressCircle(const Vector2 &center, const float &radius, const float &thickness, float progress, const Color &input_color)
{
    progress = std::clamp(progress, 0.0f, 1.0f);

    const float start_angle = -std::numbers::pi_v<float> / 2.0f;
    const float end_angle = start_angle + progress * (2.0f * std::numbers::pi_v<float>);

    const int num_segments = 179;
    const int segments_to_draw = static_cast<int>(num_segments * progress);
    const float cap_radius = thickness * 0.5f;

    Color c = input_color.ScaleAlpha(alpha);

    if (progress <= 0.0f)
    {
        Vector2 start_pos(center.x + std::cosf(start_angle) * radius, center.y + std::sinf(start_angle) * radius);
        drawList->AddCircleFilled(start_pos, cap_radius, c, 12);
        return;
    }

    Vector2 start_pos(center.x + std::cosf(start_angle) * radius, center.y + std::sinf(start_angle) * radius);
    drawList->AddCircleFilled(start_pos, cap_radius, c, 12);

    Vector2 end_pos(center.x + std::cosf(end_angle) * radius, center.y + std::sinf(end_angle) * radius);
    float end_tangent = end_angle + std::numbers::pi_v<float> * 0.5f;
    drawList->PathClear();
    drawList->PathArcTo(ImVec2(end_pos.x, end_pos.y), cap_radius, end_tangent - std::numbers::pi_v<float> * 0.5f,
                           end_tangent + std::numbers::pi_v<float> * 0.5f, 24);
    drawList->PathFillConvex(c);

    drawList->PathClear();
    int seg_draw = std::max(1, segments_to_draw);
    for (int i = 0; i <= seg_draw; ++i)
    {
        float current_progress = static_cast<float>(i) / num_segments;
        float angle = start_angle + current_progress * (2.0f * std::numbers::pi_v<float>);
        drawList->PathLineTo(ImVec2(center.x + std::cosf(angle) * radius, center.y + std::sinf(angle) * radius));
    }
    drawList->PathStroke(c, 0, thickness);
}

void Render::AddFrostedGlass(const Vector2 &position, const Vector2 &size, const Color &input_color, float strength, float rounding)
{
    Color c = input_color.ScaleAlpha(alpha);
}