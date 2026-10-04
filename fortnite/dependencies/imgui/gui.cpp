#define IMGUI_DEFINE_MATH_OPERATORS

#include "gui.h"
#include "gui_font.h"
#include "gui_colors.h"
#include "gui_image.h"

#include "imgui_freetype.h"
#include "../oxorany/oxorany.h"

std::vector<TabCategory> categories;

std::string to_lower(const std::string& str) {
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return result;
}

void ui::initialize_images()
{
	D3DX11_IMAGE_LOAD_INFO info; ID3DX11ThreadPump* pump{ nullptr };
	if (images::background == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, wallpaper, sizeof(wallpaper), &info, pump, &images::background, 0);
}

void ui::initialize_fonts()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImFontConfig spacegrotesk_cfg;
	spacegrotesk_cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LoadColor;
	
	spacegrotesk_cfg.Density = ui::dpi_scale;
	spacegrotesk_cfg.OversampleH = 1;
	spacegrotesk_cfg.OversampleV = 1;

	font.montserrat_semibold[0] = io.Fonts->AddFontFromMemoryTTF(PoppinsMedium, sizeof(PoppinsMedium), 22, &spacegrotesk_cfg, io.Fonts->GetGlyphRangesCyrillic());

	font.spacegrotesk_medium[0] = io.Fonts->AddFontFromMemoryTTF(PoppinsMedium, sizeof(PoppinsMedium), 24.0f, &spacegrotesk_cfg, io.Fonts->GetGlyphRangesCyrillic());
	font.spacegrotesk_medium[1] = io.Fonts->AddFontFromMemoryTTF(PoppinsMedium, sizeof(PoppinsMedium), 16.0f, &spacegrotesk_cfg, io.Fonts->GetGlyphRangesCyrillic());
	font.spacegrotesk_medium[2] = io.Fonts->AddFontFromMemoryTTF(PoppinsMedium, sizeof(PoppinsMedium), 20.0f, &spacegrotesk_cfg, io.Fonts->GetGlyphRangesCyrillic());
	font.tab_icon = io.Fonts->AddFontFromMemoryTTF(tab_font, sizeof(tab_font), 16.0f, &spacegrotesk_cfg, io.Fonts->GetGlyphRangesCyrillic());
	font.widget_icon[0] = io.Fonts->AddFontFromMemoryTTF(tab_font, sizeof(tab_font), 12.0f, &spacegrotesk_cfg, io.Fonts->GetGlyphRangesCyrillic());
	font.widget_icon[1] = io.Fonts->AddFontFromMemoryTTF(tab_font, sizeof(tab_font), 16.0f, &spacegrotesk_cfg, io.Fonts->GetGlyphRangesCyrillic());
	font.notification_icon = io.Fonts->AddFontFromMemoryTTF(notification_font, sizeof(notification_font), 12.0f, &spacegrotesk_cfg, io.Fonts->GetGlyphRangesCyrillic());

	io.FontDefault = font.spacegrotesk_medium[1];
}

void ui::items::text_shadow(ImDrawList* draw_list, ImVec2 pos, ImColor text_color, const char* text, ImFont* font = nullptr, float font_size = 0.0f)
{
	ImVec2 shadow_offset = ImVec2(1.0f, 1.0f); 
	ImColor shadow_color = IM_COL32(0, 0, 0, text_color.Value.w * 255); 

	if (font == nullptr)
		font = ImGui::GetFont();

	if (font_size == 0.0f)
		font_size = ImGui::GetFontSize();

	draw_list->AddText(font, font_size, ImVec2(pos.x + shadow_offset.x, pos.y + shadow_offset.y), shadow_color, text);

	draw_list->AddText(font, font_size, pos, text_color, text);
}

struct Notification {
	std::string message;
	std::string icon;
	float alpha = 0.0f;
	float timer = 0.0f;
	bool fading_out = false;
	float duration = 3.0f;
	float target_y = 0.0f;
	float current_y = 0.0f;
	ImVec4 bg_color = ui::colors::background_dark;
	ImVec4 icon_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
};

