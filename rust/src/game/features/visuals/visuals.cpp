#include "visuals.hpp"
#include <game/bundle_icons/bundle_icons.hpp>
#include <game/cache/cache.hpp>
#include <game/features/aimbot/aimbot.hpp>
#include <game/features/physx/physx.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <sdk/rust/entity/entity.hpp>

#include <settings/settings.hpp>
#include <string_encryption.hpp>
#include <utils/debug.hpp>
#include <window/window.hpp>

#include <app/winapp.hpp>
#include <cstdio>
#include <format>
#include <render/render.hpp>
#include <sdk/math/math.hpp>

#include <game/game.hpp>
#include <chrono>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utils/style.hpp>
#include <vector>

namespace features::visuals
{
	void player(rust::Entity* entity);
	void health_bar(rust::Entity* entity);
	void distance_label(rust::Entity* entity);
	void view_line(rust::Entity* entity);
	void render_as(std::string category, std::string type, rust::Entity* entity);

	void render_radar();
	void render_bullet_tracers();

	struct EntityLookup
	{
		bool* category_enabled = nullptr;
		Settings::VisualsSettings::EntityTypeSettings* type_settings = nullptr;
	};

	static const EntityLookup* lookup_entity(const std::string& category, const std::string& type)
	{
		using ETS = Settings::VisualsSettings::EntityTypeSettings;
		static const std::unordered_map<std::string, EntityLookup> table = []() {
			std::unordered_map<std::string, EntityLookup> t;
			auto& e = settings.visuals.entities;
			auto add = [&](const char* cat, const char* type, bool* cen, ETS* ts) {
				std::string key(cat);
				key.push_back('|');
				key.append(type);
				t.emplace(std::move(key), EntityLookup{ cen, ts });
			};

			{
				auto& c = e.ores_collectibles;
				add("Ores & Collectibles", "Stone",         &c.enabled, &c.stone);
				add("Ores & Collectibles", "Sulfur",        &c.enabled, &c.sulfur);
				add("Ores & Collectibles", "Metal",         &c.enabled, &c.metal);
				add("Ores & Collectibles", "Hemp",          &c.enabled, &c.hemp);
				add("Ores & Collectibles", "Wood",          &c.enabled, &c.wood);
				add("Ores & Collectibles", "Diesel fuel",   &c.enabled, &c.diesel_fuel);
				add("Ores & Collectibles", "Green keycard", &c.enabled, &c.green_keycard);
				add("Ores & Collectibles", "Blue keycard",  &c.enabled, &c.blue_keycard);
				add("Ores & Collectibles", "Red keycard",   &c.enabled, &c.red_keycard);
			}
			{
				auto& c = e.crates_barrels;
				add("Crates & Barrels", "Elite crate",      &c.enabled, &c.elite_crate);
				add("Crates & Barrels", "Military crate",   &c.enabled, &c.military_crate);
				add("Crates & Barrels", "Locked crate",     &c.enabled, &c.locked_crate);
				add("Crates & Barrels", "Air drop",         &c.enabled, &c.air_drop);
				add("Crates & Barrels", "Loot barrel",      &c.enabled, &c.loot_barrel);
				add("Crates & Barrels", "Oil barrel",       &c.enabled, &c.oil_barrel);
				add("Crates & Barrels", "Food crate",       &c.enabled, &c.food_crate);
				add("Crates & Barrels", "Tool crate",       &c.enabled, &c.tool_crate);
				add("Crates & Barrels", "Small crate",      &c.enabled, &c.small_crate);
				add("Crates & Barrels", "Health crate",     &c.enabled, &c.health_crate);
				add("Crates & Barrels", "Small food crate", &c.enabled, &c.small_food_crate);
				add("Crates & Barrels", "Vehicle parts",    &c.enabled, &c.vehicle_parts);
				add("Crates & Barrels", "Crate",            &c.enabled, &c.crate);
			}
			{
				auto& c = e.deployables;
				add("Deployables", "Wooden box",       &c.enabled, &c.wooden_box);
				add("Deployables", "Large wood box",   &c.enabled, &c.large_wood_box);
				add("Deployables", "Tool cupboard",    &c.enabled, &c.tool_cupboard);
				add("Deployables", "Tier 1 workbench", &c.enabled, &c.tier_1_workbench);
				add("Deployables", "Tier 2 workbench", &c.enabled, &c.tier_2_workbench);
				add("Deployables", "Tier 3 workbench", &c.enabled, &c.tier_3_workbench);
				add("Deployables", "Repair bench",     &c.enabled, &c.repair_bench);
				add("Deployables", "Research table",   &c.enabled, &c.research_table);
				add("Deployables", "Small stash",      &c.enabled, &c.small_stash);
				add("Deployables", "Sleeping bag",     &c.enabled, &c.sleeping_bag);
				add("Deployables", "Furnace",          &c.enabled, &c.furnace);
				add("Deployables", "Locker",           &c.enabled, &c.locker);
			}
			{
				auto& c = e.traps_turrets;
				add("Traps & Turrets", "Auto turret",  &c.enabled, &c.auto_turret);
				add("Traps & Turrets", "Flame turret", &c.enabled, &c.flame_turret);
				add("Traps & Turrets", "Shotgun trap", &c.enabled, &c.shotgun_trap);
			}
			{
				auto& c = e.npcs_animals;
				add("NPCs & Animals", "Scientist", &c.enabled, &c.scientist);
				add("NPCs & Animals", "Dweller",   &c.enabled, &c.dweller);
				add("NPCs & Animals", "Bear",      &c.enabled, &c.bear);
				add("NPCs & Animals", "Wolf",      &c.enabled, &c.wolf);
				add("NPCs & Animals", "Stag",      &c.enabled, &c.stag);
				add("NPCs & Animals", "Boar",      &c.enabled, &c.boar);
				add("NPCs & Animals", "Horse",     &c.enabled, &c.horse);
				add("NPCs & Animals", "Chicken",   &c.enabled, &c.chicken);
				add("NPCs & Animals", "Shark",     &c.enabled, &c.shark);
				add("NPCs & Animals", "Scarecrow", &c.enabled, &c.scarecrow);
				add("NPCs & Animals", "Corpse",    &c.enabled, &c.corpse);
			}
			{
				auto& c = e.static_monuments;
				add("Static & Monuments", "Recycler",         &c.enabled, &c.recycler);
				add("Static & Monuments", "Phone booth",      &c.enabled, &c.phone_booth);
				add("Static & Monuments", "Fuse box",         &c.enabled, &c.fuse_box);
				add("Static & Monuments", "Refinery",         &c.enabled, &c.refinery);
				add("Static & Monuments", "Elevator",         &c.enabled, &c.elevator);
				add("Static & Monuments", "Car lift",         &c.enabled, &c.car_lift);
				add("Static & Monuments", "Computer station", &c.enabled, &c.computer_station);
			}
			{
				auto& c = e.vehicles;
				add("Vehicles", "Motorbike",         &c.enabled, &c.motorbike);
				add("Vehicles", "Sidecar motorbike", &c.enabled, &c.sidecar_motorbike);
				add("Vehicles", "Pedal bike",        &c.enabled, &c.pedal_bike);
				add("Vehicles", "Snowmobile",        &c.enabled, &c.snowmobile);
				add("Vehicles", "Tomaha snowmobile", &c.enabled, &c.tomaha_snowmobile);
				add("Vehicles", "Tugboat",           &c.enabled, &c.tugboat);
				add("Vehicles", "Rowboat",           &c.enabled, &c.rowboat);
				add("Vehicles", "RHIB",              &c.enabled, &c.rhib);
				add("Vehicles", "Kyak",              &c.enabled, &c.kyak);
				add("Vehicles", "Solo submarine",    &c.enabled, &c.solo_submarine);
				add("Vehicles", "Duo submarine",     &c.enabled, &c.duo_submarine);
				add("Vehicles", "Minicopter",        &c.enabled, &c.minicopter);
				add("Vehicles", "Scrap heli",        &c.enabled, &c.scrap_heli);
				add("Vehicles", "Attack heli",       &c.enabled, &c.attack_heli);
			}
			{
				auto& c = e.dropped_weapons;
				add("Dropped weapons", "Dropped rifles",    &c.enabled, &c.dropped_rifles);
				add("Dropped weapons", "Dropped snipers",   &c.enabled, &c.dropped_snipers);
				add("Dropped weapons", "Dropped SMGs",      &c.enabled, &c.dropped_smgs);
				add("Dropped weapons", "Dropped shotguns",  &c.enabled, &c.dropped_shotguns);
				add("Dropped weapons", "Dropped pistols",   &c.enabled, &c.dropped_pistols);
				add("Dropped weapons", "Dropped LMGs",      &c.enabled, &c.dropped_lmgs);
				add("Dropped weapons", "Dropped launchers", &c.enabled, &c.dropped_launchers);
				add("Dropped weapons", "Dropped bows",      &c.enabled, &c.dropped_bows);
				add("Dropped weapons", "Dropped melee",     &c.enabled, &c.dropped_melee);
			}
			{
				auto& c = e.dropped_items;
				add("Dropped items", "Throwables", &c.enabled, &c.throwables);
				add("Dropped items", "Tools",      &c.enabled, &c.tools);
				add("Dropped items", "Medical",    &c.enabled, &c.medical);
				add("Dropped items", "Ammo",       &c.enabled, &c.ammo);
				add("Dropped items", "Misc",       &c.enabled, &c.misc);
			}
			return t;
		}();

		std::string key;
		key.reserve(category.size() + 1 + type.size());
		key.append(category); key.push_back('|'); key.append(type);
		auto it = table.find(key);
		return (it != table.end()) ? &it->second : nullptr;
	}

