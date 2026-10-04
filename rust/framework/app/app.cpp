#include "../base/events/events.hpp"
#include "../config/config.hpp"
#include "../controls/button/button.hpp"
#include "../controls/checkbox/checkbox.hpp"
#include "../controls/container/container.hpp"
#include "../controls/dropdown/dropdown.hpp"
#include "../controls/label/label.hpp"
#include "../controls/listbox/listbox.hpp"
#include "../controls/popup/popup.hpp"
#include "../controls/slider/slider.hpp"
#include "../controls/subtab_bar/subtab_bar.hpp"
#include "../controls/tab_control/tab_control.hpp"
#include "../controls/text_input/text_input.hpp"

#include <app/app.hpp>
#include <controls/color_picker/color_picker.hpp>
#include <controls/multi_dropdown/multi_dropdown.hpp>
#include <ctime>
#include <iomanip>
#include <map>
#include <render/assets/font_awesome.hpp>
#include <render/render.hpp>
#include <sstream>
#include <string>
#include <string_encryption.hpp>
#include <utils/style.hpp>
#include <windowsx.h>

#include <sdk/globals.hpp>
#include <sdk/rust/entity/entity.hpp>
#include <sdk/rust/entity/weapon.hpp>

#include <game/cache/cache.hpp>
#include <game/game.hpp>
#include <globals.hpp>

namespace app
{
	gui::Slider<float>* g_time_changer_slider{ nullptr };
	std::vector<std::pair<std::string, gui::SubTabBar::TabPage*>> g_weapon_group_tabs{};

	struct GroupControls {
		gui::SubTabBar* inner_bar{};
	};

	std::map<std::string, GroupControls> g_group_controls{};