std::vector<Notification> notifications;

void ui::add_notification(const std::string& icon, const std::string& msg, const ImVec4& icon_color) {
	notifications.push_back({ msg, icon, 0.0f, 0.0f, false, 3.0f, 0.0f, 0.0f,
							   ImVec4(0.12f, 0.12f, 0.12f, 1.0f), icon_color });
}

void ui::render_notification() {
	const float padding = 14.0f;
	const float right_padding = 10.0f;
	const ImVec2 viewport_pos = ImGui::GetMainViewport()->WorkPos;
	const ImVec2 viewport_size = ImGui::GetMainViewport()->Size;
	float total_offset = 0.0f;

	for (int i = 0; i < notifications.size(); ++i) {
		Notification& note = notifications[i];
		float delta = ImGui::GetIO().DeltaTime;
		note.timer += delta;

		if (!note.fading_out) {
			note.alpha = ImClamp(note.alpha + delta * 3.0f, 0.0f, 1.0f);
			if (note.timer >= note.duration)
				note.fading_out = true;
		}
		else {
			note.alpha = ImClamp(note.alpha - delta * 2.0f, 0.0f, 1.0f);
		}

		if (note.fading_out && note.alpha <= 0.0f) {
			notifications.erase(notifications.begin() + i);
			--i;
			continue;
		}

		ImGui::PushFont(font.notification_icon);
		ImVec2 icon_size = ImGui::CalcTextSize(note.icon.c_str());
		ImGui::PopFont();

		ImGui::PushFont(font.spacegrotesk_medium[1]);
		ImVec2 text_size = ImGui::CalcTextSize(note.message.c_str());
		ImGui::PopFont();

		ImVec2 box_size(icon_size.x + text_size.x + 30.0f + right_padding, max(icon_size.y, text_size.y) + 10.0f);

		float target_y = viewport_pos.y + padding + total_offset;
		if (note.target_y == 0.0f)
			note.target_y = target_y;

		note.target_y = target_y;
		note.current_y += (note.target_y - note.current_y) * delta * 10.0f;

		ImVec2 pos(viewport_pos.x + viewport_size.x - box_size.x - padding, note.current_y);

		ImDrawList* draw = ImGui::GetForegroundDrawList();
		draw->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + box_size.x, pos.y + box_size.y), ImColor(note.bg_color.x, note.bg_color.y, note.bg_color.z, note.alpha), 6.0f);
		draw->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + box_size.x, pos.y + box_size.y), ImGui::GetColorU32(ui::colors::outline), 6.0f);

		ImGui::PushFont(font.notification_icon);
		draw->AddText(
			ImVec2(pos.x + 10, pos.y + 7),
			ImColor(note.icon_color.x, note.icon_color.y, note.icon_color.z, note.alpha),
			note.icon.c_str()
		);
		ImGui::PopFont();

		float line_x = pos.x + 15 + icon_size.x + 5.0f;
		float line_y_start = pos.y + 5.0f;
		float line_y_end = pos.y + box_size.y - 5.0f;
		draw->AddLine(
			ImVec2(line_x - 1, line_y_start),
			ImVec2(line_x - 1, line_y_end),
			ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, note.alpha * 0.3f)),
			1.0f
		);

		ImGui::PushFont(font.spacegrotesk_medium[1]);
		draw->AddText(
			ImVec2(line_x + 8, pos.y + 5),
			ImColor(1.0f, 1.0f, 1.0f, note.alpha),
			note.message.c_str()
		);
		ImGui::PopFont();

		total_offset += box_size.y + 5.0f;
	}
}

static float left_cursor_y = 0.0f;
static float right_cursor_y = 0.0f;
static ImVec2 base_pos_left = ImVec2(0, 0);
static ImVec2 base_pos_right = ImVec2(0, 0);
static float spacing_x = 12.0f; 
static float spacing_y = 8.0f; 