	void on_render()
	{
		[[maybe_unused]] static bool s_bi_init = []{ bundle_icons::init(); return true; }();

		std::unique_lock<std::recursive_mutex> entity_lock(cache::cache_mut, std::try_to_lock);
		if (!entity_lock.owns_lock()) return;

		if (!game::is_in_game()) return;
		if (!game::impl::local_player) return;

		render_radar();
		render_bullet_tracers();

		bool& inventory_enabled = settings.visuals.screen.player_inventory;
		glm::vec2 screen_center = { ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f };

		cache::try_for_each_entity<rust::Entity>([&](rust::Entity* entity) {
			if (entity->type != EntityType::Player)
				return;

			bool is_target = inventory_enabled
				&& entity->is_alive()
				&& !entity->is_sleeping
				&& entity->base_address != game::impl::local_player->base_address;
			entity->inventory_anim.update(is_target);

			float alpha = entity->inventory_anim;
			if (alpha <= 0.0f) return;

			if (entity->belt_items.empty()) return;

			const auto& bbox = entity->bounding_box;

			const f32 item_size = 13.2f; // 40% smaller than 22px
			const glm::vec2 item_sz{ item_size, item_size };
			const f32 padding  = 1.2f;
			const f32 strip_pad = 1.8f;
			const std::size_t n = entity->belt_items.size();
			const f32 strip_w = (item_size * n) + (padding * (n > 0 ? n - 1 : 0)) + strip_pad * 2.0f;
			const f32 strip_h = item_size + strip_pad * 2.0f;

			constexpr f32 name_clearance = 14.0f;
			glm::vec2 anchor{};
			if (bbox.valid)
			{
				anchor = { bbox.position.x + bbox.size.x * 0.5f, bbox.position.y };
			}
			else
			{
				glm::vec2 head_screen{};
				if (!math::world_to_screen(entity->origin + glm::vec3{ 0.0f, 1.8f, 0.0f }, &head_screen))
					return;
				anchor = head_screen;
			}

			const glm::vec2 strip_pos = {
				anchor.x - strip_w * 0.5f,
				anchor.y - name_clearance - strip_h
			};

			const glm::vec2 first_item = strip_pos + glm::vec2{ strip_pad, strip_pad };
			for (std::size_t i = 0; i < n; ++i)
			{
				auto& item = entity->belt_items[i];
				glm::vec2 pos = first_item + glm::vec2{ i * (item_size + padding), 0.0f };

				bool drew_icon = false;
				if (item.texture)
				{
					if (item.texture->is_valid()) {
						if (auto srv = item.texture->get_srv())
						{
							render::add_image(srv, pos, item_sz, 0.0f, Color::white().scale_alpha(alpha));
							drew_icon = true;
						}
					}
				}

				if (!drew_icon && !item.shortname.empty())
				{
					std::string label = item.shortname.substr(0, 4);
					render::add_text(render::Fonts::Pixelmix10px, label,
						pos + item_sz * 0.5f,
						Color::white().scale_alpha(alpha),
						render::TextFlagsDropShadow,
						glm::vec2{ 0.5f, 0.5f });
				}

				if (item.is_active)
				{
					render::add_rect(pos, item_sz, Color::white().scale_alpha(alpha), 4.0f, 1.5f);
				}

				if (item.amount > 1)
				{
					std::string amt_str = (item.amount > 1000) ? std::to_string(item.amount / 1000) + xs("k") : std::to_string(item.amount);
					render::add_text(render::Fonts::Arial14px, amt_str,
						pos + glm::vec2{ item_size - 2.0f, item_size - 2.0f },
						Color::white().scale_alpha(alpha),
						render::TextFlagsDropShadow, glm::vec2{ 1.0f, 1.0f });
				}
			}
			});

		bool& entities_enabled = settings.visuals.entities.enabled;

		const bool debug_esp = settings.misc.general.debug_esp;
		const Color debug_color = debug_esp ? settings.misc.general.debug_esp_color : Color{};

		f32 closest_distance = 100.0f;
		std::string closest_prefab;
		struct TextStack {
			glm::vec2 pos;
			std::vector<std::pair<uptr, std::string>> names;
		};
		std::vector<TextStack> debug_stacks;

		cache::try_for_each_entity<rust::Entity>([&](rust::Entity* entity) {
			if (!entity->base_address)
				return;

			if (entity->base_address == game::impl::local_player->base_address)
				return;

			if (debug_esp && !entity->prefab_name.empty() && entity->origin.x != 0.0f) {
				if (entity->prefab_name.find(xs("subents")) != std::string::npos ||
					entity->prefab_name.find(xs("seats")) != std::string::npos)
					return;

				float dist3d = glm::distance(entity->origin, game::impl::local_player->origin);
				if (dist3d < 10.0f)
				{
					glm::vec2 out{};
					if (math::world_to_screen(entity->origin, &out)) {

						bool found_stack = false;
						for (auto& stack : debug_stacks) {
							if (glm::distance(stack.pos, out) < 10.0f) {
								stack.names.push_back(std::make_pair(entity->base_address, entity->prefab_name));
								found_stack = true;
								break;
							}
						}

						if (!found_stack) {
							debug_stacks.push_back({ out, { std::make_pair(entity->base_address, entity->prefab_name) } });
						}

						float dist = glm::distance(out, screen_center);
						if (dist < closest_distance) {
							closest_distance = dist;
							closest_prefab = entity->prefab_name;
						}
					}
				}
			}

			switch (entity->type)
			{
			case EntityType::Stone: render_as(xs("Ores & Collectibles"), xs("Stone"), entity); break;
			case EntityType::Sulfur: render_as(xs("Ores & Collectibles"), xs("Sulfur"), entity); break;
			case EntityType::Metal: render_as(xs("Ores & Collectibles"), xs("Metal"), entity); break;
			case EntityType::Hemp: render_as(xs("Ores & Collectibles"), xs("Hemp"), entity); break;
			case EntityType::Wood: render_as(xs("Ores & Collectibles"), xs("Wood"), entity); break;
			case EntityType::DieselFuel: render_as(xs("Ores & Collectibles"), xs("Diesel fuel"), entity); break;
			case EntityType::GreenKeycard: render_as(xs("Ores & Collectibles"), xs("Green keycard"), entity); break;
			case EntityType::BlueKeycard: render_as(xs("Ores & Collectibles"), xs("Blue keycard"), entity); break;
			case EntityType::RedKeycard: render_as(xs("Ores & Collectibles"), xs("Red keycard"), entity); break;

			case EntityType::EliteCrate: render_as(xs("Crates & Barrels"), xs("Elite crate"), entity); break;
			case EntityType::MilitaryCrate: render_as(xs("Crates & Barrels"), xs("Military crate"), entity); break;
			case EntityType::LockedCrate: render_as(xs("Crates & Barrels"), xs("Locked crate"), entity); break;
			case EntityType::AirDrop: render_as(xs("Crates & Barrels"), xs("Air drop"), entity); break;
			case EntityType::LootBarrel: render_as(xs("Crates & Barrels"), xs("Loot barrel"), entity); break;
			case EntityType::OilBarrel: render_as(xs("Crates & Barrels"), xs("Oil barrel"), entity); break;
			case EntityType::FoodCrate: render_as(xs("Crates & Barrels"), xs("Food crate"), entity); break;
			case EntityType::ToolCrate: render_as(xs("Crates & Barrels"), xs("Tool crate"), entity); break;
			case EntityType::SmallCrate: render_as(xs("Crates & Barrels"), xs("Small crate"), entity); break;
			case EntityType::HealthCrate: render_as(xs("Crates & Barrels"), xs("Health crate"), entity); break;
			case EntityType::SmallFoodCrate: render_as(xs("Crates & Barrels"), xs("Small food crate"), entity); break;
			case EntityType::VehicleParts: render_as(xs("Crates & Barrels"), xs("Vehicle parts"), entity); break;
			case EntityType::Crate: render_as(xs("Crates & Barrels"), xs("Crate"), entity); break;

			case EntityType::WoodenBox: render_as(xs("Deployables"), xs("Wooden box"), entity); break;
			case EntityType::LargeWoodBox: render_as(xs("Deployables"), xs("Large wood box"), entity); break;
			case EntityType::ToolCupboard: render_as(xs("Deployables"), xs("Tool cupboard"), entity); break;
			case EntityType::Workbench1Alt: render_as(xs("Deployables"), xs("Tier 1 workbench"), entity); break;
			case EntityType::Workbench2Alt: render_as(xs("Deployables"), xs("Tier 2 workbench"), entity); break;
			case EntityType::Workbench3Alt: render_as(xs("Deployables"), xs("Tier 3 workbench"), entity); break;
			case EntityType::RepairBench: render_as(xs("Deployables"), xs("Repair bench"), entity); break;
			case EntityType::ResearchTable: render_as(xs("Deployables"), xs("Research table"), entity); break;
			case EntityType::SmallStash: render_as(xs("Deployables"), xs("Small stash"), entity); break;
			case EntityType::SleepingBag: render_as(xs("Deployables"), xs("Sleeping bag"), entity); break;
			case EntityType::Furnace: render_as(xs("Deployables"), xs("Furnace"), entity); break;
			case EntityType::Locker: render_as(xs("Deployables"), xs("Locker"), entity); break;

			case EntityType::AutoTurret: render_as(xs("Traps & Turrets"), xs("Auto turret"), entity); break;
			case EntityType::FlameTurret: render_as(xs("Traps & Turrets"), xs("Flame turret"), entity); break;
			case EntityType::ShotgunTrap: render_as(xs("Traps & Turrets"), xs("Shotgun trap"), entity); break;

			case EntityType::Player: player(entity); break;
			case EntityType::Scientist: render_as(xs("NPCs & Animals"), xs("Scientist"), entity); break;
			case EntityType::Dweller: render_as(xs("NPCs & Animals"), xs("Dweller"), entity); break;
			case EntityType::Bear: render_as(xs("NPCs & Animals"), xs("Bear"), entity); break;
			case EntityType::Wolf: render_as(xs("NPCs & Animals"), xs("Wolf"), entity); break;
			case EntityType::Stag: render_as(xs("NPCs & Animals"), xs("Stag"), entity); break;
			case EntityType::Boar: render_as(xs("NPCs & Animals"), xs("Boar"), entity); break;
			case EntityType::Horse: render_as(xs("NPCs & Animals"), xs("Horse"), entity); break;
			case EntityType::Chicken: render_as(xs("NPCs & Animals"), xs("Chicken"), entity); break;
			case EntityType::Shark: render_as(xs("NPCs & Animals"), xs("Shark"), entity); break;
			case EntityType::Scarecrow: render_as(xs("NPCs & Animals"), xs("Scarecrow"), entity); break;
			case EntityType::Corpse: render_as(xs("NPCs & Animals"), xs("Corpse"), entity); break;

			case EntityType::Recycler: render_as(xs("Static & Monuments"), xs("Recycler"), entity); break;
			case EntityType::PhoneBooth: render_as(xs("Static & Monuments"), xs("Phone booth"), entity); break;
			case EntityType::FuseBox: render_as(xs("Static & Monuments"), xs("Fuse box"), entity); break;
			case EntityType::Refinery: render_as(xs("Static & Monuments"), xs("Refinery"), entity); break;
			case EntityType::Elevator: render_as(xs("Static & Monuments"), xs("Elevator"), entity); break;
			case EntityType::CarLift: render_as(xs("Static & Monuments"), xs("Car lift"), entity); break;
			case EntityType::ComputerStation: render_as(xs("Static & Monuments"), xs("Computer station"), entity); break;

			case EntityType::Motorbike: render_as(xs("Vehicles"), xs("Motorbike"), entity); break;
			case EntityType::SidecarMotorbike: render_as(xs("Vehicles"), xs("Sidecar motorbike"), entity); break;
			case EntityType::PedalBike: render_as(xs("Vehicles"), xs("Pedal bike"), entity); break;
			case EntityType::Snowmobile: render_as(xs("Vehicles"), xs("Snowmobile"), entity); break;
			case EntityType::TomahaSnowmobile: render_as(xs("Vehicles"), xs("Tomaha snowmobile"), entity); break;
			case EntityType::Tugboat: render_as(xs("Vehicles"), xs("Tugboat"), entity); break;
			case EntityType::Rowboat: render_as(xs("Vehicles"), xs("Rowboat"), entity); break;
			case EntityType::RHIB: render_as(xs("Vehicles"), xs("RHIB"), entity); break;
			case EntityType::Kyak: render_as(xs("Vehicles"), xs("Kyak"), entity); break;
			case EntityType::SoloSubmarine: render_as(xs("Vehicles"), xs("Solo submarine"), entity); break;
			case EntityType::DuoSubmarine: render_as(xs("Vehicles"), xs("Duo submarine"), entity); break;
			case EntityType::Minicopter: render_as(xs("Vehicles"), xs("Minicopter"), entity); break;
			case EntityType::ScrapHeli: render_as(xs("Vehicles"), xs("Scrap heli"), entity); break;
			case EntityType::AttackHeli: render_as(xs("Vehicles"), xs("Attack heli"), entity); break;

			case EntityType::DroppedRifle: render_as(xs("Dropped weapons"), xs("Dropped rifles"), entity); break;
			case EntityType::DroppedSniper: render_as(xs("Dropped weapons"), xs("Dropped snipers"), entity); break;
			case EntityType::DroppedSMG: render_as(xs("Dropped weapons"), xs("Dropped SMGs"), entity); break;
			case EntityType::DroppedShotgun: render_as(xs("Dropped weapons"), xs("Dropped shotguns"), entity); break;
			case EntityType::DroppedPistol: render_as(xs("Dropped weapons"), xs("Dropped pistols"), entity); break;
			case EntityType::DroppedLMG: render_as(xs("Dropped weapons"), xs("Dropped LMGs"), entity); break;
			case EntityType::DroppedLauncher: render_as(xs("Dropped weapons"), xs("Dropped launchers"), entity); break;
			case EntityType::DroppedBow: render_as(xs("Dropped weapons"), xs("Dropped bows"), entity); break;
			case EntityType::DroppedMelee:     render_as(xs("Dropped weapons"), xs("Dropped melee"), entity); break;
			case EntityType::DroppedThrowable: render_as(xs("Dropped items"), xs("Throwables"), entity); break;
			case EntityType::DroppedTool:      render_as(xs("Dropped items"), xs("Tools"),      entity); break;
			case EntityType::DroppedMedical:   render_as(xs("Dropped items"), xs("Medical"),    entity); break;
			case EntityType::DroppedAmmo:      render_as(xs("Dropped items"), xs("Ammo"),       entity); break;
			case EntityType::DroppedMisc:      render_as(xs("Dropped items"), xs("Misc"),       entity); break;
			}

			});

		for (const auto& stack : debug_stacks) {
			float y_offset = 0.0f;
			for (const auto& name : stack.names) {
				std::string fmt = std::vformat(xs("{:#x} {}"), std::make_format_args(name.first, name.second));
				render::add_text(render::Fonts::Arial14px, fmt, stack.pos + glm::vec2{ 0.0f, y_offset }, debug_color, render::TextFlagsDropShadow, glm::vec2{ 0.5f });
				y_offset += 14.0f;
			}
		}

		if (debug_esp && !closest_prefab.empty()) {
			static std::string last_closest = xs("");
			if (closest_prefab != last_closest) {
				DBG("{}", std::vformat(xs("Closest prefab: {}"), std::make_format_args(closest_prefab)));
				last_closest = closest_prefab;
			}
		}
	}