	void setup()
	{
		{
			std::lock_guard lock(gui_mutex);
			windows.clear();
		}

		config::init();
		render::setup();

		auto window = std::make_shared<gui::Window>(xs("Pawjob"), glm::vec2{ 100.f, 100.f }, glm::vec2{ 680, 550 }); // 580, 450
		window->window_title_show = xs("Pawjobs");
		window->window_tld = xs(".club");

		if (auto tabs = window->add_object<gui::TabControl>())
		{
			if (auto aimbot_tab = tabs->add_tab(xs("Aimbot"), ICON_FA_ARROW_POINTER))
			{
				auto setup_group_page = [&](std::string group_name, gui::SubTabBar::TabPage* page, rust::WeaponTier tier) {
					if (auto settings = page->add_container(xs("Main")))
					{
						auto main_toggle = settings->add_object<gui::Checkbox>(group_name == xs("General") ? xs("Enabled") : xs("Override general"));
						if (auto popup = main_toggle->add_object<gui::Popup>(xs("Options")))
						{
							auto dist_slider = popup->add_object<gui::Slider<int>>(xs("Max distance"), 0, 500, 350, xs(""), 0, 10);
							dist_slider->format_value = [](int val) -> std::string {
								if (val >= 500) return xs("Infinite");
								return std::to_string(val) + xs("m");
								};
						}

						auto silent = settings->add_object<gui::Checkbox>(xs("Silent aim"));
						if (auto popup = silent->add_object<gui::Popup>(xs("Options")))
							popup->add_object<gui::Slider<int>>(xs("Hit chance"), 0, 100, 100, xs("%"));

						settings->add_object<gui::Checkbox>(xs("Smart target"));
						settings->add_object<gui::Checkbox>(xs("Target 360"));
						settings->add_object<gui::Checkbox>(xs("Desync"));
						settings->add_object<gui::Checkbox>(xs("Always shoot"));

						// order MUST match resolve_hit_bone() switch in aimbot.cpp.
						static const std::vector<std::string> aimbot_bone_names = {
							xs("Head"), xs("Neck"), xs("Chest"),
							xs("Stomach"), xs("Pelvis"), xs("Legs"),
						};
						settings->add_object<gui::Dropdown>(xs("Target bone"), 0, aimbot_bone_names);

						// off by default so existing aim feel is unchanged.
						auto prediction = settings->add_object<gui::Checkbox>(xs("Prediction"));
						if (auto popup = prediction->add_object<gui::Popup>(xs("Options")))
						{
							// 1.0 = physically correct after ping comp.
							popup->add_object<gui::Slider<float>>(xs("Strength"), 0.3f, 2.5f, 1.0f, xs("x"), 1, 0.05f);

							popup->add_object<gui::Slider<float>>(xs("Drop"), 0.0f, 3.0f, 1.0f, xs("x"), 1, 0.05f);

							// subtracted from t_of — server rewinds targets by this much. 80ms = NA default.
							auto ping = popup->add_object<gui::Slider<int>>(xs("Ping comp"), 0, 250, 80, xs("ms"), 0, 5);
							ping->format_value = [](int v) -> std::string { return std::to_string(v) + xs(" ms"); };
						}
					}

					if (auto filters = page->add_container(xs("Filters")))
					{
						filters->add_object<gui::Checkbox>(xs("Skip scientists"), true);
						filters->add_object<gui::Checkbox>(xs("Skip dwellers"), true);
						filters->add_object<gui::Checkbox>(xs("Skip sleepers"), true);
						filters->add_object<gui::Checkbox>(xs("Skip wounded"), true);
						filters->add_object<gui::Checkbox>(xs("Skip animals"), true);
					}

					if (auto humanization = page->add_container(xs("Humanization")))
					{
						humanization->add_object<gui::Slider<float>>(xs("Smoothing"), 1.0f, 50.0f, 10.0f, xs(""), 0, 0.05f);
					}
					};

				if (auto weapons_bar = aimbot_tab->add_full_subtab_bar())
				{
					if (auto general_page = weapons_bar->add_tab(xs("General")))
					{
						setup_group_page(xs("General"), general_page, rust::WeaponTier::None);
					}

					g_weapon_group_tabs.push_back({ xs("Rifles"), weapons_bar->add_tab(xs("Rifles")) });
					g_weapon_group_tabs.push_back({ xs("Snipers"), weapons_bar->add_tab(xs("Snipers")) });
					g_weapon_group_tabs.push_back({ xs("Shotguns"), weapons_bar->add_tab(xs("Shotguns")) });
					g_weapon_group_tabs.push_back({ xs("Pistols"), weapons_bar->add_tab(xs("Pistols")) });
					g_weapon_group_tabs.push_back({ xs("Bows"), weapons_bar->add_tab(xs("Bows")) });
					g_weapon_group_tabs.push_back({ xs("LMGs"), weapons_bar->add_tab(xs("LMGs")) });
					g_weapon_group_tabs.push_back({ xs("SMGs"), weapons_bar->add_tab(xs("SMGs")) });

					setup_group_page(xs("Rifles"), g_weapon_group_tabs[0].second, rust::WeaponTier::Rifle);
					setup_group_page(xs("Snipers"), g_weapon_group_tabs[1].second, rust::WeaponTier::Sniper);
					setup_group_page(xs("Shotguns"), g_weapon_group_tabs[2].second, rust::WeaponTier::Shotgun);
					setup_group_page(xs("Pistols"), g_weapon_group_tabs[3].second, rust::WeaponTier::Pistol);
					setup_group_page(xs("Bows"), g_weapon_group_tabs[4].second, rust::WeaponTier::Bow);
					setup_group_page(xs("LMGs"), g_weapon_group_tabs[5].second, rust::WeaponTier::LMG);
					setup_group_page(xs("SMGs"), g_weapon_group_tabs[6].second, rust::WeaponTier::SMG);
				}

				if (auto visualization = aimbot_tab->add_container(xs("Visualization")))
				{
					auto fov_circle = visualization->add_object<gui::Checkbox>(xs("Field of view circle"));
					fov_circle->add_object<gui::ColorPicker>(xs("Color"), Color::white());
					if (auto popup = fov_circle->add_object<gui::Popup>(xs("Options")))
					{
						popup->add_object<gui::Slider<int>>(xs("Size"), 10, 500, 125, xs("px"), 0, 1);

						auto target_line = popup->add_object<gui::Checkbox>(xs("Target line"));
						target_line->add_object<gui::ColorPicker>(xs("Color"), Color::white());
					}

					auto tracers = visualization->add_object<gui::Checkbox>(xs("Bullet tracers"));
					tracers->add_object<gui::ColorPicker>(xs("Color"), Color(255, 200, 80));

					auto path = visualization->add_object<gui::Checkbox>(xs("Prediction path"));
					path->add_object<gui::ColorPicker>(xs("Color"), Color(100, 200, 255));
				}

				if (auto weapons = aimbot_tab->add_container(xs("Weapons")))
				{
					weapons->add_object<gui::Checkbox>(xs("Automatic weapons"));
					weapons->add_object<gui::Checkbox>(xs("Instant bow"));
					weapons->add_object<gui::Checkbox>(xs("Instant eoka"));
					weapons->add_object<gui::Checkbox>(xs("Thick bullet"));
					weapons->add_object<gui::Checkbox>(xs("Rapid fire"));
					weapons->add_object<gui::Checkbox>(xs("No sway"));
					weapons->add_object<gui::Checkbox>(xs("No animation"));

					static const std::vector<std::string> bone_names = {
						xs("Head"), xs("Neck"), xs("Chest"),
						xs("Stomach"), xs("Pelvis"), xs("Legs"),
					};
					auto hitbox = weapons->add_object<gui::Checkbox>(xs("Hitbox override"));
					hitbox->add_object<gui::Dropdown>(xs("Bone"), 0, bone_names);

					auto spread = weapons->add_object<gui::Checkbox>(xs("Override weapon spread"));
					if (auto popup = spread->add_object<gui::Popup>(xs("Options")))
						popup->add_object<gui::Slider<int>>(xs("Amount"), 0, 100, 55, "%");

					auto recoil = weapons->add_object<gui::Checkbox>(xs("Override weapon recoil"));
					if (auto popup = recoil->add_object<gui::Popup>(xs("Options")))
						popup->add_object<gui::Slider<int>>(xs("Amount"), 0, 100, 0, "%");

					auto melee = weapons->add_object<gui::Checkbox>(xs("Override melee range"));
					if (auto popup = melee->add_object<gui::Popup>(xs("Options")))
						popup->add_object<gui::Slider<int>>(xs("Amount"), 100, 500, 100, "%");
				}
			}

			if (auto visuals_tab = tabs->add_tab(xs("Visuals"), ICON_FA_MOON))
			{
				// flags() ORs this with the Players-nested copy.
				visuals_tab->add_object<gui::MultiDropdown>(xs("Flags"), 0,
					std::vector<std::string>{xs("Distance"), xs("Sleeping"), xs("Wounded"), xs("Team ID"), xs("Aiming")});

				if (auto esp = visuals_tab->add_container(xs("Players")))
				{
					auto enabled = esp->add_object<gui::Checkbox>(xs("Enabled"), true);
					if (auto popup = enabled->add_object<gui::Popup>(xs("Options")))
					{
						auto dist_slider = popup->add_object<gui::Slider<int>>(xs("Max distance"), 0, 500, 350, xs(""), 0, 10);
						dist_slider->format_value = [](int val) -> std::string {
							if (val >= 500) return xs("Infinite");
							return std::to_string(val) + xs("m");
							};

						popup->add_object<gui::Checkbox>(xs("Include in radar"));
					}
					esp->add_object<gui::Checkbox>(xs("Show sleepers"));

					// raycast local eye -> target; LOS blocked = dim or hide.
					auto vischeck = esp->add_object<gui::Checkbox>(xs("Vischeck"));
					if (auto popup = vischeck->add_object<gui::Popup>(xs("Options")))
					{
						// Visible/Invisible color checkboxes are enable toggles; alpha = dim amount.
						auto vis_col = popup->add_object<gui::Checkbox>(xs("Visible color"), true);
						vis_col->add_object<gui::ColorPicker>(xs("Color"), Color::white());

						auto inv_col = popup->add_object<gui::Checkbox>(xs("Invisible color"), true);
						inv_col->add_object<gui::ColorPicker>(xs("Color"), Color(180, 180, 180, 80));

						popup->add_object<gui::Checkbox>(xs("Dim instead of hide"), true);
						popup->add_object<gui::Checkbox>(xs("Per-bone skeleton"));
					}

					auto out_of_view = esp->add_object<gui::Checkbox>(xs("Out of view arrows"));
					out_of_view->add_object<gui::ColorPicker>(xs("Color"), Color::white());
					if (auto popup = out_of_view->add_object<gui::Popup>(xs("Options")))
					{
						popup->add_object<gui::Checkbox>(xs("Glow"), true);

						popup->add_object<gui::Slider<float>>(xs("Radius"), 0.0f, 2.0f, 0.5f);
						popup->add_object<gui::Slider<float>>(xs("Size"), 0.0f, 30.0f, 10.0f);
					}

					auto name_esp = esp->add_object<gui::Checkbox>(xs("Name"), true);
					name_esp->add_object<gui::ColorPicker>(xs("Color"), Color::white());
					if (auto popup = name_esp->add_object<gui::Popup>(xs("Options")))
					{
						popup->add_object<gui::Checkbox>(xs("Show avatar"));
					}

					auto box = esp->add_object<gui::Checkbox>(xs("Bounding box"));
					box->add_object<gui::ColorPicker>(xs("Color"), Color::white());
					if (auto popup = box->add_object<gui::Popup>(xs("Options")))
					{
						auto gradient = popup->add_object<gui::Checkbox>(xs("Gradient"));
						gradient->add_object<gui::ColorPicker>(xs("Color"), Color(255, 180, 255));

						auto fill = popup->add_object<gui::Checkbox>(xs("Fill"));
						fill->add_object<gui::ColorPicker>(xs("Color top"), Color(0, 255, 255, 100));
						fill->add_object<gui::ColorPicker>(xs("Color bottom"), Color(255, 100, 255, 100));
					}

					auto skeleton = esp->add_object<gui::Checkbox>(xs("Skeleton"), true);
					skeleton->add_object<gui::ColorPicker>(xs("Color"), Color::white());
					if (auto popup = skeleton->add_object<gui::Popup>(xs("Options")))
					{
						popup->add_object<gui::Checkbox>(xs("Rounded"));
						// When on, every bone segment is raycast from the local eye
						// and colored green (visible) / red (occluded). Overrides
						// the user-picked skeleton color while it's enabled.
						popup->add_object<gui::Checkbox>(xs("Vis colors"));
					}

					auto head_circle = esp->add_object<gui::Checkbox>(xs("Head circle"));
					head_circle->add_object<gui::ColorPicker>(xs("Color"), Color::white());

					auto view_line = esp->add_object<gui::Checkbox>(xs("View line"));
					view_line->add_object<gui::ColorPicker>(xs("Color"), Color::white());

					auto snap_line = esp->add_object<gui::Checkbox>(xs("Snap lines"));
					snap_line->add_object<gui::ColorPicker>(xs("Color"), Color::white());
					if (auto popup = snap_line->add_object<gui::Popup>(xs("Options")))
						popup->add_object<gui::Dropdown>(xs("Alignment"), 1, std::vector<std::string>{xs("Top"), xs("Middle"), xs("Bottom")});

					auto item_name = esp->add_object<gui::Checkbox>(xs("Item name"));
					item_name->add_object<gui::ColorPicker>(xs("Color"), Color::white());

					auto item_icon = esp->add_object<gui::Checkbox>(xs("Item icon"));
					item_icon->add_object<gui::ColorPicker>(xs("Color"), Color::white());

					auto flags = esp->add_object<gui::MultiDropdown>(xs("Flags"), 0, std::vector<std::string>{xs("Distance"), xs("Sleeping"), xs("Wounded"), xs("Team ID"), xs("Aiming")});
					if (auto popup = flags->add_object<gui::Popup>(xs("Options")))
					{
						auto sleeping_color = popup->add_object<gui::Label>(xs("Sleeping color"));
						sleeping_color->add_object<gui::ColorPicker>(xs("Color"), Color::white());

						auto wounded_color = popup->add_object<gui::Label>(xs("Wounded color"));
						wounded_color->add_object<gui::ColorPicker>(xs("Color"), Color::white());

						auto team_id_color = popup->add_object<gui::Label>(xs("Team ID color"));
						team_id_color->add_object<gui::ColorPicker>(xs("Color"), Color::white());

						auto ads_color = popup->add_object<gui::Label>(xs("Aiming color"));
						ads_color->add_object<gui::ColorPicker>(xs("Color"), Color::white());
					}

					esp->add_object<gui::Dropdown>(xs("Chams"), 0, g_material_names);
				}

				if (auto entities = visuals_tab->add_container(xs("Entities")))
				{
					entities->hide_subtab_labels = true;
					// Master Entities toggle defaults ON — it's the "kill
					// all ESP" panic switch, not the per-feature gate.
					// Per-category and per-item toggles default OFF
					// (see add_entity_category below) so customers still
					// opt-in to each category, but enabling one actually
					// works without first hunting for this master switch.
					entities->add_object<gui::Checkbox>(xs("Enabled"), true);

					auto add_entity_category = [&](const std::string& category_name, const std::string& icon, std::vector<std::pair<std::string, Color>> items) {
						if (auto page = entities->add_subtab(category_name, icon)) {
							// Category master toggle defaults OFF — customers
							// enable the categories they want; nothing draws
							// until the category AND the per-item box are on.
							page->add_object<gui::Checkbox>(xs("Enabled"), false);

							for (const auto& item : items) {
								auto checkbox = page->add_object<gui::Checkbox>(item.first, false);
								checkbox->add_object<gui::ColorPicker>(xs("Color"), item.second);
								if (auto popup = checkbox->add_object<gui::Popup>(xs("Options"))) {
									auto dist_slider = popup->add_object<gui::Slider<int>>(xs("Max distance"), 0, 500, 250, xs(""), 0, 10);
									dist_slider->format_value = [](int val) -> std::string {
										if (val >= 500) return xs("Infinite");
										return std::to_string(val) + xs("m");
										};

									popup->add_object<gui::Checkbox>(xs("Include in radar"));
								}
							}
						}
						};

					add_entity_category(xs("Ores & Collectibles"), ICON_FA_GEM, {
						{xs("Stone"), Color(150, 150, 150)}, {xs("Sulfur"), Color(200, 200, 50)}, {xs("Metal"), Color(150, 100, 50)},
						{xs("Hemp"), Color(50, 200, 50)}, {xs("Wood"), Color(100, 50, 0)}, {xs("Diesel fuel"), Color(100, 100, 100)},
						{xs("Green keycard"), Color(50, 200, 50)}, {xs("Blue keycard"), Color(50, 50, 200)}, {xs("Red keycard"), Color(200, 50, 50)}
						});

					add_entity_category(xs("Crates & Barrels"), ICON_FA_BOX, {
						{xs("Elite crate"), Color(200, 50, 200)}, {xs("Military crate"), Color(50, 200, 50)}, {xs("Locked crate"), Color(200, 50, 50)},
						{xs("Air drop"), Color(200, 200, 50)}, {xs("Loot barrel"), Color(150, 150, 150)}, {xs("Oil barrel"), Color(200, 50, 50)},
						{xs("Food crate"), Color(50, 200, 50)}, {xs("Tool crate"), Color(150, 150, 150)}, {xs("Small crate"), Color(150, 150, 150)},
						{xs("Health crate"), Color(200, 50, 50)}, {xs("Small food crate"), Color(50, 200, 50)}, {xs("Vehicle parts"), Color(150, 150, 150)},
						{xs("Crate"), Color(150, 150, 150)}
						});

					add_entity_category(xs("Deployables"), ICON_FA_HAMMER, {
						{xs("Wooden box"), Color(150, 100, 50)}, {xs("Large wood box"), Color(150, 100, 50)}, {xs("Tool cupboard"), Color(200, 50, 50)},
						{xs("Tier 1 workbench"), Color(150, 150, 150)}, {xs("Tier 2 workbench"), Color(150, 150, 150)}, {xs("Tier 3 workbench"), Color(150, 150, 150)},
						{xs("Repair bench"), Color(150, 150, 150)}, {xs("Research table"), Color(150, 150, 150)}, {xs("Small stash"), Color(200, 200, 50)},
						{xs("Sleeping bag"), Color(50, 200, 200)}, {xs("Furnace"), Color(200, 100, 50)}, {xs("Locker"), Color(150, 150, 150)}
						});

					add_entity_category(xs("Traps & Turrets"), ICON_FA_SHIELD_HALVED, {
						{xs("Auto turret"), Color(200, 50, 50)}, {xs("Flame turret"), Color(200, 100, 50)}, {xs("Shotgun trap"), Color(200, 200, 50)}
						});

					add_entity_category(xs("NPCs & Animals"), ICON_FA_PAW, {
						{xs("Scientist"), Color(50, 50, 200)}, {xs("Dweller"), Color(200, 50, 50)},
						{xs("Bear"), Color(150, 100, 50)}, {xs("Wolf"), Color(100, 100, 100)}, {xs("Stag"), Color(150, 100, 50)},
						{xs("Boar"), Color(150, 100, 50)}, {xs("Horse"), Color(150, 100, 50)}, {xs("Chicken"), Color(200, 200, 200)},
						{xs("Shark"), Color(50, 50, 200)}, {xs("Scarecrow"), Color(200, 100, 50)}, {xs("Corpse"), Color(150, 150, 150)}
						});

					add_entity_category(xs("Static & Monuments"), ICON_FA_BUILDING, {
						{xs("Recycler"), Color(150, 150, 150)}, {xs("Phone booth"), Color(200, 50, 200)}, {xs("Fuse box"), Color(50, 200, 50)},
						{xs("Refinery"), Color(200, 100, 50)}, {xs("Elevator"), Color(150, 150, 150)}, {xs("Car lift"), Color(150, 150, 150)},
						{xs("Computer station"), Color(150, 150, 150)}
						});

					add_entity_category(xs("Vehicles"), ICON_FA_CAR, {
						{xs("Motorbike"), Color(200, 200, 200)}, {xs("Sidecar motorbike"), Color(200, 200, 200)}, {xs("Pedal bike"), Color(200, 200, 200)},
						{xs("Snowmobile"), Color(200, 200, 200)}, {xs("Tomaha snowmobile"), Color(200, 200, 200)},
						{xs("Tugboat"), Color(50, 150, 200)}, {xs("Rowboat"), Color(150, 100, 50)}, {xs("RHIB"), Color(50, 150, 200)}, {xs("Kyak"), Color(200, 100, 50)},
						{xs("Solo submarine"), Color(50, 150, 200)}, {xs("Duo submarine"), Color(50, 150, 200)},
						{xs("Minicopter"), Color(200, 200, 200)}, {xs("Scrap heli"), Color(200, 200, 200)}, {xs("Attack heli"), Color(200, 200, 200)}
						});

					add_entity_category(xs("Dropped weapons"), ICON_FA_GUN, {
						{xs("Dropped rifles"), Color(200, 50, 50)}, {xs("Dropped snipers"), Color(50, 50, 200)}, {xs("Dropped SMGs"), Color(200, 200, 50)},
						{xs("Dropped shotguns"), Color(200, 100, 50)}, {xs("Dropped pistols"), Color(50, 200, 200)}, {xs("Dropped LMGs"), Color(200, 50, 200)},
						{xs("Dropped launchers"), Color(200, 200, 200)}, {xs("Dropped bows"), Color(150, 100, 50)}, {xs("Dropped melee"), Color(150, 150, 150)},
						{xs("Dropped misc"), Color(150, 150, 150)}
						});

					add_entity_category(xs("Dropped items"), ICON_FA_BOX, {
						{xs("Throwables"), Color(255, 100, 50)},
						{xs("Tools"),      Color(180, 180, 180)},
						{xs("Medical"),    Color(255, 80, 80)},
						{xs("Ammo"),       Color(255, 220, 80)},
						{xs("Misc"),       Color(200, 200, 200)},
						});
				}

				if (auto screen = visuals_tab->add_container(xs("Screen")))
				{
					auto crosshair = screen->add_object<gui::Checkbox>(xs("Crosshair"));
					crosshair->add_object<gui::ColorPicker>(xs("Color"), Color(255, 85, 200));

					screen->add_object<gui::Checkbox>(xs("Player inventory"));
				}

				if (auto local = visuals_tab->add_container(xs("Local")))
				{
					local->add_object<gui::Checkbox>(xs("Custom FOV"));
					auto fov_slider = local->add_object<gui::Slider<float>>(xs("Field of view"), 40.0f, 150.0f, 90.0f);
					if (auto popup = fov_slider->add_object<gui::Popup>(xs("Options")))
						popup->add_object<gui::Slider<float>>(xs("Zoom FOV"), 10.0f, 150.0f, 35.0f);
					local->add_object<gui::Checkbox>(xs("Third person"));

					local->add_object<gui::Checkbox>(xs("Debug camera"));
					local->add_object<gui::Slider<float>>(xs("Debug camera speed"), 1.0f, 50.0f, 10.0f, xs(" m/s"), 0, 1.0f);
					local->add_object<gui::Slider<float>>(xs("Mouse sensitivity"), 1.0f, 200.0f, 40.0f, xs("%"), 0, 1.0f);
				}

				if (auto world = visuals_tab->add_container(xs("World")))
				{
					auto time_changer_toggle = world->add_object<gui::Checkbox>(xs("Time changer"));
					g_time_changer_slider = world->add_object<gui::Slider<float>>(xs("Time"), 0.0f, 24.0f, 18.0f, xs(""), 0, 1.0f / 12.0f);
					g_time_changer_slider->format_value = [](float val) {
						int hours = static_cast<int>(val);
						int minutes = static_cast<int>((val - hours) * 60.0f);
						if (hours >= 24) { hours = 23; minutes = 59; }

						std::string period = (hours >= 12) ? xs("pm") : xs("am");
						int display_hours = (hours % 12 == 0) ? 12 : hours % 12;

						std::ostringstream ss;
						ss << display_hours << xs(":") << std::setfill('0') << std::setw(2) << minutes << period;
						return ss.str();
						};

					auto sun_color = world->add_object<gui::Checkbox>(xs("Modulate sun"));
					sun_color->add_object<gui::ColorPicker>(xs("Color"), Color(125, 125, 205));

					auto light_color = world->add_object<gui::Checkbox>(xs("Modulate lights"));
					light_color->add_object<gui::ColorPicker>(xs("Color"), Color(125, 125, 205));

					auto rays_color = world->add_object<gui::Checkbox>(xs("Modulate rays"));
					rays_color->add_object<gui::ColorPicker>(xs("Color"), Color(125, 125, 205));

					auto sky_color = world->add_object<gui::Checkbox>(xs("Modulate sky"));
					sky_color->add_object<gui::ColorPicker>(xs("Color"), Color(125, 125, 205));

					auto clouds_color = world->add_object<gui::Checkbox>(xs("Modulate clouds"));
					clouds_color->add_object<gui::ColorPicker>(xs("Color"), Color(125, 125, 205));

					auto fog_color = world->add_object<gui::Checkbox>(xs("Modulate fog"));
					fog_color->add_object<gui::ColorPicker>(xs("Color"), Color(125, 125, 205));

					auto ambient_color = world->add_object<gui::Checkbox>(xs("Modulate ambience"));
					ambient_color->add_object<gui::ColorPicker>(xs("Color"), Color(125, 125, 205));

					// Bright night = simple on/off; brightness is fixed at 20×
					// in the spin thread (see misc.cpp:bright_night_spin_loop).
					// The old per-slider value was a flicker risk anyway —
					// going static makes the result rock-solid.
					world->add_object<gui::Checkbox>(xs("Bright night"));
				}


				if (auto radar = visuals_tab->add_container(xs("Radar")))
				{
					radar->add_object<gui::Checkbox>(xs("Enabled"));
					radar->add_object<gui::Slider<float>>(xs("Size"), 50.0f, 250.0f, 120.0f, xs("px"));
					radar->add_object<gui::Slider<float>>(xs("Range"), 10.0f, 500.0f, 150.0f, xs("m"));
				}
			}

			if (auto misc_tab = tabs->add_tab(xs("Misc"), ICON_FA_BURGER))
			{
				if (auto general = misc_tab->add_container(xs("General")))
				{
					general->add_object<gui::Checkbox>(xs("Instant untie crate"));
					general->add_object<gui::Checkbox>(xs("Fast loot"));
					general->add_object<gui::Checkbox>(xs("Instant interactions"));
					general->add_object<gui::Checkbox>(xs("Instant revive"));

					auto debug_esp = general->add_object<gui::Checkbox>(xs("Debug ESP"));
					debug_esp->add_object<gui::ColorPicker>(xs("Color"), Color::white());
				}

				if (auto movement = misc_tab->add_container(xs("Movement")))
				{
					movement->add_object<gui::Checkbox>(xs("Spider man"));
					movement->add_object<gui::Checkbox>(xs("Omni sprint"));
					movement->add_object<gui::Checkbox>(xs("Fly"));
					movement->add_object<gui::Checkbox>(xs("Silent walk"));
					movement->add_object<gui::Checkbox>(xs("No fall"));
					movement->add_object<gui::Checkbox>(xs("Speed hack"));
					movement->add_object<gui::Checkbox>(xs("Remove water drag"));
					movement->add_object<gui::Checkbox>(xs("Walk on water"));
					movement->add_object<gui::Checkbox>(xs("Anti-aim"));
				}
			}

			if (auto settings_tab = tabs->add_tab(xs("Settings"), ICON_FA_GEAR))
			{
				if (auto ui = settings_tab->add_container(xs("UI")))
				{
					auto accent = ui->add_object<gui::Checkbox>(xs("Override accent"));
					accent->add_object<gui::ColorPicker>(xs("Color"), gui::style::colors::accent);
					ui->add_object<gui::Checkbox>(xs("Vertical sync"));
					ui->add_object<gui::Checkbox>(xs("Enable memory writes"));
				}

				if (auto configs = settings_tab->add_container(xs("Configs")))
				{
					configs->should_save = false;

					auto config_list = configs->add_object<gui::Listbox>(xs("Config files"), std::vector<std::string>{}, 10);
					config_list->show_label = false;
					auto config_name = configs->add_object<gui::TextInput>(xs("Config name"), xs("Enter new config name"));
					config_name->show_label = false;

					class CreateButton : public gui::Button {
					public:
						gui::TextInput* input;
						CreateButton(gui::TextInput* i) : gui::Button(xs("Create")), input(i) {}
						void update_layout() override {
							should_display = !input->storage.empty();
							gui::Button::update_layout();
						}
					};

					auto create_btn = configs->add_object<CreateButton>(config_name);
					create_btn->on_press = [config_list, config_name]() {
						std::string name = config_name->storage;
						if (!name.empty())
						{
							config::save_by_name(name);
							config_name->storage.clear();
							config::refresh_file_list(config_list);
						}
						};

					auto save_btn = configs->add_object<gui::Button>(xs("Save"));
					save_btn->on_press = [config_list]() {
						if (config_list->value >= 0 && config_list->value < (int)config_list->options.size())
						{
							std::string name = config_list->options[config_list->value];
							if (!name.empty())
							{
								config::save_by_name(name);
								config::refresh_file_list(config_list);
							}
						}
						};

					auto load_btn = configs->add_object<gui::Button>(xs("Load"));
					load_btn->on_press = [config_list]() {
						if (config_list->value >= 0 && config_list->value < (int)config_list->options.size())
							config::load_by_name(config_list->options[config_list->value]);
						};

					auto reset_btn = configs->add_object<gui::Button>(xs("Reset"));
					reset_btn->on_press = []() {
						config::reset();
						};

					config::refresh_file_list(config_list);
				}
			}
		}

		windows.push_back(window);
	}