bool instant_switch = true; 

float ui::get_tab_alpha(int index)
{
	float target = (index == selected_tab_index) ? 1.0f : 0.0f;

	if (instant_switch)
		return target;

	float speed = 6.0f;
	float delta = ImGui::GetIO().DeltaTime;

	float& value = tab_lerp_values[index];
	value = ImLerp(value, target, 1.0f - expf(-speed * delta));

	if (fabsf(value - target) < 0.001f)
		value = target;

	return value;
}

void ui::reset_positions(ImVec2 base_left, ImVec2 base_right, float space_x, float space_y)
{
	base_pos_left = base_left;
	base_pos_right = base_right;
	spacing_x = space_x;
	spacing_y = space_y;
	left_cursor_y = 0.0f;
	right_cursor_y = 0.0f;
}

bool ui::begin_child_left(const char* id, int height)
{
	ImGui::SetCursorPos(ImVec2(base_pos_left.x, base_pos_left.y + left_cursor_y));
	bool ret = ImGui::BeginChild(id, id, ImVec2(child_width, height), true), ImGuiWindowFlags_NoScrollbar;
	ImVec2 child_size = ImGui::GetWindowSize();
	float child_height = child_size.y;

	if (ret)
		left_cursor_y += child_height + spacing_y;

	return ret;
}

bool ui::begin_child_right(const char* id, int height)
{
	ImGui::SetCursorPos(ImVec2(base_pos_right.x, base_pos_right.y + right_cursor_y));
	bool ret = ImGui::BeginChild(id, id, ImVec2(child_width, height), true, ImGuiWindowFlags_NoScrollbar);
	ImVec2 child_size = ImGui::GetWindowSize();
	float child_height = child_size.y;

	if (ret)
		right_cursor_y += child_height + spacing_y;

	return ret;
}

void ui::render_tabs_content()
{
	ImVec2 pos = ImGui::GetWindowPos();
	ImVec2 window_size = ImGui::GetWindowSize();

	ImVec2 content_pos(pos.x - 9, pos.y + 21);
	ImVec2 content_size(ui::size.x + 18, 502);

	static float current_width = 320.0f;
	float target_width = has_scrollbar ? 312.0f : 320.0f;

	float lerp_speed = 10.0f;
	float delta = ImGui::GetIO().DeltaTime;

	current_width = ImLerp(current_width, target_width, ImClamp(delta * lerp_speed, 0.0f, 1.0f));

	child_width = current_width;

	ImGui::SetCursorScreenPos(content_pos);

	if (ImGui::BeginChild("", "main_content", content_size, false, ImGuiWindowFlags_NoBackground))
	{
		has_scrollbar = ImGui::GetCurrentWindow()->ScrollbarY;

		float spacing_x_val = 12.0f;
		float spacing_y_val = 40.0f;

		ImVec2 base_left(20.0f, 11.0f);
		ImVec2 base_right(20.0f + child_width + spacing_x_val, 11.0f);
		reset_positions(base_left, base_right, spacing_x_val, spacing_y_val);
		for (int i = 0; i < categories.size(); ++i)
		{
			const auto& tab = categories[i];
			float alpha = get_tab_alpha(i);

			if (alpha <= 0.001f)
				continue;

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			ui::tabs::aimbot(tab);
			ui::tabs::visuals(tab);
			ui::tabs::settings(tab);
			ImGui::PopStyleVar();
		}
	}

	ImGui::EndChild();
}

