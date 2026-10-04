#pragma once

#include <vector>

enum e_text_flags : int
{
    TEXT_FLAG_NONE = (1 << 0),
    TEXT_FLAG_DROPSHADOW = (1 << 1),
    TEXT_FLAG_OUTLINE = (1 << 2),
    TEXT_FLAG_MAX = (1 << 3)
};

enum e_gradient_type : int
{
    horizontal,
    vertical
};

namespace Render
{
    namespace Fonts
    {
        inline bool didInit = false;
        inline ImFont *montserrat14px = nullptr;
        inline ImFont *montserrat16px = nullptr;
        inline ImFont *montserratBold16px = nullptr;
        inline ImFont *fontAwesome14px = nullptr;
        inline ImFont *arial13px = nullptr;
        inline ImFont *pixelmix10px = nullptr;

        void Init();

    } // namespace Fonts

    void StartDrawing(ImDrawList *draw_list);

    void SetDrawList(ImDrawList *draw_list);

    ImFont *GetFont(const int &font_index);

    void RevertDrawList();

    void EndDrawing();

    int GetCurrentLayer();

    void SetLayer(int layer);

    void AddRectFilled(const Vector2 &position, const Vector2 &size, const Color &input_color, const float &rounding = 0.0f, int flags = 0);
    void AddRect(const Vector2 &position, const Vector2 &size, const Color &input_color, const float &rounding = 0.0f, const float &thickness = 1.0f,
                  int flags = 0);

    void AddShadowRectFast(const Vector2 &position, const Vector2 &size, const Color &input_color, const float &shadow_thick = 15.0f,
                               const float &rounding = 0.0f, int flags = 0);

    void AddShadowRect(const Vector2 &position, const Vector2 &size, const Color &input_color, const float &shadow_thick = 15.0f,
                         const float &rounding = 0.0f, int flags = 0);

    std::vector<Vector2> GetPointsForBox(const Vector2 &position, const Vector2 &size, float rounding, const int &segments_per_corner = 8);

    void AddShadowPoly(const std::vector<Vector2> &points, const Color &input_color, const float &shadow_thick = 15.0f, bool cut_background = true);

    void AddPoly(const std::vector<Vector2> &points, const Color &input_color);

    void AddPolyLine(const std::vector<Vector2> &points, const Color &input_color, const float &thickness = 1.0f, const bool &closed = false);

    void PushClipRect(const Vector2 &position, const Vector2 &size, bool intersect = false);

    void PopClipRect();

    void AddDropshadow(const Vector2 &position, const float &width, const Color &input_color, const float &intensity);

    Vector2 CalcTextSize(const std::string &text, ImFont *font = Fonts::montserrat16px);

    void AddText(const std::string &text, const Vector2 &position, const Color &input_color, const e_text_flags &flags,
                  const Vector2 &align = Vector2{0.0f, 0.0f}, ImFont *font = Fonts::montserrat16px);

    void AddLine(const Vector2 &from, const Vector2 &to, const Color &input_color, const float &thickness = 1.0f);

    void RenderCheckmark(Vector2 position, const Color &input_color, float size, const float &progression);

    void GradientItems(const Vector2 &position, const Vector2 &size, const Color &input_first_color, const Color &input_second_color,
                        const std::function<void()> &fn, const float &rotation = 0.0f);

    void AddRectGradient(const Vector2 &position, const Vector2 &size, e_gradient_type type, const Color &input_color1, const Color &input_color2,
                           float rounding = 0.f, int flags = 0);

    void AddCircle(const Vector2 &center, const float &radius, const Color &input_color, const float &thickness = 1.0f);

    void AddCircleFilled(const Vector2 &center, const float &radius, const Color &input_color);

    void AddShadowCircle(const Vector2 &center, const float &radius, const Color &input_color, const float &shadow_thick = 15.0f);

    void AddGradientConcavePoly(const std::vector<Vector2> &points, const Color &input_color_in, const Color &input_color_out,
                                   const float &centroid_shrink = 0.3f, bool force_centroid = false, Vector2 forced_centroid = Vector2());

    void AddGradientConvexPoly(const std::vector<Vector2> &points, const Color &input_color_in, const Color &input_color_out,
                                  bool force_centroid = false, Vector2 forced_centroid = Vector2());

    void AddShadowLine(const Vector2 &from, const Vector2 &to, const Color &input_color, float thickness = 15.0f);

    bool LoadTextureFromMemory(const void *data, size_t data_size, ID3D11ShaderResourceView **out_shader, int *out_width, int *out_height,
                                  Color *out_color = nullptr);
    bool LoadTextureFromFile(const std::string &path, ID3D11ShaderResourceView **out_shader, int *out_width, int *out_height,
                                Color *out_color = nullptr);

    void AddImage(ID3D11ShaderResourceView *texture, const Vector2 &pos, const Vector2 &size, const Color &clr = Color::White());

    void AddProgressCircle(const Vector2 &center, const float &radius, const float &thickness, float progress, const Color &input_color);

    void AddFrostedGlass(const Vector2 &position, const Vector2 &size, const Color &input_color, float strength = 1.0f, float rounding = 0.0f);

    inline float alpha = 1.0f;
    inline ImDrawList *drawList = nullptr;
    inline std::vector<ImDrawList*> drawListStack;
    inline ImDrawList *oldDrawList = nullptr;
} // namespace Render