	void render()
	{
		render::set_to_background();

		handle_debug_hotkey();

		for (auto& window : windows)
		{
			window->render();
		}

		bool time_changer_toggle = get_value<gui::Checkbox>(xs("Pawjob"), xs("Visuals"), xs("World"), xs("Time changer"));
		if (g_time_changer_slider)
			g_time_changer_slider->should_display = time_changer_toggle;

	}

	std::string get_pretty_time() {
		std::time_t t = std::time(nullptr);
		std::tm* now = std::localtime(&t);
		std::stringstream ss;

		int hour = now->tm_hour;
		std::string period = (hour >= 12) ? "pm" : "am";

		hour = (hour % 12 == 0) ? 12 : hour % 12;

		ss << hour << ":"
			<< std::setfill('0') << std::setw(2) << now->tm_min
			<< period;

		return ss.str();
	}

	

	bool on_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		bool handled = false;

		auto dispatch = [&](gui::Event& e) {
			for (auto it = windows.rbegin(); it != windows.rend(); ++it) {
				(*it)->dispatch_event(e);
				if (e.handled) {
					handled = true;
					break;
				}
			}
			};

		switch (msg)
		{
		case WM_MOUSEMOVE:
		{
			gui::MouseMoveEvent e({ (float)GET_X_LPARAM(lparam), (float)GET_Y_LPARAM(lparam) });
			dispatch(e);
			break;
		}
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		{
			if (msg == WM_LBUTTONDOWN) ::SetCapture(hwnd);
			else ::ReleaseCapture();

			gui::MouseButtonEvent e(gui::MouseButton::Left, msg == WM_LBUTTONDOWN, { (float)GET_X_LPARAM(lparam), (float)GET_Y_LPARAM(lparam) });
			dispatch(e);
			break;
		}
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		{
			if (msg == WM_RBUTTONDOWN) ::SetCapture(hwnd);
			else ::ReleaseCapture();

			gui::MouseButtonEvent e(gui::MouseButton::Right, msg == WM_RBUTTONDOWN, { (float)GET_X_LPARAM(lparam), (float)GET_Y_LPARAM(lparam) });
			dispatch(e);
			break;
		}
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		{
			if (msg == WM_MBUTTONDOWN) ::SetCapture(hwnd);
			else ::ReleaseCapture();

			gui::MouseButtonEvent e(gui::MouseButton::Middle, msg == WM_MBUTTONDOWN, { (float)GET_X_LPARAM(lparam), (float)GET_Y_LPARAM(lparam) });
			dispatch(e);
			break;
		}
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		{
			gui::MouseButton btn = (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) ? gui::MouseButton::X1 : gui::MouseButton::X2;
			gui::MouseButtonEvent e(btn, msg == WM_XBUTTONDOWN, { (float)GET_X_LPARAM(lparam), (float)GET_Y_LPARAM(lparam) });
			dispatch(e);
			break;
		}
		case WM_MOUSEWHEEL:
		{
			gui::MouseScrollEvent e((float)GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA);
			dispatch(e);
			break;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			gui::KeyPressEvent e(static_cast<std::int32_t>(wparam));
			dispatch(e);
			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			gui::KeyReleaseEvent e(static_cast<std::int32_t>(wparam));
			dispatch(e);
			break;
		}
		case WM_CHAR:
		{
			gui::KeyCharEvent e(static_cast<std::int32_t>(wparam));
			dispatch(e);
			break;
		}
		}

		return handled;
	}

	gui::Object* find_widget_recursive(gui::Object* current, std::span<const std::string_view> paths, std::size_t depth)
	{
		if (depth >= paths.size())
			return nullptr;

		gui::Object* found_obj = nullptr;
		const auto target_id = fnv::hash(paths[depth]);

		current->for_each_logical_child([&](gui::Object* child) {
			if (found_obj) return;

			if (child->id == target_id) {
				if (depth == paths.size() - 1) {
					found_obj = child;
				}
				else {
					found_obj = find_widget_recursive(child, paths, depth + 1);
				}
			}
			});

		if (!found_obj) {
			current->for_each_logical_child([&](gui::Object* child) {
				if (found_obj) return;

				if (child->m_name.empty() ||
					child->id == fnv::hash_const("TabControl") ||
					child->id == fnv::hash_const("TabControlBody") ||
					child->id == fnv::hash_const("__subtab_bar") ||
					child->id == fnv::hash_const("__full_subtab_bar"))
				{
					if (auto inner = find_widget_recursive(child, paths, depth)) {
						found_obj = inner;
					}
				}
				});
		}

		return found_obj;
	}
}