void ui::show_fps()
{
	const float top_height = 50.0f;
	const float left_padding = 12.0f;
	const ImVec2 extra_padding = ImVec2(10, 6);
	const ImVec2 pos = ImVec2(10, 4);

	const char* watermark_text = oxorany("emulated.vip");
	ImDrawList* draw_list = ImGui::GetForegroundDrawList();

	ImGui::PushFont(font.montserrat_semibold[0]);
	ImVec2 watermark_size = ImGui::CalcTextSize(watermark_text);
	ImGui::PopFont();

	ImGui::PushFont(font.spacegrotesk_medium[1]);
	std::string user_text = oxorany("fps: ") + std::to_string((int)ImGui::GetIO().Framerate);
	ImVec2 username_size = ImGui::CalcTextSize(user_text.c_str());
	ImGui::PopFont();

	float text_height = ImGui::GetFontSize();
	float centered_y = pos.y + (top_height - text_height) * 0.5f;
	ImVec2 start_pos = ImVec2(pos.x + left_padding, centered_y);

	float separator_width = 1.0f;
	float spacing = 8.0f;

	float total_width = watermark_size.x + spacing + separator_width + spacing + username_size.x;

	ImVec2 bg_min = ImVec2(start_pos.x - extra_padding.x, start_pos.y - extra_padding.y);
	ImVec2 bg_max = ImVec2(start_pos.x + total_width + extra_padding.x,
		start_pos.y + watermark_size.y + extra_padding.y);

	draw_list->AddRectFilled(bg_min, bg_max, ImGui::GetColorU32(ui::colors::background), 3.0f);
	draw_list->AddRect(bg_min, bg_max, ImGui::GetColorU32(ui::colors::outline), 3.0f);

	ImGui::PushFont(font.montserrat_semibold[0]);

	std::string text_str = watermark_text;
	size_t dot_pos = text_str.find('.');

	if (dot_pos != std::string::npos)
	{
		std::string first_part = text_str.substr(0, dot_pos);
		std::string second_part = text_str.substr(dot_pos);

		ui::items::text_shadow(draw_list, start_pos, ImColor(250, 250, 250), first_part.c_str());

		float first_width = ImGui::CalcTextSize(first_part.c_str()).x;
		ImVec2 second_pos = ImVec2(start_pos.x + first_width, start_pos.y);
		ui::items::text_shadow(draw_list, second_pos, ImGui::GetColorU32(ui::colors::main), second_part.c_str());
	}
	else
	{
		ui::items::text_shadow(draw_list, start_pos, ImColor(250, 250, 250), watermark_text);
	}
	ImGui::PopFont();

	ImVec4 white_transparent = ImVec4(1.f, 1.f, 1.f, 0.6f);
	ImU32 separator_color = ImGui::GetColorU32(white_transparent);

	float line_height = 10.0f;

	float offset_y = 1.0f;

	ImVec2 separator_start = ImVec2(
		start_pos.x + watermark_size.x + spacing,
		start_pos.y + (watermark_size.y - line_height) * 0.5f + offset_y
	);
	ImVec2 separator_end = ImVec2(
		separator_start.x,
		separator_start.y + line_height
	);

	draw_list->AddLine(separator_start, separator_end, separator_color, separator_width);

	ImGui::PushFont(font.spacegrotesk_medium[1]);
	ImVec2 user_pos = ImVec2(separator_start.x + separator_width + spacing, start_pos.y - 1);

	ImVec4 col_start = ui::colors::text;
	ImVec4 col_end = ui::colors::main;

	int length = (int)user_text.size();
	ImVec2 cursor_pos = user_pos;

	for (int i = 0; i < length; i++)
	{
		float t = (length > 1) ? (float)i / (length - 1) : 0.0f;

		ImVec4 col = ImVec4(
			col_start.x + (col_end.x - col_start.x) * t,
			col_start.y + (col_end.y - col_start.y) * t,
			col_start.z + (col_end.z - col_start.z) * t,
			1.0f);

		char letter[2] = { user_text[i], '\0' };

		ui::items::text_shadow(draw_list, cursor_pos + ImVec2(0, 5), ImColor(col), letter);

		ImVec2 letter_size = ImGui::CalcTextSize(letter);
		cursor_pos.x += letter_size.x;
	}

	ImGui::PopFont();
}