	void render_bullet_tracers()
	{
		bool& enabled = settings.visuals.local.bullet_tracers;
		if (!enabled || !game::impl::local_player) return;

		const Color picker_color = settings.visuals.local.bullet_tracers_color;

		struct CompletedTracer { std::vector<glm::vec3> pts; DWORD complete_tick; };
		static std::unordered_map<uptr, std::vector<glm::vec3>> g_active;
		static std::vector<CompletedTracer>                     g_completed;
		static DWORD g_last_proj_walk = 0;

		const DWORD now = GetTickCount();
		constexpr DWORD lifetime_ms     = 3000;
		constexpr float sample_min_dist = 0.05f;

		auto sane = [](const glm::vec3& v) {
			return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z) &&
			       (v.x != 0.0f || v.y != 0.0f || v.z != 0.0f);
		};

		const uptr local_addr = game::impl::local_player->base_address;

		std::unordered_set<uptr> alive;
		if (now - g_last_proj_walk >= 16)
		{
			g_last_proj_walk = now;
			const uptr list_component = memory::read<uptr>(
				game::impl::game_assembly + offsets::ListComponent_Projectile::ListComponent_C);
			const uptr static_fields = list_component
				? memory::read<uptr>(list_component + offsets::ListComponent_Projectile::static_fields) : 0;
			const uptr instance = static_fields
				? memory::read<uptr>(static_fields + offsets::ListComponent_Projectile::instance) : 0;
			const uptr listhashset = instance
				? memory::read<uptr>(instance + offsets::ListComponent_Projectile::buffer) : 0;
			const i32 count = listhashset
				? memory::read<i32>(listhashset + offsets::ListComponent_Projectile::count) : 0;
			const uptr values_array = (listhashset && count > 0 && count <= 2048)
				? memory::read<uptr>(listhashset + offsets::ListComponent_Projectile::values_array) : 0;

			if (values_array)
			{
				alive.reserve(count);
				for (i32 i = 0; i < count; ++i)
				{
					const uptr proj = memory::read<uptr>(values_array + 0x20 + i * 0x8);
					if (!proj) continue;

					const uptr owner = memory::read<uptr>(proj + offsets::Projectile::ownerPlayer);
					if (owner != local_addr) continue;

					alive.insert(proj);

					glm::vec3 pos = memory::read<glm::vec3>(proj + offsets::Projectile::positionDirect);
					if (!sane(pos))
						pos = memory::read<glm::vec3>(proj + offsets::Projectile::currentPosition);
					if (!sane(pos)) continue;

					auto& path = g_active[proj];
					if (path.empty() || glm::distance(path.back(), pos) > sample_min_dist)
					{
						path.push_back(pos);
						if (path.size() > 512) path.erase(path.begin());
					}
				}
			}
		}
		else
		{
			alive.reserve(g_active.size());
			for (auto& kv : g_active) alive.insert(kv.first);
		}

		for (auto it = g_active.begin(); it != g_active.end(); )
		{
			if (alive.count(it->first)) { ++it; continue; }
			if (it->second.size() >= 2)
				g_completed.push_back({ std::move(it->second), now });
			it = g_active.erase(it);
		}

		g_completed.erase(std::remove_if(g_completed.begin(), g_completed.end(),
			[&](const CompletedTracer& ct) { return now - ct.complete_tick >= lifetime_ms; }),
			g_completed.end());

		auto draw_path = [&](const std::vector<glm::vec3>& pts, float alpha_mult) {
			if (pts.size() < 2) return;
			const Color c = picker_color.scale_alpha(alpha_mult);
			for (size_t i = 1; i < pts.size(); ++i)
			{
				glm::vec2 a{}, b{};
				if (!math::world_to_screen(pts[i - 1], &a)) continue;
				if (!math::world_to_screen(pts[i],     &b)) continue;
				render::add_line(a, b, c, 1.5f);
			}
		};

		for (const auto& kv : g_active) draw_path(kv.second, 1.0f);
		for (const auto& ct : g_completed)
		{
			const float age_frac = float(now - ct.complete_tick) / float(lifetime_ms);
			draw_path(ct.pts, 1.0f - age_frac);
		}

