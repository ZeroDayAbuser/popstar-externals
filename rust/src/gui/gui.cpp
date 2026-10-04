#include "../globals.hpp"
#include "gui.hpp"

#include "../app/winapp.hpp"

#include <cmath>
#include <format>
#include <imgui.h>
#include <vector>

#include <string_encryption.hpp>

#include <app/app.hpp>
#include <window/window.hpp>

#include <render/render.hpp>
#include <utils/style.hpp>

#include <game/cache/cache.hpp>
#include <game/features/visuals/visuals.hpp>
#include <sdk/math/math.hpp>
#include <sdk/rust/entity/entity.hpp>

#include <game/features/aimbot/aimbot.hpp>
#include <game/game.hpp>
#include <gordoui/OverlayBridge.hpp>

#include "../settings/settings.hpp"

namespace gui
{
	void on_render()
	{
		render::draw_list = ImGui::GetBackgroundDrawList();

		features::visuals::on_render();
		features::aimbot::on_render();

		bool& crosshair = settings.visuals.screen.crosshair;
		if (crosshair)
		{
			Color& crosshair_color = settings.visuals.screen.crosshair_color;
			const glm::vec2 center = winapp::impl::window_size * 0.5f;

			if (settings.visuals.screen.crosshair_spin)
			{
				static float spin_angle = 0.0f;
				spin_angle += ImGui::GetIO().DeltaTime * 3.0f;
				constexpr float kTwoPi = 6.28318530718f;
				if (spin_angle > kTwoPi) spin_angle -= kTwoPi;

				constexpr float kRadius   = 14.0f;
				constexpr float kArcSpan  = 0.85f;   // radians — a bit under 50°
				constexpr int   kSegments = 10;
				constexpr float kThird    = kTwoPi / 3.0f;

				const Color outline = Color::black().scale_alpha(crosshair_color.scalable_alpha());

				for (int side = 0; side < 3; ++side)
				{
					const float base_angle = spin_angle + kThird * static_cast<float>(side);
					const float arc_start  = base_angle - kArcSpan * 0.5f;

					std::vector<glm::vec2> points;
					points.reserve(static_cast<std::size_t>(kSegments) + 1);
					for (int i = 0; i <= kSegments; ++i)
					{
						const float t = static_cast<float>(i) / static_cast<float>(kSegments);
						const float a = arc_start + kArcSpan * t;
						points.emplace_back(center + glm::vec2{ std::cos(a) * kRadius, std::sin(a) * kRadius });
					}

					render::add_polyline(points, outline,          3.0f, false);
					render::add_polyline(points, crosshair_color,  1.75f, false);
				}
			}
			else
			{
				render::add_circle_filled(center, 3.0f, Color::black().scale_alpha(crosshair_color.scalable_alpha()));
				render::add_circle_filled(center, 2.0f, crosshair_color);
			}
		}

		is_open = OverlayBridge::IsMenuOpen();
		winapp::impl::window_size = glm::vec2{ ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y };

		bool& vertical_sync = settings.settings.ui.vertical_sync;
		winapp::impl::present_with_virtual_sync = vertical_sync;

		bool& override_accent = settings.settings.ui.override_accent;
		Color& accent = settings.settings.ui.accent_color;
		gui::style::colors::accent = override_accent ? accent : Color(255, 85, 200);
	}
}