void ui::render_background()
{
	ImVec2 pos = ImGui::GetWindowPos();
	ImVec2 size = ImGui::GetWindowSize();
	ImVec2 center = ImVec2(size.x / 2.f, size.y / 2.f);
	float max_dist = sqrtf(center.x * center.x + center.y * center.y);

	ImGui::GetBackgroundDrawList()->AddRectFilled(pos, ImVec2(pos.x + ui::size.x, pos.y + ui::size.y), ImColor(ui::colors::background), ui::rounding, ImDrawFlags_RoundCornersAll);

	const float spacing = 20.0f;
	const float outer_radius = 4.0f;
	const float inner_radius = 3.f;

	ImVec4 base_color = ui::colors::main;
	ImVec4 background_color = ui::colors::background;

	float time = ImGui::GetTime();

	for (float y = 0; y < size.y; y += spacing)
	{
		for (float x = 0; x < size.x; x += spacing)
		{
			float dist = sqrtf((x - center.x) * (x - center.x) + (y - center.y) * (y - center.y));

			float base_alpha = 0.1f * (1.0f - dist / max_dist);
			base_alpha = ImClamp(base_alpha, 0.0f, 1.0f);

			float alpha = base_alpha * (0.5f + 0.5f * sinf(time * 2.0f + (x + y) * 0.05f));

			if (alpha > 0.005f)
			{
				ImU32 col_outer = ImGui::GetColorU32(ImVec4(base_color.x, base_color.y, base_color.z, alpha));
				ImU32 col_inner = ImGui::GetColorU32(ImVec4(background_color.x, background_color.y, background_color.z, alpha));

				ImVec2 circle_pos = ImVec2(pos.x + x + 1.0f, pos.y + y + 1.0f);
				float adjusted_outer_radius = outer_radius - 1.0f;
				float adjusted_inner_radius = inner_radius - 1.0f;

				ImGui::GetBackgroundDrawList()->AddCircleFilled(circle_pos, adjusted_outer_radius, col_outer);
				ImGui::GetBackgroundDrawList()->AddCircleFilled(circle_pos, adjusted_inner_radius, col_inner);
			}
		}
	}

	ImVec2 rect_top_pos = ImVec2(pos.x, pos.y);
	ImVec2 rect_top_end = ImVec2(rect_top_pos.x + ui::size.x, rect_top_pos.y + 32);

	ImGui::GetBackgroundDrawList()->AddRectFilled(rect_top_pos, rect_top_end, ImColor(ui::colors::background), ui::rounding, ImDrawFlags_RoundCornersTop);

	ImVec2 line_top_start(rect_top_pos.x + 1, rect_top_end.y - 1);
	ImVec2 line_top_end(rect_top_pos.x + ui::size.x - 1, rect_top_end.y - 1);

	ImGui::GetBackgroundDrawList()->AddLine(
		line_top_start,
		line_top_end,
		ImColor(ui::colors::outline),
		1.0f
	);

	ImVec2 rect_pos = ImVec2(pos.x, pos.y + size.y - 39);
	ImVec2 rect_end = ImVec2(rect_pos.x + ui::size.x, rect_pos.y + 39);

	ImGui::GetBackgroundDrawList()->AddRectFilled(rect_pos, rect_end, ImColor(ui::colors::background), ui::rounding, ImDrawFlags_RoundCornersBottom);

	ImVec2 line_start(rect_pos.x + 1, rect_pos.y);
	ImVec2 line_end(rect_pos.x + ui::size.x - 2, rect_pos.y);

	ImGui::GetBackgroundDrawList()->AddLine(line_start, line_end, ImColor(ui::colors::outline), 1.0f);
}