		if (g_active.size()    > 50)  g_active.clear();
		if (g_completed.size() > 30)
			g_completed.erase(g_completed.begin(), g_completed.begin() + 10);
	}

	void render_radar()
	{
		bool& enabled = settings.visuals.radar.enabled;
		if (!enabled)
			return;

		if (!game::impl::local_player)
			return;

		const f32 radar_radius = settings.visuals.radar.size;
		const f32 radar_range = settings.visuals.radar.range;
		const f32 dot_size = 3.0f;
		const f32 background_opacity = 0.6f;

		const glm::vec2 position{ 25.0f, 25.0f + 50.0f };
		const glm::vec2 center = position + radar_radius;

		render::add_circle_shadow(center, radar_radius, Color::black(), 25.0f);
		render::add_circle_filled(center, radar_radius, Color::black().scale_alpha(background_opacity));

		render::add_rect_filled(position + glm::vec2{ radar_radius, 0.0f }, glm::vec2{ 1.0f, radar_radius * 2.0f }, Color::black().scale_alpha(0.5f));
		render::add_rect_filled(position + glm::vec2{ 0.0f, radar_radius }, glm::vec2{ radar_radius * 2.0f, 1.0f }, Color::black().scale_alpha(0.5f));

		const f32 fov = settings.visuals.local.field_of_view;
		const f32 aspect_ratio = winapp::impl::window_size.x / winapp::impl::window_size.y;
		const f32 horizontal_fov = 2.0f * std::atan(std::tan(glm::radians(fov) * 0.5f) * aspect_ratio);

		const glm::vec2 left_fov_dir = { std::sin(-horizontal_fov * 0.5f), -std::cos(-horizontal_fov * 0.5f) };
		const glm::vec2 right_fov_dir = { std::sin(horizontal_fov * 0.5f), -std::cos(horizontal_fov * 0.5f) };

		render::add_line(center, center + left_fov_dir * radar_radius, Color::white().scale_alpha(0.3f));
		render::add_line(center, center + right_fov_dir * radar_radius, Color::white().scale_alpha(0.3f));

		render::add_circle(center, radar_radius, Color::black().scale_alpha(0.5f));

		const glm::vec3 local_pos = game::impl::local_player->origin;
		const f32 local_yaw = game::impl::local_player->view_angles.y;

		bool players_enabled = settings.visuals.players.enabled;
		bool entities_enabled = settings.visuals.entities.enabled;

		cache::try_for_each_entity<rust::Entity>([&](rust::Entity* entity) {
			if (!entity->base_address || entity->base_address == game::impl::local_player->base_address)
				return;

			bool should_render = false;
			Color dot_color = Color::white();

		if (entity->type == EntityType::Player) {
			if (!players_enabled) return;

			bool& radar_enabled = settings.visuals.players.include_in_radar;
			if (radar_enabled) {
				should_render = true;
				dot_color = settings.visuals.players.bounding_box_color;
			}
		}
		else {
			if (settings.visuals.radar.players_only) return;

			std::string category, name;
				switch (entity->type)
				{
				case EntityType::Stone: category = xs("Ores & Collectibles"); name = xs("Stone"); break;
				case EntityType::Sulfur: category = xs("Ores & Collectibles"); name = xs("Sulfur"); break;
				case EntityType::Metal: category = xs("Ores & Collectibles"); name = xs("Metal"); break;
				case EntityType::Hemp: category = xs("Ores & Collectibles"); name = xs("Hemp"); break;
				case EntityType::Wood: category = xs("Ores & Collectibles"); name = xs("Wood"); break;
				case EntityType::DieselFuel: category = xs("Ores & Collectibles"); name = xs("Diesel fuel"); break;
				case EntityType::GreenKeycard: category = xs("Ores & Collectibles"); name = xs("Green keycard"); break;
				case EntityType::BlueKeycard: category = xs("Ores & Collectibles"); name = xs("Blue keycard"); break;
				case EntityType::RedKeycard: category = xs("Ores & Collectibles"); name = xs("Red keycard"); break;

				case EntityType::EliteCrate: category = xs("Crates & Barrels"); name = xs("Elite crate"); break;
				case EntityType::MilitaryCrate: category = xs("Crates & Barrels"); name = xs("Military crate"); break;
				case EntityType::LockedCrate: category = xs("Crates & Barrels"); name = xs("Locked crate"); break;
				case EntityType::AirDrop: category = xs("Crates & Barrels"); name = xs("Air drop"); break;
				case EntityType::LootBarrel: category = xs("Crates & Barrels"); name = xs("Loot barrel"); break;
				case EntityType::OilBarrel: category = xs("Crates & Barrels"); name = xs("Oil barrel"); break;
				case EntityType::FoodCrate: category = xs("Crates & Barrels"); name = xs("Food crate"); break;
				case EntityType::ToolCrate: category = xs("Crates & Barrels"); name = xs("Tool crate"); break;
				case EntityType::SmallCrate: category = xs("Crates & Barrels"); name = xs("Small crate"); break;
				case EntityType::HealthCrate: category = xs("Crates & Barrels"); name = xs("Health crate"); break;
				case EntityType::SmallFoodCrate: category = xs("Crates & Barrels"); name = xs("Small food crate"); break;
				case EntityType::VehicleParts: category = xs("Crates & Barrels"); name = xs("Vehicle parts"); break;
				case EntityType::Crate: category = xs("Crates & Barrels"); name = xs("Crate"); break;

				case EntityType::WoodenBox: category = xs("Deployables"); name = xs("Wooden box"); break;
				case EntityType::LargeWoodBox: category = xs("Deployables"); name = xs("Large wood box"); break;
				case EntityType::ToolCupboard: category = xs("Deployables"); name = xs("Tool cupboard"); break;
				case EntityType::Workbench1Alt: category = xs("Deployables"); name = xs("Tier 1 workbench"); break;
				case EntityType::Workbench2Alt: category = xs("Deployables"); name = xs("Tier 2 workbench"); break;
				case EntityType::Workbench3Alt: category = xs("Deployables"); name = xs("Tier 3 workbench"); break;
				case EntityType::RepairBench: category = xs("Deployables"); name = xs("Repair bench"); break;
				case EntityType::ResearchTable: category = xs("Deployables"); name = xs("Research table"); break;
				case EntityType::SmallStash: category = xs("Deployables"); name = xs("Small stash"); break;
				case EntityType::SleepingBag: category = xs("Deployables"); name = xs("Sleeping bag"); break;
				case EntityType::Furnace: category = xs("Deployables"); name = xs("Furnace"); break;
				case EntityType::Locker: category = xs("Deployables"); name = xs("Locker"); break;

				case EntityType::AutoTurret: category = xs("Traps & Turrets"); name = xs("Auto turret"); break;
				case EntityType::FlameTurret: category = xs("Traps & Turrets"); name = xs("Flame turret"); break;
				case EntityType::ShotgunTrap: category = xs("Traps & Turrets"); name = xs("Shotgun trap"); break;

				case EntityType::Scientist: category = xs("NPCs & Animals"); name = xs("Scientist"); break;
				case EntityType::Dweller: category = xs("NPCs & Animals"); name = xs("Dweller"); break;
				case EntityType::Bear: category = xs("NPCs & Animals"); name = xs("Bear"); break;
				case EntityType::Wolf: category = xs("NPCs & Animals"); name = xs("Wolf"); break;
				case EntityType::Stag: category = xs("NPCs & Animals"); name = xs("Stag"); break;
				case EntityType::Boar: category = xs("NPCs & Animals"); name = xs("Boar"); break;
				case EntityType::Horse: category = xs("NPCs & Animals"); name = xs("Horse"); break;
				case EntityType::Chicken: category = xs("NPCs & Animals"); name = xs("Chicken"); break;
				case EntityType::Shark: category = xs("NPCs & Animals"); name = xs("Shark"); break;
				case EntityType::Scarecrow: category = xs("NPCs & Animals"); name = xs("Scarecrow"); break;
				case EntityType::Corpse: category = xs("NPCs & Animals"); name = xs("Corpse"); break;

				case EntityType::Recycler: category = xs("Static & Monuments"); name = xs("Recycler"); break;
				case EntityType::PhoneBooth: category = xs("Static & Monuments"); name = xs("Phone booth"); break;
				case EntityType::FuseBox: category = xs("Static & Monuments"); name = xs("Fuse box"); break;
				case EntityType::Refinery: category = xs("Static & Monuments"); name = xs("Refinery"); break;
				case EntityType::Elevator: category = xs("Static & Monuments"); name = xs("Elevator"); break;
				case EntityType::CarLift: category = xs("Static & Monuments"); name = xs("Car lift"); break;
				case EntityType::ComputerStation: category = xs("Static & Monuments"); name = xs("Computer station"); break;

				case EntityType::Motorbike: category = xs("Vehicles"); name = xs("Motorbike"); break;
				case EntityType::SidecarMotorbike: category = xs("Vehicles"); name = xs("Sidecar motorbike"); break;
				case EntityType::PedalBike: category = xs("Vehicles"); name = xs("Pedal bike"); break;
				case EntityType::Snowmobile: category = xs("Vehicles"); name = xs("Snowmobile"); break;
				case EntityType::TomahaSnowmobile: category = xs("Vehicles"); name = xs("Tomaha snowmobile"); break;
				case EntityType::Tugboat: category = xs("Vehicles"); name = xs("Tugboat"); break;
				case EntityType::Rowboat: category = xs("Vehicles"); name = xs("Rowboat"); break;
				case EntityType::RHIB: category = xs("Vehicles"); name = xs("RHIB"); break;
				case EntityType::Kyak: category = xs("Vehicles"); name = xs("Kyak"); break;
				case EntityType::SoloSubmarine: category = xs("Vehicles"); name = xs("Solo submarine"); break;
				case EntityType::DuoSubmarine: category = xs("Vehicles"); name = xs("Duo submarine"); break;
				case EntityType::Minicopter: category = xs("Vehicles"); name = xs("Minicopter"); break;
				case EntityType::ScrapHeli: category = xs("Vehicles"); name = xs("Scrap heli"); break;
				case EntityType::AttackHeli: category = xs("Vehicles"); name = xs("Attack heli"); break;

				case EntityType::DroppedRifle: category = xs("Dropped weapons"); name = xs("Dropped rifles"); break;
				case EntityType::DroppedSniper: category = xs("Dropped weapons"); name = xs("Dropped snipers"); break;
				case EntityType::DroppedSMG: category = xs("Dropped weapons"); name = xs("Dropped SMGs"); break;
				case EntityType::DroppedShotgun: category = xs("Dropped weapons"); name = xs("Dropped shotguns"); break;
				case EntityType::DroppedPistol: category = xs("Dropped weapons"); name = xs("Dropped pistols"); break;
				case EntityType::DroppedLMG: category = xs("Dropped weapons"); name = xs("Dropped LMGs"); break;
				case EntityType::DroppedLauncher: category = xs("Dropped weapons"); name = xs("Dropped launchers"); break;
				case EntityType::DroppedBow: category = xs("Dropped weapons"); name = xs("Dropped bows"); break;
				case EntityType::DroppedMelee: category = xs("Dropped weapons"); name = xs("Dropped melee"); break;
				case EntityType::DroppedThrowable: category = xs("Dropped items"); name = xs("Throwables"); break;
				case EntityType::DroppedTool:      category = xs("Dropped items"); name = xs("Tools");      break;
				case EntityType::DroppedMedical:   category = xs("Dropped items"); name = xs("Medical");    break;
				case EntityType::DroppedAmmo:      category = xs("Dropped items"); name = xs("Ammo");       break;
				case EntityType::DroppedMisc:      category = xs("Dropped items"); name = xs("Misc");       break;
				}

				if (!category.empty()) {
					if (!entities_enabled) return;

					if (const EntityLookup* lu = lookup_entity(category, name); lu && lu->type_settings) {
						if (lu->type_settings->include_in_radar) {
							should_render = true;
							dot_color = lu->type_settings->color;
						}
					}
				}
			}

			if (should_render) {
				const f32 dx = entity->origin.x - local_pos.x;
				const f32 dz = entity->origin.z - local_pos.z;

				const f32 dist = std::sqrt(dx * dx + dz * dz);
				if (dist > radar_range)
					return;

				const f32 yaw_rad = glm::radians(local_yaw);
				const f32 cos_y = std::cos(yaw_rad);
				const f32 sin_y = std::sin(yaw_rad);

				const f32 rotated_x = dx * cos_y - dz * sin_y;
				const f32 rotated_z = dx * sin_y + dz * cos_y;

				const f32 scale = radar_radius / radar_range;
				const glm::vec2 point_pos = center + glm::vec2{ rotated_x * scale, -rotated_z * scale };

				render::add_circle_filled(point_pos, dot_size, dot_color);
				render::add_circle(point_pos, dot_size, Color::black().scale_alpha(0.5f), 1.0f);
			}
			});
	}

	void render_as(std::string category, std::string type, rust::Entity* entity)
	{
		struct Snap {
			bool  master       = false;
			bool  category_en  = false;
			bool  enabled      = false;
			i32   max_distance = 0;
			Color color;
			std::chrono::steady_clock::time_point last_refresh{};
		};
		static std::unordered_map<std::string, Snap> s_cache;
		const auto now = std::chrono::steady_clock::now();

		std::string key;
		key.reserve(category.size() + 1 + type.size());
		key.append(category); key.push_back('|'); key.append(type);

		Snap& snap = s_cache[key];
		const bool first_seen = snap.last_refresh.time_since_epoch().count() == 0;
		const bool stale = !first_seen &&
			std::chrono::duration_cast<std::chrono::milliseconds>(now - snap.last_refresh).count() >= 250;
		if (first_seen || stale) {
			snap.last_refresh = now;
			const EntityLookup* lu = lookup_entity(category, type);
			snap.master      = settings.visuals.entities.enabled;
			snap.category_en = (lu && lu->category_enabled) ? *lu->category_enabled : false;
			snap.enabled     = (lu && lu->type_settings) ? lu->type_settings->enabled : false;
			if (snap.enabled && lu && lu->type_settings) {
				snap.max_distance = lu->type_settings->max_distance;
				snap.color        = lu->type_settings->color;
			}
		}
		if (!snap.master || !snap.category_en || !snap.enabled)
			return;

		if (entity->origin.x == 0.0f &&
			entity->origin.y == 0.0f &&
			entity->origin.z == 0.0f)
			return;

		const i32 distance = static_cast<i32>(glm::distance(game::impl::local_player->origin, entity->origin));
		if (distance > snap.max_distance)
			return;

		glm::vec2 out{};
		if (!math::world_to_screen(entity->origin, &out))
			return;

		texture_t* icon = bundle_icons::for_category_type(category, type);
		if (icon && icon->is_valid()) {
			if (auto srv = icon->get_srv()) {
				constexpr float kIconSize = 12.0f;
				const glm::vec2 icon_pos{ out.x - kIconSize * 0.5f, out.y - kIconSize - 2.0f };
				render::add_image(srv, icon_pos, glm::vec2{ kIconSize }, 0.0f, snap.color);
			}
		}

		const std::string fmt = std::vformat(xs("{} [{}m]"), std::make_format_args(entity->name, distance));
		render::add_text(render::Fonts::Pixelmix10px, fmt, out, snap.color, render::TextFlagsDropShadow, glm::vec2{ 0.5f });
	}

	void name(rust::Entity* entity)
	{
		struct NameSnap {
			bool  name_enabled  = false;
			Color color;
			i32   gui_flags     = 0;
			bool  avatar_on     = false;
		};
		static NameSnap s_n;
		static auto s_n_last = std::chrono::steady_clock::time_point{};
		auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - s_n_last).count() >= 250) {
			s_n_last = now;
			s_n.name_enabled = settings.visuals.players.name;
			if (s_n.name_enabled) {
				s_n.color      = settings.visuals.players.name_color;
				s_n.gui_flags  = settings.visuals.flags | settings.visuals.players.flags;
				s_n.avatar_on  = settings.visuals.players.name_show_avatar;
			}
		}
		if (!s_n.name_enabled)
			return;

		Color color = s_n.color;
		if (entity->is_wounded) {
			color = Color(255, 220, 0, color.a);
		}
		if (features::aimbot::current_target == entity)
		{
			const Color picked_color = color;
			color = Color(255, 215, 60, picked_color.a);
		}
		glm::vec2 head_screen{};
		if (entity->bounding_box.valid) {
			head_screen = glm::vec2{
				entity->bounding_box.position.x + entity->bounding_box.size.x * 0.5f,
				entity->bounding_box.position.y
			};
		}
		else if (!math::world_to_screen(entity->origin + glm::vec3{ 0.0f, 1.8f, 0.0f }, &head_screen))
			return;

		static i32 s_overlay_flags = 0;
		static auto s_overlay_last = std::chrono::steady_clock::time_point{};
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - s_overlay_last).count() >= 50) {
			s_overlay_last = now;
			i32 of = 0;
			for (int i = 0; i < 5; ++i)
				if (settings.visuals.players.extra_player_flags[i]) of |= (1 << i);
			s_overlay_flags = of;
		}
		std::string name_str = entity->name;

		const glm::vec2 text_size = render::get_text_size(render::Fonts::Pixelmix10px, name_str);

		glm::vec2 rect_size{ text_size.y };

		const bool avatar = s_n.avatar_on;

		const bool entity_ready = entity->avatar_status == rust::Entity::AvatarStatus::Ready && entity->avatar_texture.is_valid();
		ID3D11ShaderResourceView* avatar_srv = nullptr;
		if (avatar) {
			avatar_srv = entity_ready
				? entity->avatar_texture.get_srv()
				: render::steam_avatar.get_srv();
		}

		if (avatar_srv)
		{
			const float full_width = rect_size.x + text_size.x + 4.0f;
			render::add_image(
				avatar_srv,
				head_screen + glm::vec2{ -full_width * 0.5f, -3.0f + -rect_size.y },
				rect_size,
				0.0f,
				Color::white(),
				4.0f);
			render::add_text(render::Fonts::Pixelmix10px, name_str, head_screen + glm::vec2{ -full_width * 0.5f + (rect_size.x + 4.0f), -3.0f }, color, render::TextFlagsDropShadow, glm::vec2{ 0.0f, 1.0f });
		}
		else {
			render::add_text(render::Fonts::Pixelmix10px, name_str, head_screen + glm::vec2{ 0.0f, -3.0f }, color, render::TextFlagsDropShadow, glm::vec2{ 0.5f, 1.0f });
		}
	}

	void item_name(rust::Entity* entity)
	{
		struct ItemSnap { bool name_on=false, icon_on=false; Color name_color, icon_color; };
		static ItemSnap s_is;
		static auto s_is_last = std::chrono::steady_clock::time_point{};
		const auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - s_is_last).count() >= 250) {
			s_is_last = now;
			s_is.name_on    = settings.visuals.players.item_name;
			s_is.icon_on    = settings.visuals.players.item_icon;
			if (s_is.name_on) s_is.name_color = settings.visuals.players.item_name_color;
			if (s_is.icon_on) s_is.icon_color = settings.visuals.players.item_icon_color;
		}

		if (!s_is.name_on && !s_is.icon_on)
			return;

		glm::vec2 feet_screen{};
		if (!math::world_to_screen(entity->origin, &feet_screen))
			return;
		float y_offset = 2.0f;

		if (s_is.name_on && !entity->item_name.empty())
		{
			render::add_text(render::Fonts::Pixelmix10px, entity->item_name, feet_screen + glm::vec2{ 0.0f, y_offset }, s_is.name_color, render::TextFlagsDropShadow, glm::vec2{ 0.5f, 0.0f });
			y_offset += 12.0f;
		}

		if (s_is.icon_on && entity->item_texture && entity->item_texture->is_valid())
		{
			if (auto srv = entity->item_texture->get_srv())
			{
				const glm::vec2 icon_sz{ 32.0f, 20.0f };
				render::add_image(srv, feet_screen + glm::vec2{ -icon_sz.x * 0.5f, y_offset }, icon_sz, 0.0f, s_is.icon_color);
			}
		}
	}

	void skeleton(rust::Entity* entity)
	{
		struct SkSnap {
			bool  enabled=false, head_circle=false, rounded=false, outline=true;
			bool  vischeck_on=false, per_bone_vis=false;
			Color user_color, vis_color, inv_color;
		};
		static SkSnap s_sk;
		static auto s_sk_last = std::chrono::steady_clock::time_point{};
		const auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - s_sk_last).count() >= 250) {
			s_sk_last = now;
			s_sk.enabled = settings.visuals.players.skeleton;
			if (s_sk.enabled) {
				s_sk.user_color   = settings.visuals.players.skeleton_color;
				s_sk.head_circle  = settings.visuals.players.head_circle;
				s_sk.rounded      = settings.visuals.players.skeleton_rounded;
				s_sk.outline      = settings.visuals.players.skeleton_outline;
				s_sk.vischeck_on  = settings.visuals.players.vischeck;
				s_sk.per_bone_vis = s_sk.vischeck_on && settings.visuals.players.vis_per_bone_skeleton;
				s_sk.vis_color    = settings.visuals.players.vis_visible_color;
				s_sk.inv_color    = settings.visuals.players.vis_invisible_color;
			}
		}
		if (!s_sk.enabled) return;

		Color user_color  = s_sk.user_color;
		const bool  head_circle = s_sk.head_circle;
		const bool  rounded     = s_sk.rounded;
		const bool  outline     = s_sk.outline;
		if (entity->is_wounded) user_color = Color(255, 220, 0, user_color.a);
		Color color = user_color;

		const bool vischeck_on  = s_sk.vischeck_on;
		const bool per_bone_vis = s_sk.per_bone_vis;
		const Color vis_color   = s_sk.vis_color;
		const Color inv_color   = s_sk.inv_color;

		struct VisCache {
			std::chrono::steady_clock::time_point updated{};
			std::unordered_map<rust::BoneList, bool> bones;
		};
		static std::unordered_map<rust::Entity*, VisCache> s_vis_cache;
		const auto now_vis = std::chrono::steady_clock::now();
		VisCache& cache = s_vis_cache[entity];
		const bool refresh = std::chrono::duration_cast<std::chrono::milliseconds>(now_vis - cache.updated).count() > 100;

		auto is_bone_visible = [&](rust::BoneList id, const glm::vec3& world_pos) -> bool {
			if (!per_bone_vis) return true;
			auto it = cache.bones.find(id);
			if (it != cache.bones.end() && !refresh) return it->second;
			bool v = true;
			if (features::physx::is_ready()
				&& game::impl::local_player && entity != game::impl::local_player)
			{
				glm::vec3 eye_pos{};
				if (game::impl::local_player->bones) {
					auto eye = game::impl::local_player->bones->find(rust::BoneList::head);
					if (eye != game::impl::local_player->bones->end())
						eye_pos = eye->second.position;
				}
				if (eye_pos.x == 0.0f && eye_pos.y == 0.0f && eye_pos.z == 0.0f
					&& game::impl::local_player->has_origin)
					eye_pos = game::impl::local_player->origin + glm::vec3{ 0.0f, 1.55f, 0.0f };
				if (eye_pos.x != 0.0f || eye_pos.y != 0.0f || eye_pos.z != 0.0f) {
					
					v = features::physx::is_visible(eye_pos, world_pos);
				}
			}
			cache.bones[id] = v;
			cache.updated = now_vis;
			return v;
		};

		using B = rust::BoneList;
		static constexpr B kTrunk[] = { B::pelvis, B::spine1, B::spine2, B::spine3, B::spine4, B::neck, B::head };
		static constexpr B kLArm[]  = { B::neck,   B::l_clavicle, B::l_upperarm, B::l_forearm, B::l_hand };
		static constexpr B kRArm[]  = { B::neck,   B::r_clavicle, B::r_upperarm, B::r_forearm, B::r_hand };
		static constexpr B kLLeg[]  = { B::pelvis, B::l_hip, B::l_knee, B::l_foot };
		static constexpr B kRLeg[]  = { B::pelvis, B::r_hip, B::r_knee, B::r_foot };
		struct ChainSpan { const B* data; std::size_t size; };
		static constexpr ChainSpan kJointChains[] = {
			{ kTrunk, std::size(kTrunk) },
			{ kLArm,  std::size(kLArm)  },
			{ kRArm,  std::size(kRArm)  },
			{ kLLeg,  std::size(kLLeg)  },
			{ kRLeg,  std::size(kRLeg)  },
		};

		const bool skip_head_neck = head_circle;

		auto catmull = [](const glm::vec3& p0, const glm::vec3& p1,
			const glm::vec3& p2, const glm::vec3& p3, float t) -> glm::vec3
		{
			const float t2 = t * t;
			const float t3 = t2 * t;
			return 0.5f * (
				(p1 * 2.0f) +
				(p2 - p0) * t +
				(p0 * 2.0f - p1 * 5.0f + p2 * 4.0f - p3) * t2 +
				(p1 * 3.0f - p0 - p2 * 3.0f + p3) * t3
			);
		};

		const bool smooth_now = rounded && entity->distance <= 25.0f;
		constexpr int kSubdivisions = 5;

		std::vector<glm::vec3> world_pts;
		std::vector<glm::vec2> screen_pts;
		world_pts.reserve(8);
		screen_pts.reserve(40);

		std::array<B, 8> chain_bone_ids_arr{};
		std::size_t      chain_bone_ids_n = 0;
		for (std::size_t chain_idx = 0; chain_idx < std::size(kJointChains); ++chain_idx)
		{
			const auto& chain = kJointChains[chain_idx];
			if (!entity->bones) return;

			world_pts.clear();
			chain_bone_ids_n = 0;
			for (std::size_t k = 0; k < chain.size; ++k)
			{
				const B id = chain.data[k];
				auto it = entity->bones->find(id);
				if (it != entity->bones->end() && it->second.visible) {
					world_pts.push_back(it->second.position);
					if (chain_bone_ids_n < chain_bone_ids_arr.size())
						chain_bone_ids_arr[chain_bone_ids_n++] = id;
				}
			}

			if (world_pts.size() < 2)
				continue;

			screen_pts.clear();

			if (smooth_now && world_pts.size() >= 4)
			{
				glm::vec3 prev_emit = world_pts[0];
				glm::vec2 sp;
				if (math::world_to_screen(prev_emit, &sp)) screen_pts.push_back(sp);

				for (size_t i = 0; i + 1 < world_pts.size(); ++i)
				{
					const glm::vec3& p0 = (i == 0)                       ? world_pts[i]     : world_pts[i - 1];
					const glm::vec3& p1 = world_pts[i];
					const glm::vec3& p2 = world_pts[i + 1];
					const glm::vec3& p3 = (i + 2 >= world_pts.size())    ? world_pts[i + 1] : world_pts[i + 2];

					for (int j = 1; j <= kSubdivisions; ++j)
					{
						const float t = static_cast<float>(j) / static_cast<float>(kSubdivisions);
						glm::vec3 w = catmull(p0, p1, p2, p3, t);
						if (math::world_to_screen(w, &sp))
							screen_pts.push_back(sp);
					}
				}
			}
			else
			{
				glm::vec2 sp;
				for (const auto& w : world_pts)
				{
					if (math::world_to_screen(w, &sp))
						screen_pts.push_back(sp);
				}
			}

			const bool is_trunk = (chain_idx == 0);
			const size_t end = screen_pts.size();
			const size_t last_drawn = (is_trunk && skip_head_neck && end > 1) ? end - 1 : end;

			const bool smoothed_now = (smooth_now && world_pts.size() >= 4);
			const size_t subdiv_per_seg = smoothed_now ? kSubdivisions : 1;

			for (size_t i = 1; i < last_drawn; ++i) {
				Color seg_color = color;
				if (per_bone_vis) {
					const size_t world_idx_b = std::min(((i - 1) + subdiv_per_seg / 2) / subdiv_per_seg, world_pts.size() - 1);
					const size_t world_idx_a = (world_idx_b == 0) ? 0 : world_idx_b - 1;
					const B id_a = chain_bone_ids_arr[world_idx_a];
					const B id_b = chain_bone_ids_arr[world_idx_b];
					const bool va = is_bone_visible(id_a, world_pts[world_idx_a]);
					const bool vb = is_bone_visible(id_b, world_pts[world_idx_b]);
					seg_color = (va || vb) ? vis_color : inv_color;
				}
				if (outline)
					render::add_line(screen_pts[i - 1], screen_pts[i], Color(0, 0, 0, seg_color.a), 2.0f);
				render::add_line(screen_pts[i - 1], screen_pts[i], seg_color, 1.0f);
			}
		}
	}

	void head_circle(rust::Entity* entity)
	{
		struct HCSnap { bool enabled=false; Color color; };
		static HCSnap s_hc;
		static auto s_hc_last = std::chrono::steady_clock::time_point{};
		const auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - s_hc_last).count() >= 250) {
			s_hc_last = now;
			s_hc.enabled = settings.visuals.players.head_circle;
			if (s_hc.enabled) s_hc.color = settings.visuals.players.head_circle_color;
		}
		if (!s_hc.enabled) return;

		if (!entity->bones) return;
		auto head_it = entity->bones->find(rust::BoneList::head);
		auto neck_it = entity->bones->find(rust::BoneList::neck);
		if (head_it == entity->bones->end() || !head_it->second.visible ||
			neck_it == entity->bones->end() || !neck_it->second.visible)
			return;

		glm::vec2 p_head{}, p_neck{};
		if (math::world_to_screen(head_it->second.position, &p_head) &&
			math::world_to_screen(neck_it->second.position, &p_neck))
		{
			Color c = s_hc.color;
			if (entity->is_wounded) c = Color(255, 220, 0, c.a);
			const float radius = glm::distance(p_head, p_neck);
			render::add_circle(p_head, radius, c, 1.5f);
		}
	}

	void view_line(rust::Entity* entity)
	{
		struct VLSnap { bool enabled=false; Color color; };
		static VLSnap s_vl;
		static auto s_vl_last = std::chrono::steady_clock::time_point{};
		const auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - s_vl_last).count() >= 250) {
			s_vl_last = now;
			s_vl.enabled = settings.visuals.players.view_line;
			if (s_vl.enabled) s_vl.color = settings.visuals.players.view_line_color;
		}
		if (!s_vl.enabled) return;

		if (!entity->bones) return;
		auto head_it = entity->bones->find(rust::BoneList::head);
		if (head_it == entity->bones->end() || !head_it->second.visible)
			return;

		glm::vec3 forward{};
		math::angle_vectors(entity->view_angles, &forward);

		glm::vec3 end_pos = head_it->second.position + (forward * 1.25f);

		glm::vec2 p_start{}, p_end{};
		if (math::world_to_screen(head_it->second.position, &p_start) &&
			math::world_to_screen(end_pos, &p_end))
		{
			Color c = s_vl.color;
			if (entity->is_wounded) c = Color(255, 220, 0, c.a);
			render::add_line(p_start, p_end, c, 1.5f);
		}
	}

	void bounding_box(rust::Entity* entity)
	{
		struct BBSnap {
			bool  box=false, gradient=false, fill=false, outline=true, corner=false;
			Color color, gradient_color, fill_color_top, fill_color_bottom;
		};
		static BBSnap s_bb;
		static auto s_bb_last = std::chrono::steady_clock::time_point{};
		const auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - s_bb_last).count() >= 250) {
			s_bb_last = now;
			s_bb.box = settings.visuals.players.bounding_box;
			if (s_bb.box) {
				s_bb.color             = settings.visuals.players.bounding_box_color;
				s_bb.outline           = settings.visuals.players.bounding_box_outline;
				s_bb.corner            = settings.visuals.players.bounding_box_corner;
				s_bb.gradient          = settings.visuals.players.bounding_box_gradient;
				s_bb.gradient_color    = settings.visuals.players.bounding_box_gradient_color;
				s_bb.fill              = settings.visuals.players.bounding_box_fill;
				s_bb.fill_color_top    = settings.visuals.players.bounding_box_fill_color_top;
				s_bb.fill_color_bottom = settings.visuals.players.bounding_box_fill_color_bottom;
			}
		}
		if (!s_bb.box) return;

		const auto& bbox = entity->bounding_box;
		if (bbox.size.x < 2.0f || bbox.size.y < 2.0f) return;

		Color color                    = s_bb.color;
		const bool   gradient          = s_bb.gradient;
		Color gradient_color           = s_bb.gradient_color;
		const bool   fill              = s_bb.fill;
		Color fill_color_top           = s_bb.fill_color_top;
		Color fill_color_bottom        = s_bb.fill_color_bottom;
		if (entity->is_wounded) {
			color             = Color(255, 220, 0, color.a);
			gradient_color    = Color(255, 220, 0, gradient_color.a);
			fill_color_top    = Color(255, 220, 0, fill_color_top.a);
			fill_color_bottom = Color(255, 220, 0, fill_color_bottom.a);
		}

		if (fill)
		{
			render::add_rect_gradient(bbox.position + glm::vec2{ 2.0f }, bbox.size - glm::vec2{ 4.0f }, render::GradientType::Vertical, fill_color_top, fill_color_bottom);
		}

		const Color outline_col{ 0, 0, 0, color.a };
		auto make_corner_box = [&](glm::vec2 pos, glm::vec2 size, Color col) {
			const f32 x = pos.x, y = pos.y, w = size.x, h = size.y;
			const f32 len = w * 0.3f;
			if (len < 2.0f) return;
			render::add_rect_filled({ x, y }, { len, 1.0f }, col);
			render::add_rect_filled({ x, y }, { 1.0f, len }, col);
			render::add_rect_filled({ x + w - len, y }, { len, 1.0f }, col);
			render::add_rect_filled({ x + w - 1.0f, y }, { 1.0f, len }, col);
			render::add_rect_filled({ x, y + h - 1.0f }, { len, 1.0f }, col);
			render::add_rect_filled({ x, y + h - len }, { 1.0f, len }, col);
			render::add_rect_filled({ x + w - len, y + h - 1.0f }, { len, 1.0f }, col);
			render::add_rect_filled({ x + w - 1.0f, y + h - len }, { 1.0f, len }, col);
		};

		if (s_bb.corner)
		{
			if (s_bb.outline)
			{
				make_corner_box(bbox.position - glm::vec2{ 1.0f }, bbox.size + glm::vec2{ 2.0f }, outline_col);
				make_corner_box(bbox.position + glm::vec2{ 1.0f }, bbox.size - glm::vec2{ 2.0f }, outline_col);
			}
			make_corner_box(bbox.position, bbox.size, color);
		}
		else if (gradient)
		{
			if (s_bb.outline)
			{
				render::add_rect(bbox.position - glm::vec2{ 1.0f }, bbox.size + glm::vec2{ 2.0f }, outline_col, 0.0f, 1.0f);
				render::add_rect(bbox.position + glm::vec2{ 1.0f }, bbox.size - glm::vec2{ 2.0f }, outline_col, 0.0f, 1.0f);
			}
			render::gradient_items(bbox.position, bbox.size, color, gradient_color, [&] {
				render::add_rect(bbox.position, bbox.size, color, 0.0f, 1.0f);
			}, 0.75f);
		}
		else
		{
			if (s_bb.outline)
			{
				render::add_rect(bbox.position - glm::vec2{ 1.0f }, bbox.size + glm::vec2{ 2.0f }, outline_col, 0.0f, 1.0f);
				render::add_rect(bbox.position + glm::vec2{ 1.0f }, bbox.size - glm::vec2{ 2.0f }, outline_col, 0.0f, 1.0f);
			}
			render::add_rect(bbox.position, bbox.size, color, 0.0f, 1.0f);
		}
	}

	void flags(rust::Entity* entity)
	{
		struct FlagsSnap { i32 flags = 0; };
		static FlagsSnap s_fl;
		static auto s_fl_last = std::chrono::steady_clock::time_point{};
		const auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - s_fl_last).count() >= 250) {
			s_fl_last = now;
			i32 f = 0;
			for (int i = 0; i < 5; ++i)
				if (settings.visuals.players.extra_player_flags[i]) f |= (1 << i);
			f |= settings.visuals.flags;
			f |= settings.visuals.players.flags;
			s_fl.flags = f;
		}
		const i32 flags = s_fl.flags;
		if (!flags) return;   // no flags requested ? skip everything below

		const bool flags_distance = flags & (1 << 0);
		const bool flags_sleeper  = flags & (1 << 1);
		const bool flags_wounded  = flags & (1 << 2);
		const bool flags_team_id  = flags & (1 << 3);
		const bool flags_ads      = flags & (1 << 4);

		std::array<std::string_view, 8> flag_opts_sv;
		std::array<std::string, 2>      flag_opts_own;   // for the formatted "Nm" / "TEAM N" strings
		std::size_t flag_opt_count = 0;
		std::size_t own_count      = 0;

		if (flags_distance) {
			const i32 distance_int = static_cast<i32>(entity->distance);
			flag_opts_own[own_count] = std::vformat(xs("{}m"), std::make_format_args(distance_int));
			flag_opts_sv[flag_opt_count++] = flag_opts_own[own_count++];
		}

		if (flags_sleeper && entity->is_sleeping)
			flag_opts_sv[flag_opt_count++] = xs("SLEEP");

		if (flags_wounded && entity->is_wounded)
			flag_opts_sv[flag_opt_count++] = xs("WOUNDED");

		if (flags_team_id)
		{
			if (entity->team == 0) {
				flag_opts_sv[flag_opt_count++] = xs("SOLO");
			} else if (game::impl::local_player && entity->team == game::impl::local_player->team) {
				flag_opts_sv[flag_opt_count++] = xs("TEAMMATE");
			} else {
				flag_opts_own[own_count] = std::vformat(xs("TEAM {}"), std::make_format_args(entity->team));
				flag_opts_sv[flag_opt_count++] = flag_opts_own[own_count++];
			}
		}

		if (flags_ads && (entity->is_aiming || entity->is_scoped))
			flag_opts_sv[flag_opt_count++] = xs("AIM");
		if (entity->is_crouching)
			flag_opts_sv[flag_opt_count++] = xs("CROUCH");
		if (entity->is_prone)
			flag_opts_sv[flag_opt_count++] = xs("PRONE");

		if (!flag_opt_count) return;

		auto pos  = entity->bounding_box.position;
		auto size = entity->bounding_box.size;

		float offset = -3.0f;
		for (std::size_t i = 0; i < flag_opt_count; ++i)
		{
			const std::string_view flag_text = flag_opts_sv[i];
			render::add_text(render::Fonts::Pixelmix10px, flag_text,
				pos + glm::vec2{ size.x + 3.0f, offset },
				Color::white(), render::TextFlagsOutline);
			offset += render::get_text_size(render::Fonts::Pixelmix10px, flag_text).y - 2.0f;
		}
	}

	void health_bar(rust::Entity* entity)
	{
		if (!settings.visuals.players.health_bar)
			return;
		if (!entity->bounding_box.valid)
			return;
		const float max_hp = entity->max_health > 1.0f ? entity->max_health : 100.0f;
		const float hp = std::clamp(entity->health, 0.0f, max_hp);
		const float ratio = std::clamp(hp / max_hp, 0.0f, 1.0f);

		const auto& box = entity->bounding_box;
		const float gap = 3.0f;
		const float width = 2.0f;
		const glm::vec2 bar_pos{ box.position.x - gap - width, box.position.y };
		const glm::vec2 bar_size{ width, box.size.y };
		if (bar_size.y < 2.0f)
			return;

		render::add_rect(bar_pos - glm::vec2{ 1.0f }, bar_size + glm::vec2{ 2.0f }, Color(0, 0, 0, 220), 0.0f, 1.0f);
		render::add_rect_filled(bar_pos, bar_size, Color(30, 30, 30, 200));

		const float filled = bar_size.y * ratio;
		Color fill = settings.visuals.players.health_bar_color;
		if (ratio > 0.5f) {
			const float t = (ratio - 0.5f) * 2.0f;
			fill = Color(int(255 * (1.0f - t)), 255, 0, fill.a);
		} else {
			const float t = ratio * 2.0f;
			fill = Color(255, int(255 * t), 0, fill.a);
		}
		render::add_rect_filled({ bar_pos.x, bar_pos.y + bar_size.y - filled }, { bar_size.x, filled }, fill);
	}

	void distance_label(rust::Entity* entity)
	{
		if (!settings.visuals.players.distance)
			return;
		if (!entity->bounding_box.valid)
			return;
		char buf[24];
		std::snprintf(buf, sizeof(buf), "%dm", static_cast<int>(entity->distance));
		const auto& box = entity->bounding_box;
		const glm::vec2 pos{
			box.position.x + box.size.x * 0.5f,
			box.position.y + box.size.y + 3.0f
		};
		render::add_text(render::Fonts::Pixelmix10px, buf, pos, settings.visuals.players.distance_color,
			render::TextFlagsOutline, glm::vec2{ 0.5f, 0.0f });
	}

	void snaplines(rust::Entity* entity)
	{
		struct SLSnap { bool enabled=false; int alignment=1; Color color; };
		static SLSnap s_sl;
		static auto s_sl_last = std::chrono::steady_clock::time_point{};
		const auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - s_sl_last).count() >= 250) {
			s_sl_last = now;
			s_sl.enabled = settings.visuals.players.snap_lines;
			if (s_sl.enabled) {
				s_sl.color     = settings.visuals.players.snap_lines_color;
				s_sl.alignment = settings.visuals.players.snap_lines_alignment;
			}
		}
		if (!s_sl.enabled) return;

		glm::vec2 position{ winapp::impl::window_size.x * 0.5f, winapp::impl::window_size.y };
		switch (s_sl.alignment)
		{
		case 0:
			position.y = 0.0f;
			break;
		case 1:
			position.y *= 0.5f;
			break;
		}

		Color sl_c = s_sl.color;
		if (entity->is_wounded) sl_c = Color(255, 220, 0, sl_c.a);
		render::add_line(position, entity->bounding_box.position + glm::vec2{ entity->bounding_box.size.x * 0.5f, entity->bounding_box.size.y }, sl_c);
	}

	void out_of_view(rust::Entity* entity)
	{
		struct OOVSnap { bool enabled=false; bool glow=false; Color color; };
		static OOVSnap s_oov;
		static auto s_oov_last = std::chrono::steady_clock::time_point{};
		const auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - s_oov_last).count() >= 250) {
			s_oov_last = now;
			s_oov.enabled = settings.visuals.players.out_of_view_arrows;
			s_oov.glow = settings.visuals.players.arrow_glow;
			if (s_oov.enabled) s_oov.color = settings.visuals.players.out_of_view_arrows_color;
		}
		if (!s_oov.enabled) return;
		if (entity->bounding_box.valid) return;
		const Color color = s_oov.color;
		const bool glow = s_oov.glow;

		const auto rotate_triangle = [&](std::vector<glm::vec2>& points, f32 rotation) -> void
			{
				glm::vec2 points_center{};
				for (const auto& e : points)
					points_center += e;

				points_center.x /= points.size();
				points_center.y /= points.size();
				const glm::vec2 offset = points_center;

				const f32 theta = rotation * (std::numbers::pi_v<float> / 180.f);
				const f32 c = std::cosf(theta);
				const f32 s = std::sinf(theta);

				for (auto& point : points)
				{
					point -= offset;
					const f32 temp_x = point.x;
					point.x = temp_x * c - point.y * s;
					point.y = temp_x * s + point.y * c;
					point.x += offset.x;
					point.y += offset.y;
				}
			};

		const glm::vec2 center = winapp::impl::window_size * 0.5f;
		const f32 size = settings.visuals.players.out_of_view_arrows_size;

		const f32 radius = (winapp::impl::window_size.y * 0.45f) * settings.visuals.players.out_of_view_arrows_radius;

		const f32 target_yaw = math::calc_angle(game::impl::local_player->origin, entity->origin).y;
		f32 relative_yaw = target_yaw - game::impl::local_player->view_angles.y;
		while (relative_yaw > 180.0f) relative_yaw -= 360.0f;
		while (relative_yaw < -180.0f) relative_yaw += 360.0f;

		const f32 angle_yaw = relative_yaw - 90.0f;
		const f32 yaw = angle_yaw * (std::numbers::pi_v<float> / 180.f);

		const f32 x = center.x + radius * std::cos(yaw);
		const f32 y = center.y + radius * std::sin(yaw);

		std::vector<glm::vec2> points{ glm::vec2(x - size, y - size), glm::vec2(x + size, y), glm::vec2(x - size, y + size) };

		rotate_triangle(points, angle_yaw);
		std::vector<glm::vec2> rounded_triangle = math::subdivide_arc(points, 0.3f, 12);

		if (glow)
			render::add_shadow_poly(rounded_triangle, color, 25.0f);
		render::add_polyline(rounded_triangle, color, 1.5f);
	}

	void player(rust::Entity* entity)
	{
		bool& enabled = settings.visuals.players.enabled;
		if (!enabled) return;

		if (!entity->has_origin) return;

		int& max_dist = settings.visuals.players.max_distance;
		// Skip distance cull when local origin is unknown (distance falls back to 0
		// or camera-based); otherwise a zeroed local origin culls everyone.
		const bool local_ok = game::impl::local_player && game::impl::local_player->has_origin;
		if (local_ok && entity->distance > static_cast<float>(max_dist)) return;

		bool& show_sleepers = settings.visuals.players.show_sleepers;
		if (entity->is_sleeping && !show_sleepers) return;
		if (settings.visuals.players.skip_crouching && entity->is_crouching) return;
		if (settings.visuals.players.skip_scoped && entity->is_scoped) return;

		const bool is_teammate = (game::impl::local_player &&
		                          entity != game::impl::local_player &&
		                          entity->team != 0 &&
		                          entity->team == game::impl::local_player->team);

		bool chest_visible = true;
		const bool vischeck_on = settings.visuals.players.vischeck;
		if (vischeck_on && game::impl::local_player && entity != game::impl::local_player
			&& game::impl::local_player->bones && entity->bones)
		{
			auto local_head = game::impl::local_player->bones->find(rust::BoneList::head);
			auto target_chest = entity->bones->find(rust::BoneList::spine3);
			if (local_head != game::impl::local_player->bones->end() && local_head->second.visible &&
				target_chest != entity->bones->end() && target_chest->second.visible)
			{
				const glm::vec3 from = local_head->second.position;
				const glm::vec3 to   = target_chest->second.position;
				chest_visible = features::physx::is_visible(from, to);
				if (!chest_visible) {
					const bool dim = settings.visuals.players.vis_dim_instead_of_hide;
					if (!dim) return;
				}
			}
		}

		const bool tint_enable = vischeck_on && (chest_visible
			? settings.visuals.players.vis_visible_color_enabled
			: settings.visuals.players.vis_invisible_color_enabled);
		const Color chest_tint = chest_visible
			? settings.visuals.players.vis_visible_color
			: settings.visuals.players.vis_invisible_color;
		const bool per_bone_vis_on = vischeck_on && settings.visuals.players.vis_per_bone_skeleton;

		entity->bounding_box.valid = false;
		if (entity->bones && !entity->bones->empty())
		{
			glm::vec2 mn{ FLT_MAX, FLT_MAX };
			glm::vec2 mx{ -FLT_MAX, -FLT_MAX };
			bool any = false;
			for (const auto& [id, bd] : *entity->bones)
			{
				if (!bd.visible) continue;
				glm::vec2 sp{};
				if (!math::world_to_screen(bd.position, &sp)) continue;
				any = true;
				mn = glm::min(mn, sp);
				mx = glm::max(mx, sp);
			}
			if (any)
			{
				const float h = mx.y - mn.y;
				const float pad = h * 0.15f;
				entity->bounding_box.position = mn - glm::vec2{ pad, pad };
				entity->bounding_box.size     = (mx + glm::vec2{ pad, pad }) - entity->bounding_box.position;
				const auto& sz = entity->bounding_box.size;
				const auto& po = entity->bounding_box.position;
				const auto& ws = winapp::impl::window_size;
				const bool shape_ok = sz.y >= sz.x * 0.5f;
				entity->bounding_box.valid =
					shape_ok &&
					sz.x > 1.0f && sz.y > 1.0f &&
					po.x + sz.x > 0.0f && po.y + sz.y > 0.0f &&
					po.x < ws.x && po.y < ws.y && sz.x < ws.x && sz.y < ws.y;
			}
		}

		if (!entity->bounding_box.valid && entity->has_origin)
		{
			glm::vec2 feet{}, head{};
			const bool feet_ok = math::world_to_screen(entity->origin, &feet);
			const bool head_ok = math::world_to_screen(entity->origin + glm::vec3{ 0.0f, 1.8f, 0.0f }, &head);
			if (feet_ok && head_ok)
			{
				const float top = feet.y < head.y ? feet.y : head.y;
				const float bot = feet.y > head.y ? feet.y : head.y;
				const float h = bot - top;
				if (h > 2.0f)
				{
					const float w = h * 0.45f;
					const float cx = (feet.x + head.x) * 0.5f;
					entity->bounding_box.position = { cx - w * 0.5f, top };
					entity->bounding_box.size     = { w, h };
					const auto& sz = entity->bounding_box.size;
					const auto& po = entity->bounding_box.position;
					const auto& ws = winapp::impl::window_size;
					entity->bounding_box.valid =
						sz.x > 1.0f && sz.y > 1.0f &&
						po.x + sz.x > 0.0f && po.y + sz.y > 0.0f &&
						po.x < ws.x && po.y < ws.y && sz.x < ws.x && sz.y < ws.y;
				}
			}
		}

		std::int32_t start_vtx = render::draw_list->VtxBuffer.Size;

		if (!entity->bounding_box.valid)
		{
			out_of_view(entity);

			std::int32_t end_vtx = render::draw_list->VtxBuffer.Size;
			if (tint_enable) render::modulate_color(start_vtx, end_vtx, chest_tint);
			if (is_teammate) render::modulate_color(start_vtx, end_vtx, Color(0, 255, 0));
			render::modify_alpha(start_vtx, end_vtx, entity->alpha_anim.value);
			return;
		}

		name(entity);
		snaplines(entity);
		bounding_box(entity);
		health_bar(entity);

		const std::int32_t skel_start_vtx = render::draw_list->VtxBuffer.Size;
		skeleton(entity);
		const std::int32_t skel_end_vtx = render::draw_list->VtxBuffer.Size;

		head_circle(entity);
		view_line(entity);
		item_name(entity);
		flags(entity);
		distance_label(entity);

		std::int32_t end_vtx = render::draw_list->VtxBuffer.Size;

		if (tint_enable) {
			if (per_bone_vis_on) {
				render::modulate_color(start_vtx, skel_start_vtx, chest_tint);
				render::modulate_color(skel_end_vtx, end_vtx, chest_tint);
			} else {
				render::modulate_color(start_vtx, end_vtx, chest_tint);
			}
		}
		if (is_teammate) render::modulate_color(start_vtx, end_vtx, Color(0, 255, 0));
		render::modify_alpha(start_vtx, end_vtx, entity->alpha_anim.value);
	}
}