void ui::render_title()
{
	ImVec2 pos = ImGui::GetWindowPos();
	ImVec2 size = ImGui::GetWindowSize();

	ImGui::PushFont(font.montserrat_semibold[0]);

	const float padding_right = 13.0f;
	const float padding_bottom = 13.0f;

	const char* title = ImGui::GetCurrentWindow()->Name;
	std::string title_str = title;
	size_t dot_pos = title_str.find('.');

	float text_height = ImGui::GetFontSize();

	if (dot_pos != std::string::npos)
	{
		std::string first_part = title_str.substr(0, dot_pos);
		std::string second_part = title_str.substr(dot_pos);

		float first_part_width = ImGui::CalcTextSize(first_part.c_str()).x;
		float second_part_width = ImGui::CalcTextSize(second_part.c_str()).x;
		float total_width = first_part_width + second_part_width;

		float second_part_x = pos.x + size.x - second_part_width - padding_right;
		float first_part_x = second_part_x - first_part_width;
		float text_y = pos.y + size.y - text_height - padding_bottom;

		ui::items::text_shadow(ImGui::GetBackgroundDrawList(), ImVec2(first_part_x, text_y), ImColor(ui::colors::text), first_part.c_str());
		ui::items::text_shadow(ImGui::GetBackgroundDrawList(), ImVec2(second_part_x, text_y), ImGui::GetColorU32(ui::colors::main), second_part.c_str());
	}
	else
	{
		float text_width = ImGui::CalcTextSize(title).x;
		float text_x = pos.x + size.x - text_width - padding_right;
		float text_y = pos.y + size.y - text_height - padding_bottom;

		ui::items::text_shadow(ImGui::GetBackgroundDrawList(), ImVec2(text_x, text_y), ImColor(ui::colors::text), title);
	}

	ImGui::PopFont();
}

void ui::render_title_cheat(std::string game_name)
{
	ImVec2 pos = ImGui::GetWindowPos();
	ImVec2 size = ImGui::GetWindowSize();

	ImGui::PushFont(font.spacegrotesk_medium[1]);

	const float top_height = 32.0f;
	const float left_padding = 13.0f;

	float text_height = ImGui::GetFontSize();
	float centered_y = pos.y + (top_height - text_height) * 0.5f;

	float text_x = pos.x + left_padding;

	ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

	ImVec2 cursor_pos = ImVec2(text_x, centered_y - 1);

	ImVec4 col_start = ui::colors::text;
	ImVec4 col_end = ui::colors::main;

	int length = (int)game_name.size();

	for (int i = 0; i < length; i++)
	{
		float t = (length > 1) ? (float)i / (length - 1) : 0.0f; 

		ImVec4 col = ImVec4(
			col_start.x + (col_end.x - col_start.x) * t,
			col_start.y + (col_end.y - col_start.y) * t,
			col_start.z + (col_end.z - col_start.z) * t,
			1.0f);

		char letter[2] = { game_name[i], '\0' };

		ui::items::text_shadow(draw_list, cursor_pos, ImColor(col), letter);

		ImVec2 letter_size = ImGui::CalcTextSize(letter);
		cursor_pos.x += letter_size.x;
	}

	ImGui::PopFont();
}

void ui::render_build_date()
{
	ImVec2 pos = ImGui::GetWindowPos();
	ImVec2 size = ImGui::GetWindowSize();

	ImGui::PushFont(font.spacegrotesk_medium[1]);

	const float top_height = 32.0f;
	const float right_padding = 13.0f;  

	float text_height = ImGui::GetFontSize();
	float centered_y = pos.y + (top_height - text_height) * 0.5f;

	std::string build_date = std::string("build: ") + __DATE__;
	std::string build_date_str = to_lower(build_date);

	float text_width = ImGui::CalcTextSize(build_date_str.c_str()).x;

	float text_x = pos.x + size.x - right_padding - text_width;

	ImVec2 text_pos = ImVec2(text_x, centered_y - 1);

	ui::items::text_shadow(ImGui::GetBackgroundDrawList(), text_pos, ImColor(ui::colors::text), build_date_str.c_str());

	ImGui::PopFont();
}

void ui::initialize_tabs()
{
	categories.clear();

	categories.push_back(TabCategory{
		"aimbot", "A"
		});

	categories.push_back(TabCategory{
		"visuals", "B"
		});

	categories.push_back(TabCategory{
		"settings", "D"
		});

	tab_lerp_values.resize(categories.size(), 0.0f);
}

bool ui::is_tab_selected(const std::string& tab_name) {
	if (selected_tab_index >= 0 && selected_tab_index < static_cast<int>(categories.size())) {
		return categories[selected_tab_index].name == tab_name;
	}
	return false;
}

void ui::render_tabs(float dt)
{
	ImVec2 pos = ImGui::GetWindowPos();
	ImVec2 size = ImGui::GetWindowSize();

	float tab_height = 39.0f;

	ImVec2 rect_pos = ImVec2(pos.x, pos.y + size.y - tab_height);

	float padding_x = 15.0f;
	float spacing = 30.0f;
	float space_between_icon_and_text = 5.0f;

	float x = rect_pos.x + padding_x;
	float y_center = rect_pos.y + tab_height * 0.5f - 1;

	ImVec2 mouse_pos = ImGui::GetIO().MousePos;

	bool mouse_hand_set = false;

	ImVec4 base_color = ui::colors::text;
	ImVec4 highlight_color = ui::colors::main;

	float anim_speed = 12.0f;

	for (size_t i = 0; i < categories.size(); ++i)
	{
		float target = (selected_tab_index == (int)i) ? 1.0f : 0.0f;
		tab_lerp_values[i] = ImLerp(tab_lerp_values[i], target, dt * anim_speed);
	}

	for (size_t i = 0; i < categories.size(); ++i)
	{
		const auto& tab = categories[i];

		std::string icon = tab.icon;
		std::string name = tab.name;

		ImGui::PushFont(font.tab_icon);
		ImVec2 icon_size = ImGui::CalcTextSize(icon.c_str());
		ImGui::PopFont();

		ImGui::PushFont(font.spacegrotesk_medium[1]);
		ImVec2 name_size = ImGui::CalcTextSize(name.c_str());
		ImGui::PopFont();

		float icon_offset_y = 0.0f;

		if (icon == "A") icon_offset_y = 0.0f;
		else if (icon == "B") icon_offset_y = 2.0f;
		else if (icon == "D") icon_offset_y = 2.0f;

		ImVec2 icon_pos = ImVec2(x, y_center - icon_size.y * 0.5f + icon_offset_y);
		ImVec2 name_pos = ImVec2(icon_pos.x + icon_size.x + space_between_icon_and_text, y_center - name_size.y * 0.5f);

		ImRect icon_rect(icon_pos, ImVec2(icon_pos.x + icon_size.x, icon_pos.y + icon_size.y));
		ImRect name_rect(name_pos, ImVec2(name_pos.x + name_size.x, name_pos.y + name_size.y));

		ImVec4 lerp_color = ImLerp(base_color, highlight_color, tab_lerp_values[i]);
		ImColor color(lerp_color.x, lerp_color.y, lerp_color.z, lerp_color.w);

		ui::items::text_shadow(ImGui::GetBackgroundDrawList(), icon_pos, color, icon.c_str(), font.tab_icon);

		ui::items::text_shadow(ImGui::GetBackgroundDrawList(), name_pos, color, name.c_str(), font.spacegrotesk_medium[1]);

		if (icon_rect.Contains(mouse_pos) || name_rect.Contains(mouse_pos))
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			mouse_hand_set = true;

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				selected_tab_index = (int)i;
		}

		x += icon_size.x + space_between_icon_and_text + name_size.x + spacing;
	}

	if (!mouse_hand_set)
		ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
}

void ui::render_outline()
{
	ImVec2 pos(ImGui::GetWindowPos());
	ImVec2 size(pos.x + ui::size.x, pos.y + ui::size.y);

	ImGui::GetBackgroundDrawList()->AddRect(
		pos,
		size,
		IM_COL32(
			ui::colors::outline.x * 255,
			ui::colors::outline.y * 255,
			ui::colors::outline.z * 255,
			20
		),
		ui::rounding - 1,
		ImDrawFlags_RoundCornersAll);
}
