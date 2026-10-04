#include "cache.hpp"
#include <game/bundle_icons/bundle_icons.hpp>
#include <game/game.hpp>
#include <globals.hpp>
#include <memory/memory.hpp>
#include <sdk/rust/entity/entity.hpp>
#include <sdk/decryptions.hpp>
#include <sdk/offsets.hpp>
#include <sdk/rust/gchandle.hpp>
#include <sdk/unity/unity.hpp>

#include <algorithm>
#include <atomic>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <print>
#include <utils/debug.hpp>
#include <sdk/math/math.hpp>
#include <thread>
#include <vector>
#include <glm/gtc/quaternion.hpp>

#include <app/winapp.hpp>
#include <fstream>
#include <sdk/rust/entity/weapon.hpp>
#include <settings/settings.hpp>
#include <string_encryption.hpp>

#include <condition_variable>
#include <queue>
#include <render/texture/texture.hpp>
#include <mutex>
#include <unordered_set>

namespace render {
    extern texture_t steam_avatar;
}

namespace cache
{
	static std::optional<glm::vec3> resolve_transform_world_pos(uptr access)
	{
		if (!offsets::UnityTransform::parent_off ||
		    !offsets::UnityTransform::local_pos_off ||
		    !offsets::UnityTransform::world_rot_off)
		{
			return std::nullopt;
		}

		auto sane = [](const glm::vec3& v) {
			return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
		};

	constexpr int kMaxDepth = 8;

	glm::vec3 accum{ 0.0f, 0.0f, 0.0f };
	glm::quat accum_rot{ 1.0f, 0.0f, 0.0f, 0.0f };

	uptr node = access;
	for (int depth = 0; depth < kMaxDepth && node; ++depth)
		{
			const glm::vec3 local_pos = memory::read<glm::vec3>(node + offsets::UnityTransform::local_pos_off);
			if (!sane(local_pos)) return std::nullopt;

			accum = accum_rot * local_pos + accum;

			const uptr parent = memory::read<uptr>(node + offsets::UnityTransform::parent_off);
			if (!parent)
			{
				return accum;
			}

			const glm::quat parent_rot = memory::read<glm::quat>(parent + offsets::UnityTransform::world_rot_off);
			if (!std::isfinite(parent_rot.x) || !std::isfinite(parent_rot.y) ||
			    !std::isfinite(parent_rot.z) || !std::isfinite(parent_rot.w))
			{
				return std::nullopt;
			}
		accum_rot = parent_rot;

		node = parent;
	}
	return std::nullopt;
}

	static bool try_read_transform_world_pos(uptr access, glm::vec3& out)
	{
		// Prefer the dump-confirmed native world position (access+0x90).
		// Parent-walk offsets (parent/local_pos/world_rot) are not in the
		// current dump — when wrong they return sane-looking local-space
		// junk and starve ESP of real origins.
		const glm::vec3 wp = memory::read<glm::vec3>(access + offsets::UnityTransform::world_pos_off);
		if (std::isfinite(wp.x) && std::isfinite(wp.y) && std::isfinite(wp.z) &&
		    (wp.x != 0.0f || wp.y != 0.0f || wp.z != 0.0f) &&
		    std::abs(wp.x) < 10000.0f && std::abs(wp.y) < 10000.0f && std::abs(wp.z) < 10000.0f)
		{
			out = wp;
			return true;
		}

		if (auto walked = resolve_transform_world_pos(access))
		{
			out = *walked;
			return true;
		}
		return false;
	}

	static uintptr_t resolve_local_player_static()
	{
		const uintptr_t typeinfo = memory::read(game::impl::game_assembly + offsets::LocalPlayer_Static::typeinfo);
		if (!typeinfo) return 0;
		const uintptr_t sf = memory::read(typeinfo + offsets::LocalPlayer_Static::static_fields);
		if (!sf) return 0;

		if (uintptr_t wrap = memory::read(sf + offsets::LocalPlayer_Static::Entity)) {
			if (uintptr_t ent = decryption::client_entities(wrap))
				return ent;
		}

		auto decode_ent = [](uintptr_t ent) -> uintptr_t {
			if (!ent) return 0;
			if (ent & 1) {
				ent = gchandle::get_target(ent);
				if (!ent) return 0;
			}
			if (ent > 0x100000000ULL && ent < 0x00007FFFFFFFFFFFULL)
				return ent;
			return 0;
		};

		constexpr uintptr_t kEntOffs[] = { (uintptr_t)0x8, (uintptr_t)0x48, (uintptr_t)0x18, (uintptr_t)0x20 };
		for (uintptr_t off : kEntOffs) {
			if (uintptr_t ent = decode_ent(memory::read(sf + off)))
				return ent;
		}
		return 0;
	}

	static std::unordered_set<uptr>* g_player_klass_cache = nullptr;
	static std::mutex                g_player_klass_mut;

	static bool klass_known_player(uptr entity_ptr) {
		const uptr klass = memory::read<uptr>(entity_ptr);
		if (klass <= 0x10000000ULL || klass >= 0x00007FFFFFFFFFFFULL)
			return false;
		std::lock_guard lk(g_player_klass_mut);
		return g_player_klass_cache && g_player_klass_cache->count(klass) > 0;
	}

	static void remember_player_klass(uptr entity_ptr) {
		const uptr klass = memory::read<uptr>(entity_ptr);
		if (klass <= 0x10000000ULL || klass >= 0x00007FFFFFFFFFFFULL)
			return;
		std::lock_guard lk(g_player_klass_mut);
		if (!g_player_klass_cache) g_player_klass_cache = new std::unordered_set<uptr>;
		g_player_klass_cache->insert(klass);
	}

	struct KlassInfo { EntityType type; std::string name; };
	static std::unordered_map<uptr, KlassInfo>* g_world_klass_cache = nullptr;
	static std::mutex                            g_world_klass_mut;

	static bool klass_known_world(uptr entity_ptr, KlassInfo& out) {
		const uptr klass = memory::read<uptr>(entity_ptr);
		if (klass <= 0x10000000ULL || klass >= 0x00007FFFFFFFFFFFULL)
			return false;
		std::lock_guard lk(g_world_klass_mut);
		if (!g_world_klass_cache) return false;
		auto it = g_world_klass_cache->find(klass);
		if (it == g_world_klass_cache->end()) return false;
		out = it->second;
		return true;
	}

	static bool is_dropped_bucket(EntityType type) {
		switch (type) {
		case EntityType::DroppedRifle: case EntityType::DroppedSniper:
		case EntityType::DroppedSMG: case EntityType::DroppedShotgun:
		case EntityType::DroppedPistol: case EntityType::DroppedLMG:
		case EntityType::DroppedLauncher: case EntityType::DroppedBow:
		case EntityType::DroppedMelee: case EntityType::DroppedThrowable:
		case EntityType::DroppedTool: case EntityType::DroppedMedical:
		case EntityType::DroppedAmmo: case EntityType::DroppedMisc:
			return true;
		default:
			return false;
		}
	}

	static void remember_world_klass(uptr entity_ptr, EntityType type, const std::string& name) {
		if (type == EntityType::None || type == EntityType::Player) return;
		// Shared klasses (collectables / WorldItem) must not poison siblings.
		switch (type) {
		case EntityType::Hemp:
		case EntityType::Wood:
		case EntityType::Stone:
		case EntityType::Sulfur:
		case EntityType::Metal:
		case EntityType::DieselFuel:
		case EntityType::GreenKeycard:
		case EntityType::BlueKeycard:
		case EntityType::RedKeycard:
			return;
		default:
			break;
		}
		if (is_dropped_bucket(type)) return;
		const uptr klass = memory::read<uptr>(entity_ptr);
		if (klass <= 0x10000000ULL || klass >= 0x00007FFFFFFFFFFFULL)
			return;
		std::lock_guard lk(g_world_klass_mut);
		if (!g_world_klass_cache) g_world_klass_cache = new std::unordered_map<uptr, KlassInfo>;
		(*g_world_klass_cache)[klass] = { type, name };
	}

	static std::atomic<u32> s_prefab_id_off{ 0x54 };

	static void maybe_relearn_prefab_id_off(const std::vector<uptr>& sample) {
		if (!prefabs || sample.empty()) return;
		static constexpr u32 kCandidates[] = { 0x54, 0x50, 0x58, 0x5C, 0x60, 0x64, 0x68, 0x6C, 0x70 };
		const u32 current = s_prefab_id_off.load(std::memory_order_relaxed);
		int cur_hits = 0;
		int best_hits = 0;
		u32 best_off = current;
		for (u32 off : kCandidates) {
			int hits = 0;
			for (uptr p : sample) {
				if (!p) continue;
				u32 v = memory::read<u32>(p + off);
				if (prefabs->find(v) != prefabs->end()) ++hits;
			}
			if (off == current) cur_hits = hits;
			if (hits > best_hits) { best_hits = hits; best_off = off; }
		}
		if (best_hits > cur_hits && best_hits > 0 && best_off != current) {
			DBG("[prefab-id] relearning offset: 0x{:x} -> 0x{:x} (hits {} -> {} in sample of {})",
				current, best_off, cur_hits, best_hits, sample.size());
			s_prefab_id_off.store(best_off, std::memory_order_relaxed);
		}
	}

	static uptr cached_buffer_list = 0;
	static i32 failed_reads = 0;
	static i32 ticks_since_chain_walk = 0;
	static constexpr i32 CHAIN_REWALK_EVERY_N_TICKS = 60;
	static constexpr i32 MAX_ENT_LIST_COUNT         = 65536;

	struct avatar_data_t {
		std::vector<unsigned char> buffer;
	};

	// Steam avatar / profile network fetches were removed to eliminate all
	// external references. This is a no-op stub kept only so call sites in the
	// entity loop still compile; every request resolves to the "failed" sentinel
	// so consumers fall back to the default (unloaded) avatar path.
	class avatar_manager_t {
	public:
		static constexpr std::size_t SENTINEL_SIZE = 1;
		static bool is_sentinel(const std::vector<unsigned char>& b) {
			return b.size() == SENTINEL_SIZE && b[0] == 0;
		}

		void start() { /* no network worker */ }

		void request(uint64_t steam_id) {
			std::unique_lock lock(m_results_mutex);
			(*m_results)[steam_id].assign(SENTINEL_SIZE, 0);
		}

		std::vector<unsigned char> pop_result(uint64_t steam_id) {
			std::unique_lock lock(m_results_mutex);
			if (m_results->contains(steam_id)) {
				auto res = std::move((*m_results)[steam_id]);
				m_results->erase(steam_id);
				return res;
			}
			return {};
		}

		bool load_default_into(texture_t& /*tex*/) { return false; }

	private:
		std::unordered_map<uint64_t, std::vector<unsigned char>>* m_results = new std::unordered_map<uint64_t, std::vector<unsigned char>>();
		std::mutex m_results_mutex;
	};

	static avatar_manager_t g_avatar_manager;

	class item_texture_manager_t {
	public:
		void start(std::vector<std::string> shortnames) {
			std::thread([this, shortnames = std::move(shortnames)]() mutable {
				std::string game_dir;
				for (int i = 0; i < 100; ++i) {
					if (g_should_terminate) return;
					game_dir = memory::get_game_directory();
					if (!game_dir.empty()) break;
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				if (game_dir.empty()) return;

				const std::string items_dir = game_dir + "\\Bundles\\items\\";

				std::unordered_set<std::string> seen;
				for (const auto& sn : shortnames) {
					if (g_should_terminate) break;
					if (sn.empty() || !seen.insert(sn).second) continue;

					auto bytes = read_png(items_dir + sn + ".png");
					if (bytes.empty()) continue;

					std::unique_lock lock(m_results_mtx);
					m_pending.emplace_back(sn, std::move(bytes));
				}
			}).detach();
		}

		std::vector<std::pair<std::string, std::vector<unsigned char>>> drain() {
			std::vector<std::pair<std::string, std::vector<unsigned char>>> out;
			std::unique_lock lock(m_results_mtx);
			out.swap(m_pending);
			return out;
		}

	private:
		static std::vector<unsigned char> read_png(const std::string& path) {
			std::ifstream f(path, std::ios::binary | std::ios::ate);
			if (!f.is_open()) return {};
			std::streamsize sz = f.tellg();
			if (sz <= 0 || sz > 8 * 1024 * 1024) return {};
			f.seekg(0, std::ios::beg);
			std::vector<unsigned char> bytes(static_cast<std::size_t>(sz));
			if (!f.read(reinterpret_cast<char*>(bytes.data()), sz)) return {};
			return bytes;
		}

		std::mutex m_results_mtx;
		std::vector<std::pair<std::string, std::vector<unsigned char>>> m_pending;
	};

	static item_texture_manager_t g_item_texture_manager;

	void set_material(rust::Entity* entity, u32 material_id, bool active)
	{
		uptr model = memory::read(entity->base_address + offsets::BasePlayer::playerModel);
		if (!model) return;

		uptr skinned_mesh = memory::read(model + offsets::PlayerModel::SkinnedMultiMesh);
		if (!skinned_mesh) return;

		auto is_heap = [](uptr p) {
			return p > 0x10000000ULL && p < 0x00007FFFFFFFFFFFULL;
		};

		uptr skinned_list    = 0;
		i32  materials_count = 0;

		for (uptr slot = 0; slot <= 0x200; slot += 8) {
			uptr cand_obj = memory::read<uptr>(skinned_mesh + slot);
			if (!is_heap(cand_obj)) continue;

			uptr cand_items = memory::read<uptr>(cand_obj + 0x10);
			i32  cand_count = memory::read<i32>(cand_obj + 0x18);
			if (!is_heap(cand_items) || cand_count <= 0 || cand_count > 100) continue;

			uptr probe_entry = memory::read<uptr>(cand_items + 0x20);
			if (!is_heap(probe_entry)) continue;
			uptr probe_unity = memory::read<uptr>(probe_entry + 0x10);
			if (!is_heap(probe_unity)) continue;
			uptr probe_mat_base = memory::read<uptr>(probe_unity + offsets::SkinnedMultiMesh::rendererMaterialArray);
			u32  probe_mat_size = memory::read<u32>(probe_unity + offsets::SkinnedMultiMesh::rendererMaterialArray + 0x10);
			if (!is_heap(probe_mat_base) || probe_mat_size < 1 || probe_mat_size > 64) continue;

			skinned_list = cand_items;
			materials_count = cand_count;
			break;
		}

		if (!skinned_list) return;

		for (i32 idx = 0; idx < materials_count; idx++) {
			uptr render_entry = memory::read(skinned_list + 0x20 + (idx * 0x8));
			if (!render_entry) continue;

			uptr unity_object = memory::read(render_entry + 0x10);
			if (!unity_object) continue;

			uptr material_list_base = memory::read(unity_object + offsets::SkinnedMultiMesh::rendererMaterialArray);
			uptr material_list_size = memory::read(unity_object + offsets::SkinnedMultiMesh::rendererMaterialArray + 0x10);

			if (!material_list_base || material_list_size < 1 || material_list_size > 64 || !entity->original_materials) continue;

			auto& cached = (*entity->original_materials)[material_list_base];

			if (active) {
				if (cached.empty()) {
					for (u32 i = 0; i < material_list_size; i++) {
						cached.push_back(memory::read<u32>(material_list_base + (i * 0x4)));
					}
				}
				for (u32 i = 0; i < material_list_size; i++) {
					memory::write<u32>(material_list_base + (i * 0x4), material_id);
				}
			}
			else if (!cached.empty()) {
				const u32 restore_count = std::min<u32>(static_cast<u32>(cached.size()), material_list_size);
				for (u32 i = 0; i < restore_count; i++) {
					memory::write<u32>(material_list_base + (i * 0x4), cached[i]);
				}
			}
		}
	}

	static bool is_userland_ptr(uptr v) {
		return v > 0x10000000ULL && v < 0x00007FFFFFFFFFFFULL;
	}

	static std::string read_il2cpp_string(uptr str_obj)
	{
		if (!is_userland_ptr(str_obj)) return {};
		const i32 len = memory::read<i32>(str_obj + 0x10);
		if (len <= 0 || len > 256) return {};

		std::wstring wide(static_cast<std::size_t>(len), L'\0');
		if (!memory::read_memory_raw(str_obj + 0x14, wide.data(), static_cast<std::size_t>(len) * sizeof(wchar_t)))
			return {};

		std::string out;
		out.reserve(static_cast<std::size_t>(len));
		for (wchar_t wc : wide) {
			if (wc == 0) break;
			out.push_back(wc < 0x80 ? static_cast<char>(wc) : '?');
		}
		return out;
	}

	static uptr resolve_item_definition(uptr item)
	{
		if (!is_userland_ptr(item)) return 0;
		static constexpr uptr kInfoOffs[] = {
			offsets::Item::itemDefinition,
			0x70, 0x60, 0x68, 0x78, 0x80, 0x90, 0x98, 0xA0
		};
		static constexpr uptr kShortOffs[] = {
			offsets::ItemDefinition::shortName,
			0x20, 0x28, 0x30, 0x38
		};
		for (uptr info_off : kInfoOffs) {
			const uptr def = memory::read<uptr>(item + info_off);
			if (!is_userland_ptr(def)) continue;
			for (uptr sn_off : kShortOffs) {
				const uptr sn = memory::read<uptr>(def + sn_off);
				if (is_userland_ptr(sn) && !read_il2cpp_string(sn).empty())
					return def;
			}
		}
		return 0;
	}

	static uptr read_item_shortname_ptr(uptr def)
	{
		static constexpr uptr kShortOffs[] = {
			offsets::ItemDefinition::shortName,
			0x20, 0x28, 0x30, 0x38
		};
		for (uptr sn_off : kShortOffs) {
			const uptr sn = memory::read<uptr>(def + sn_off);
			if (is_userland_ptr(sn) && !read_il2cpp_string(sn).empty())
				return sn;
		}
		return 0;
	}

	std::string get_item_name_short(uptr item)
	{
		const uptr def = resolve_item_definition(item);
		if (!def) return {};
		return read_il2cpp_string(read_item_shortname_ptr(def));
	}

	static std::string get_item_display_name(uptr item)
	{
		const uptr def = resolve_item_definition(item);
		if (!def) return {};

		const std::string short_name = read_il2cpp_string(read_item_shortname_ptr(def));
		if (!short_name.empty() && rust::item_display_names) {
			auto it = rust::item_display_names->find(short_name);
			if (it != rust::item_display_names->end() && it->second.display_name)
				return it->second.display_name;
		}

		const uptr phrase = memory::read<uptr>(def + offsets::item_definition::display_name);
		if (is_userland_ptr(phrase)) {
			for (uptr off : { (uptr)0x18, (uptr)0x14, (uptr)0x20 }) {
				const std::string english = read_il2cpp_string(memory::read<uptr>(phrase + off));
				if (!english.empty()) return english;
			}
		}
		return short_name;
	}

	static std::atomic<uptr> s_world_item_off{ 0x1F0 };

	static uptr resolve_world_item_ptr(uptr entity_ptr)
	{
		if (!is_userland_ptr(entity_ptr)) return 0;

		auto try_off = [&](uptr off) -> uptr {
			const uptr item = memory::read<uptr>(entity_ptr + off);
			return resolve_item_definition(item) ? item : 0;
		};

		const uptr learned = s_world_item_off.load(std::memory_order_relaxed);
		if (uptr item = try_off(learned))
			return item;

		for (uptr off = 0x100; off <= 0x300; off += 8) {
			if (off == learned) continue;
			if (uptr item = try_off(off)) {
				s_world_item_off.store(off, std::memory_order_relaxed);
				return item;
			}
		}
		return 0;
	}

	static EntityType tier_to_dropped_type(rust::WeaponTier tier)
	{
		switch (tier) {
		case rust::Rifle:     return EntityType::DroppedRifle;
		case rust::Sniper:    return EntityType::DroppedSniper;
		case rust::SMG:       return EntityType::DroppedSMG;
		case rust::Shotgun:   return EntityType::DroppedShotgun;
		case rust::Pistol:    return EntityType::DroppedPistol;
		case rust::LMG:       return EntityType::DroppedLMG;
		case rust::Launcher:  return EntityType::DroppedLauncher;
		case rust::Bow:       return EntityType::DroppedBow;
		case rust::Melee:     return EntityType::DroppedMelee;
		case rust::Throwable: return EntityType::DroppedThrowable;
		case rust::Tool:      return EntityType::DroppedTool;
		case rust::Medical:   return EntityType::DroppedMedical;
		case rust::Misc:      return EntityType::DroppedAmmo;
		default:              return EntityType::DroppedMisc;
		}
	}

	static bool apply_world_item_identity(rust::Entity* entity)
	{
		if (!entity || !is_dropped_bucket(entity->type)) return false;

		const uptr item = resolve_world_item_ptr(entity->base_address);
		if (!item) return false;

		const std::string short_name = get_item_name_short(item);
		if (short_name.empty()) return false;

		std::string display = get_item_display_name(item);
		if (display.empty()) display = short_name;

		entity->name = std::move(display);
		if (rust::item_display_names) {
			auto it = rust::item_display_names->find(short_name);
			if (it != rust::item_display_names->end())
				entity->type = tier_to_dropped_type(it->second.tier);
		}
		return true;
	}

	static bool needs_world_item_resolve(const rust::Entity* entity)
	{
		if (!entity || !is_dropped_bucket(entity->type)) return false;
		return entity->name.empty()
			|| entity->name == "Dropped item"
			|| entity->name == "Unknown";
	}

	std::string get_prefab_name(uptr entity)
	{
		uptr name_buffer = memory::chain_read<uptr>(entity, { 0x10, 0x30, 0x60 });
		return memory::read_string(name_buffer);
	}

	std::unordered_map<std::string, std::unique_ptr<texture_t>>* g_item_textures = {};

	static std::mutex s_update_mut;
	static void update_entities_internal();

	bool create_instance()
	{
		entity_list = new std::unordered_map<std::uintptr_t, rust::Entity*>;
		g_item_textures = new std::unordered_map<std::string, std::unique_ptr<texture_t>>;

		g_avatar_manager.start();

		{
			std::vector<std::string> sn_list;
			sn_list.reserve(rust::item_display_names->size() + weapon_held_shortnames->size());
			for (auto& [shortname, _] : *rust::item_display_names)
				sn_list.push_back(shortname);
			for (auto& [_, shortname] : *weapon_held_shortnames)
				sn_list.push_back(shortname);
			g_item_texture_manager.start(std::move(sn_list));
		}

		std::thread([&] {
			memory::pin_thread_stack();
			while (!g_should_terminate)
			{
				gather();
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
			}).detach();

		std::thread([&] {
			memory::pin_thread_stack();
			while (!g_should_terminate)
			{
				update_entities_internal();
				std::this_thread::sleep_for(std::chrono::milliseconds(16));
			}
			}).detach();

		return true;
	}

	void gather()
	{
		std::unique_lock guard(s_update_mut);

		if (!game::is_in_game() || game::impl::game_assembly == 0 || g_should_refresh_cache)
		{
			cached_buffer_list = 0;
			failed_reads = 0;

			std::unique_lock lock(cache_mut);
			for (auto& pair : *entity_list)
				delete pair.second;

			entity_list->clear();
			game::impl::local_player = nullptr;
			g_should_refresh_cache = false;
			g_cache_epoch.fetch_add(1, std::memory_order_release);
			return;
		}

		if (++ticks_since_chain_walk >= CHAIN_REWALK_EVERY_N_TICKS)
		{
			cached_buffer_list = 0;
			ticks_since_chain_walk = 0;
		}

		auto is_heap_ptr = [](uptr v) {
			return v > 0x100000000ULL && v < 0x00007FFFFFFFFFFFULL;
		};

		if (!cached_buffer_list)
		{
			ticks_since_chain_walk = 0;

			uptr base_networkable = memory::read(game::impl::game_assembly + offsets::base_networkable::typeinfo);
			if (!base_networkable) { DBG("[bn-walk] FAIL @1 typeinfo=0"); return; }

			uptr static_fields = memory::read(base_networkable + offsets::base_networkable::static_fields);
			if (!static_fields) { DBG("[bn-walk] FAIL @2 static_fields=0"); return; }

			uptr wrapper_class_ptr = memory::read(static_fields + offsets::base_networkable::wrapper_class_ptr);
			if (!wrapper_class_ptr) { DBG("[bn-walk] FAIL @3 wrapper_class_ptr=0"); return; }

			uptr wrapper_class = decryption::base_networkable_0(wrapper_class_ptr);
			if (!wrapper_class) {
				DBG("[bn-walk] FAIL @4 decrypt_0=0 (wrapper_ptr=0x{:x} get_handle=0x{:x})",
					wrapper_class_ptr, (uptr)offsets::il2cpp::get_handle);
				return;
			}

			uptr parent_static_ptr = memory::read(wrapper_class + offsets::base_networkable::parent_static_fields);
			if (!parent_static_ptr) { DBG("[bn-walk] FAIL @5 parent_static_ptr=0"); return; }

			uptr parent_statics = decryption::base_networkable_1(parent_static_ptr);
			if (!parent_statics) {
				DBG("[bn-walk] FAIL @6 decrypt_1=0 (parent_ptr=0x{:x})", parent_static_ptr);
				return;
			}

			static uptr s_learned_entities_off = 0;
			const uptr entities_off = s_learned_entities_off
				? s_learned_entities_off
				: (uptr)offsets::base_networkable::entities;

			cached_buffer_list = memory::read(parent_statics + entities_off);

			bool current_looks_valid = false;
			if (cached_buffer_list && is_heap_ptr(cached_buffer_list)) {
				uptr arr = memory::read<uptr>(cached_buffer_list + 0x10);
				i32  sz  = memory::read<i32>(cached_buffer_list + 0x18);
				if (!is_heap_ptr(arr)) { // swap-tolerant
					arr = memory::read<uptr>(cached_buffer_list + 0x18);
					sz  = memory::read<i32>(cached_buffer_list + 0x10);
				}
				if (is_heap_ptr(arr) && sz > 0 && sz < 50000
				    && is_heap_ptr(memory::read<uptr>(arr + 0x20))) {
					current_looks_valid = true;
				}
			}

			if (!current_looks_valid) {
				uptr best = 0;
				uptr best_off = 0;
				for (uptr off = 0; off <= 0x100; off += 8) {
					uptr cand = memory::read<uptr>(parent_statics + off);
					if (!is_heap_ptr(cand)) continue;
					uptr arr = memory::read<uptr>(cand + 0x10);
					i32  sz  = memory::read<i32>(cand + 0x18);
					if (!is_heap_ptr(arr)) {
						arr = memory::read<uptr>(cand + 0x18);
						sz  = memory::read<i32>(cand + 0x10);
					}
					if (!is_heap_ptr(arr) || sz <= 0 || sz >= 50000) continue;
					if (is_heap_ptr(memory::read<uptr>(arr + 0x20))) {
						best = cand;
						best_off = off;
						break;
					}
				}
				if (best) {
					cached_buffer_list = best;
					if (best_off && best_off != s_learned_entities_off) {
						DBG("[bn-rescan] learned entities_off = 0x{:x} (dumper said 0x{:x})",
							best_off, (uptr)offsets::base_networkable::entities);
						s_learned_entities_off = best_off;
					}
				}
			}
		}

		uptr buffer_list = cached_buffer_list;
		if (!buffer_list) return;

		uptr val10 = memory::read<uptr>(buffer_list + 0x10);
		uptr val18 = memory::read<uptr>(buffer_list + 0x18);
		uptr ent_list_start;
		i32  ent_list_count;
		if (val10 > val18) {
			ent_list_start = val10;
			ent_list_count = (i32)val18;
		} else {
			ent_list_start = val18;
			ent_list_count = (i32)val10;
		}

		if (ent_list_count <= 0 || ent_list_count > MAX_ENT_LIST_COUNT)
		{
			static int _cap_log_throttle = 0;
			if ((++_cap_log_throttle % 30) == 1) {
				DBG("[cache] ent_list_count={} out of range (cap={}), re-walking chain",
					ent_list_count, MAX_ENT_LIST_COUNT);
			}
			cached_buffer_list = 0;
			g_cache_epoch.fetch_add(1, std::memory_order_release);
			return;
		}

		thread_local std::vector<uptr> entity_ptrs;
		thread_local std::vector<uptr> entity_ptrs_verify;
		entity_ptrs.resize(ent_list_count);
		entity_ptrs_verify.resize(ent_list_count);
		const size_t buffer_size = ent_list_count * sizeof(uptr);
		if (!memory::read_memory_raw(ent_list_start + 0x20, entity_ptrs.data(), buffer_size))
		{
			failed_reads++;
			if (failed_reads >= 3)
			{
				cached_buffer_list = 0;
				failed_reads = 0;

				std::unique_lock lock(cache_mut);
				for (auto& pair : *entity_list)
					delete pair.second;

				entity_list->clear();
				game::impl::local_player = nullptr;
				g_cache_epoch.fetch_add(1, std::memory_order_release);
			}
			return;
		}

		if (!memory::read_memory_raw(ent_list_start + 0x20, entity_ptrs_verify.data(), buffer_size))
		{
			failed_reads++;
			if (failed_reads >= 3) { cached_buffer_list = 0; failed_reads = 0; }
			return;
		}
		if (std::memcmp(entity_ptrs.data(), entity_ptrs_verify.data(), buffer_size) != 0)
		{
			entity_ptrs.swap(entity_ptrs_verify);
		}
		failed_reads = 0;

		if (!entity_ptrs.empty() && entity_ptrs[0] != 0)
		{
			const uptr first = entity_ptrs[0];
			const bool heap_shaped = first > 0x100000000ULL && first < 0x00007FFFFFFFFFFFULL;
			if (!heap_shaped)
			{
				static int _bad_first_log = 0;
				if ((_bad_first_log++ % 30) == 0)
					DBG("[cache] entity_ptrs[0]={:#x} is not heap-shaped — invalidating cached BufferList (probably reading keys not values)",
						first);
				cached_buffer_list = 0;
				ticks_since_chain_walk = CHAIN_REWALK_EVERY_N_TICKS;
				g_cache_epoch.fetch_add(1, std::memory_order_release);
				return;
			}
		}

		bool debug_esp = settings.misc.general.debug_esp;
		std::unordered_set<uptr> found_this_tick;
		found_this_tick.reserve(ent_list_count);

		const uptr local_from_static = resolve_local_player_static();
		uptr local_player_ptr = local_from_static;
		if (!local_player_ptr && !entity_ptrs.empty())
			local_player_ptr = entity_ptrs[0];

		{
			static int s_gather_diag = 0;
			if ((++s_gather_diag % 60) == 1) {
				DBG("[cache] gather: ents={} local=0x{:x} static_ok={} buf=0x{:x}",
					ent_list_count, local_player_ptr, local_from_static != 0, buffer_list);
			}
		}

		if (!prefabs) return;

		static std::uint32_t s_gather_tick = 0;
		++s_gather_tick;
		constexpr std::uint32_t kPromoteEveryN = 5;
		const bool do_promote_pass = (s_gather_tick % kPromoteEveryN) == 0;

		if (local_player_ptr) {
			std::unique_lock lock(cache_mut);
			auto it_local = entity_list->find(local_player_ptr);
			if (it_local == entity_list->end()) {
				auto* e = new rust::Entity(local_player_ptr, EntityPrefabs::player, EntityType::Player);
				remember_player_klass(local_player_ptr);
				(*entity_list)[local_player_ptr] = e;
				game::impl::local_player = e;
			} else {
				if (it_local->second && it_local->second->type == EntityType::None)
					it_local->second->type = EntityType::Player;
				game::impl::local_player = it_local->second;
			}
			found_this_tick.insert(local_player_ptr);
		} else {
			static int s_nolocal = 0;
			if ((++s_nolocal % 30) == 1)
				DBG("[cache] gather: LocalPlayer static returned 0 (typeinfo=0x{:x} ent_off=0x{:x})",
					offsets::LocalPlayer_Static::typeinfo, offsets::LocalPlayer_Static::Entity);
		}

		guard.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(8));
		guard.lock();
		if (!game::is_in_game() || g_should_refresh_cache)
			return;
		if (local_player_ptr)
			found_this_tick.insert(local_player_ptr);

		std::unordered_set<uptr> known_snapshot;
		std::unordered_set<uptr> none_snapshot;
		{
			std::unique_lock lock(cache_mut);
			known_snapshot.reserve(entity_list->size());
			for (const auto& [ptr, e] : *entity_list) {
				if (!e) continue;
				known_snapshot.insert(ptr);
				if (e->type == EntityType::None) none_snapshot.insert(ptr);
			}
		}

		int name_resolves_left = 16;

		constexpr std::uint32_t kBlacklistRecheckEveryN = 10;
		const bool blacklist_recheck = (s_gather_tick % kBlacklistRecheckEveryN) == 0;
		static std::unordered_set<EntityPrefabs> s_useless_prefabs;
		static std::mutex s_useless_prefabs_mut;

		if (blacklist_recheck && !entity_ptrs.empty()) {
			std::vector<uptr> sample;
			sample.reserve(std::min<size_t>(entity_ptrs.size(), (size_t)32));
			for (size_t i = 0; i < entity_ptrs.size() && sample.size() < 32; ++i) {
				if (entity_ptrs[i]) sample.push_back(entity_ptrs[i]);
			}
			maybe_relearn_prefab_id_off(sample);
			std::lock_guard lk(s_useless_prefabs_mut);
			s_useless_prefabs.clear();
		}

		for (const auto& entity_ptr : entity_ptrs)
		{
			if (!entity_ptr) continue;

			if (known_snapshot.count(entity_ptr)) {
				if (!do_promote_pass || !none_snapshot.count(entity_ptr)) {
					found_this_tick.insert(entity_ptr);
					continue;
				}
			}

			const u32 pid_off = s_prefab_id_off.load(std::memory_order_relaxed);
			EntityPrefabs prefab_id = memory::read<EntityPrefabs>(entity_ptr + pid_off);

			if (!blacklist_recheck) {
				std::lock_guard lk(s_useless_prefabs_mut);
				if (s_useless_prefabs.count(prefab_id)) continue;
			}

			auto it = prefabs->find(prefab_id);
			bool is_local = (entity_ptr == local_player_ptr);
			if (it != prefabs->end() && it->second.type == EntityType::Player) {
				remember_player_klass(entity_ptr);
			}
			if (it != prefabs->end() && it->second.type != EntityType::Player) {
				remember_world_klass(entity_ptr, it->second.type, std::string(it->second.name));
			}
			if (is_local) remember_player_klass(entity_ptr);

			bool player_match = false;
			if (!is_local && it == prefabs->end()) {
				if (klass_known_player(entity_ptr)) {
					player_match = true;
				} else {
					const u64 uid = memory::read<u64>(entity_ptr + offsets::BasePlayer::userID);
					if (uid >= 76561197960265728ULL && uid <= 76561202255233023ULL)
						player_match = true;
				}
			}

			KlassInfo world_klass_info{};
			bool world_klass_match = false;
			if (!is_local && it == prefabs->end() && !player_match) {
				world_klass_match = klass_known_world(entity_ptr, world_klass_info);
			}

			if (it == prefabs->end() && !is_local && !debug_esp && !player_match && !world_klass_match)
			{
				bool already_cached = false;
				{
					std::unique_lock lock(cache_mut);
					already_cached = entity_list->find(entity_ptr) != entity_list->end();
				}
				if (already_cached)
				{
					found_this_tick.insert(entity_ptr);
					continue;
				}

				struct PrefabNameCache { bool is_droppable; EntityType type; std::string display_name; std::string prefab_name; };
				static std::unordered_map<EntityPrefabs, PrefabNameCache> s_name_cache;
				static std::mutex s_name_cache_mut;
				PrefabNameCache cached_info{};
				bool have_cache = false;
				{
					std::lock_guard lk(s_name_cache_mut);
					auto cit = s_name_cache.find(prefab_id);
					if (cit != s_name_cache.end()) { cached_info = cit->second; have_cache = true; }
				}

				if (!have_cache) {
					if (name_resolves_left <= 0)
						continue;
					--name_resolves_left;

					std::string prefab_name = get_prefab_name(entity_ptr);
					size_t world_pos = prefab_name.find(" (world)");
					cached_info.prefab_name = prefab_name;
					cached_info.is_droppable = false;
					cached_info.type = EntityType::None;
					if (world_pos != std::string::npos)
					{
						std::string short_name = prefab_name.substr(0, world_pos);
						auto item_it = rust::item_display_names->find(short_name);
						if (item_it != rust::item_display_names->end())
						{
							cached_info.is_droppable = true;
							cached_info.display_name = item_it->second.display_name;
							switch (item_it->second.tier)
							{
							case rust::Rifle:    cached_info.type = EntityType::DroppedRifle; break;
							case rust::Sniper:   cached_info.type = EntityType::DroppedSniper; break;
							case rust::SMG:      cached_info.type = EntityType::DroppedSMG; break;
							case rust::Shotgun:  cached_info.type = EntityType::DroppedShotgun; break;
							case rust::Pistol:   cached_info.type = EntityType::DroppedPistol; break;
							case rust::LMG:      cached_info.type = EntityType::DroppedLMG; break;
							case rust::Launcher: cached_info.type = EntityType::DroppedLauncher; break;
							case rust::Bow:       cached_info.type = EntityType::DroppedBow;      break;
							case rust::Melee:     cached_info.type = EntityType::DroppedMelee;    break;
							case rust::Throwable: cached_info.type = EntityType::DroppedThrowable;break;
							case rust::Tool:      cached_info.type = EntityType::DroppedTool;     break;
							case rust::Medical:   cached_info.type = EntityType::DroppedMedical;  break;
							case rust::Misc:      cached_info.type = EntityType::DroppedAmmo;     break;
							default:              cached_info.type = EntityType::DroppedMisc;     break;
							}
						}
					}
					std::lock_guard lk(s_name_cache_mut);
					s_name_cache.emplace(prefab_id, cached_info);
				}

				if (cached_info.is_droppable)
				{
					found_this_tick.insert(entity_ptr);
					std::unique_lock lock(cache_mut);
					if (entity_list->find(entity_ptr) == entity_list->end())
					{
						rust::Entity* new_entity = new rust::Entity(entity_ptr, prefab_id, cached_info.type);
						new_entity->name = cached_info.display_name;
						new_entity->prefab_name = cached_info.prefab_name;
						(*entity_list)[entity_ptr] = new_entity;
					}
					continue;
				}

				{
					std::lock_guard lk(s_useless_prefabs_mut);
					s_useless_prefabs.insert(prefab_id);
				}
				continue;
			}

			found_this_tick.insert(entity_ptr);

			std::unique_lock lock(cache_mut);
			auto existing = entity_list->find(entity_ptr);

			if (existing != entity_list->end() && existing->second->type == EntityType::None)
			{
				EntityType promoted = (it != prefabs->end()) ? it->second.type : EntityType::None;
				std::string promoted_name;
				if (promoted == EntityType::None) {
					if (klass_known_player(entity_ptr)) {
						promoted = EntityType::Player;
					} else {
						const u64 uid = memory::read<u64>(entity_ptr + offsets::BasePlayer::userID);
						if (uid >= 76561197960265728ULL && uid <= 76561202255233023ULL) {
							promoted = EntityType::Player;
						} else {
							KlassInfo wki{};
							if (klass_known_world(entity_ptr, wki)) {
								promoted = wki.type;
								promoted_name = wki.name;
							}
						}
					}
				}
				if (promoted != EntityType::None) {
					existing->second->type = promoted;
					existing->second->prefab_id = prefab_id;
					if (promoted == EntityType::Player) remember_player_klass(entity_ptr);
					if (promoted != EntityType::Player) {
						if (it != prefabs->end()) existing->second->name = std::string(it->second.name);
						else if (!promoted_name.empty()) existing->second->name = promoted_name;
						else existing->second->name = "Unknown";
					}
				}
				continue;
			}

			if (existing == entity_list->end())
			{
				EntityType resolved_type =
					is_local ? EntityType::Player :
					(it != prefabs->end() ? it->second.type : EntityType::None);
				std::string resolved_name;

				if (resolved_type == EntityType::None)
				{
					if (klass_known_player(entity_ptr)) {
						resolved_type = EntityType::Player;
					} else {
						const u64 uid = memory::read<u64>(entity_ptr + offsets::BasePlayer::userID);
						if (uid >= 76561197960265728ULL && uid <= 76561202255233023ULL) {
							resolved_type = EntityType::Player;
						} else {
							KlassInfo wki{};
							if (klass_known_world(entity_ptr, wki)) {
								resolved_type = wki.type;
								resolved_name = wki.name;
							}
						}
					}
				}

				if (resolved_type == EntityType::Player) remember_player_klass(entity_ptr);

				rust::Entity* new_entity = new rust::Entity(entity_ptr,
					is_local ? EntityPrefabs::player : prefab_id,
					resolved_type);

				if (new_entity->type != EntityType::Player) {
					if (it != prefabs->end()) new_entity->name = std::string(it->second.name);
					else if (!resolved_name.empty()) new_entity->name = resolved_name;
					else new_entity->name = "Unknown";
				}

				const bool is_dropped_item_bucket = is_dropped_bucket(new_entity->type);
				if (is_dropped_item_bucket)
					apply_world_item_identity(new_entity);

				if (debug_esp) new_entity->prefab_name = get_prefab_name(entity_ptr);

				(*entity_list)[entity_ptr] = new_entity;

				if (is_local) {
					game::impl::local_player = new_entity;
				}
			}
			else if (is_local && !game::impl::local_player) {
				game::impl::local_player = (*entity_list)[entity_ptr];
			}
		}

		std::vector<rust::Entity*> doomed;
		doomed.reserve(64);
		{
			std::unique_lock lock(cache_mut);
			for (auto it = entity_list->begin(); it != entity_list->end();)
			{
				if (found_this_tick.count(it->first) == 0)
				{
					if (it->second == game::impl::local_player ||
					    (local_player_ptr && it->first == local_player_ptr))
					{
						found_this_tick.insert(it->first);
						game::impl::local_player = it->second;
						++it;
						continue;
					}
					doomed.push_back(it->second);
					it = entity_list->erase(it);
				}
				else
				{
					++it;
				}
			}
		}
		for (rust::Entity* e : doomed) delete e;
	}

	static void update_entities_internal()
	{
		std::unique_lock guard(s_update_mut, std::try_to_lock);
		if (!guard.owns_lock()) {
			static int s_lock_skip = 0;
			if ((++s_lock_skip % 120) == 1)
				DBG("[cache] update: skipped — gather still holds lock");
			return;
		}

		auto skip_diag = [](const char* why) {
			static int s_skip = 0;
			static const char* s_last = nullptr;
			if (why != s_last || (++s_skip % 60) == 1) {
				s_last = why;
				DBG("[cache] update: skip ({}) in_game={} local={} cam={} vm00={:.3f}",
					why,
					game::is_in_game(),
					game::impl::local_player != nullptr,
					game::impl::camera_object != 0,
					game::impl::view_matrix[0][0]);
			}
		};

		if (!game::is_in_game()) {
			skip_diag("not_in_game");
			return;
		}
		if (!game::impl::local_player) {
			skip_diag("no_local");
			return;
		}
		if (!entity_list) {
			skip_diag("no_list");
			return;
		}
		static bool s_visuals_enabled = false;
		static i32  s_chams_material  = 0;
		static bool s_debug_esp       = false;
		static auto s_gui_cache_last  = std::chrono::steady_clock::time_point{};
		auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - s_gui_cache_last).count() >= 250) {
			s_gui_cache_last  = now;
			s_visuals_enabled = settings.visuals.players.enabled;
			s_chams_material  = settings.visuals.players.chams;
			s_debug_esp       = settings.misc.general.debug_esp;
		}
		const bool visuals_enabled = s_visuals_enabled;
		const i32  chams_material  = s_chams_material;
		const bool chams_enabled   = chams_material != 0 && visuals_enabled;
		const bool debug_esp       = s_debug_esp;

		glm::vec2 window_size = winapp::impl::window_size;

		static std::uint32_t s_update_tick = 0;
		++s_update_tick;

		int diag_players = 0, diag_with_origin = 0, diag_alive = 0;
		for (auto& [ptr, entity] : *entity_list)
		{
			if (!entity) continue;
			if (entity->type == EntityType::Player) {
				++diag_players;
				if (entity->has_origin) ++diag_with_origin;
				if (entity->is_alive()) ++diag_alive;
			}
		}
		{
			static int s_upd_diag = 0;
			if ((++s_upd_diag % 60) == 1) {
				const auto& lo = game::impl::local_player;
				DBG("[cache] update: cached={} players={} origin={} alive={} local_origin={} local_pos=({:.0f},{:.0f},{:.0f}) cam={} visuals={}",
					entity_list->size(), diag_players, diag_with_origin, diag_alive,
					lo && lo->has_origin,
					lo && lo->has_origin ? lo->origin.x : 0.0f,
					lo && lo->has_origin ? lo->origin.y : 0.0f,
					lo && lo->has_origin ? lo->origin.z : 0.0f,
					game::impl::camera_object != 0,
					visuals_enabled);
			}
		}

		for (auto& [ptr, entity] : *entity_list)
		{
			if (!entity) continue;

			if (debug_esp && entity->prefab_name.empty()) {
				entity->prefab_name = get_prefab_name(ptr);
			}

			if (needs_world_item_resolve(entity)) {
				// Item ptr often populates a tick late; keep trying cheaply once
				// the offset is learned, but don't full-scan every frame forever.
				static std::unordered_map<uptr, u8> s_wi_tries;
				u8& tries = s_wi_tries[ptr];
				if (tries < 30 && ((s_update_tick & 3) == 0 || tries < 5)) {
					if (apply_world_item_identity(entity))
						s_wi_tries.erase(ptr);
					else
						++tries;
				}
			}

			if (entity->has_origin
				&& entity->type != EntityType::Player
				&& entity->type != EntityType::Scientist
				&& entity->type != EntityType::Dweller)
			{
				const float d = entity->distance;
				if      (d > 350.0f && (s_update_tick & 15) != 0) continue;
				else if (d > 150.0f && (s_update_tick & 3)  != 0) continue;
			}

			entity->alpha_anim.update(entity->is_alive() ? 1.0f : 0.0f);

			if (entity->type == EntityType::Player ||
				entity->type == EntityType::Scientist ||
				entity->type == EntityType::Dweller)
			{
				auto accept_origin = [&](const glm::vec3& pos, bool allow_jump) -> bool {
					if (!std::isfinite(pos.x) || !std::isfinite(pos.y) || !std::isfinite(pos.z))
						return false;
					if (pos.x == 0.0f && pos.y == 0.0f && pos.z == 0.0f)
						return false;
					if (std::abs(pos.x) >= 10000.0f || std::abs(pos.y) >= 10000.0f || std::abs(pos.z) >= 10000.0f)
						return false;
					// Gate only the PlayerModel fallback — transform jumps are
					// real (cargoship etc). Stale PlayerModel.position flickers
					// while swimming and must not yank ESP around.
					if (!allow_jump && entity->has_origin) {
						const float dx = pos.x - entity->origin.x;
						const float dy = pos.y - entity->origin.y;
						const float dz = pos.z - entity->origin.z;
						if (dx * dx + dy * dy + dz * dz > 25.0f * 25.0f)
							return false;
					}
					entity->origin     = pos;
					entity->has_origin = true;
					return true;
				};

				bool got_origin = false;

				// 1) Unity transform world pos @ access+0x90
				{
					const uptr native = memory::read<uptr>(entity->base_address + offsets::UnityObject::cached_ptr);
					if (native) {
						const uptr go = memory::read<uptr>(native + offsets::UnityComponent::game_object);
						if (go) {
							const uptr comps = memory::read<uptr>(go + offsets::UnityGameObject::components);
							if (comps) {
								const uptr native_transform = memory::read<uptr>(comps + offsets::UnityGameObject::component_ptr_in_entry);
								if (native_transform) {
									const uptr access = memory::read<uptr>(native_transform + offsets::UnityTransform::indirect_ptr_off);
									if (access) {
										glm::vec3 pos{};
										if (try_read_transform_world_pos(access, pos))
											got_origin = accept_origin(pos, /*allow_jump=*/true);
									}
								}
							}
						}
					}
				}

				// 2) PlayerEyes.worldPosition (dump-confirmed @ 0x60)
				if (!got_origin) {
					const uptr eyes_wrap = memory::read(entity->base_address + offsets::BasePlayer::playerEyes);
					uptr eyes = 0;
					if (eyes_wrap) {
						eyes = decryption::player_eyes(eyes_wrap);
						if (!eyes && eyes_wrap > 0x100000000ULL && (eyes_wrap & 1) == 0)
							eyes = eyes_wrap;
					}
					if (eyes && offsets::PlayerEyes::worldPosition) {
						const glm::vec3 pos = memory::read<glm::vec3>(eyes + offsets::PlayerEyes::worldPosition);
						got_origin = accept_origin(pos, /*allow_jump=*/true);
					}
				}

				// 3) PlayerModel.position — try dump value + nearby candidates
				if (!got_origin)
				{
					uptr player_model = memory::read(entity->base_address + offsets::BasePlayer::playerModel);
					if (player_model)
					{
						const uptr pos_offs[] = {
							offsets::PlayerModel::position,
							(uptr)0x2f8, (uptr)0x1e8, (uptr)0x1f8, (uptr)0x208, (uptr)0x310,
						};
						for (uptr off : pos_offs) {
							if (!off) continue;
							const glm::vec3 pos = memory::read<glm::vec3>(player_model + off);
							if (accept_origin(pos, /*allow_jump=*/false)) {
								got_origin = true;
								break;
							}
						}
					}
				}
			}
			else
			{
				auto is_dynamic_world = [](EntityType t) {
					switch (t) {
						case EntityType::Bear: case EntityType::Wolf:    case EntityType::Stag:
						case EntityType::Boar: case EntityType::Horse:   case EntityType::Chicken:
						case EntityType::Shark: case EntityType::Corpse: case EntityType::Scarecrow:
						case EntityType::Motorbike: case EntityType::SidecarMotorbike: case EntityType::PedalBike:
						case EntityType::Snowmobile: case EntityType::TomahaSnowmobile:
						case EntityType::Tugboat: case EntityType::Rowboat: case EntityType::RHIB: case EntityType::Kyak:
						case EntityType::SoloSubmarine: case EntityType::DuoSubmarine:
						case EntityType::Minicopter: case EntityType::ScrapHeli: case EntityType::AttackHeli:
						case EntityType::Elevator: case EntityType::CarLift:
						case EntityType::DroppedRifle: case EntityType::DroppedSniper: case EntityType::DroppedSMG:
						case EntityType::DroppedShotgun: case EntityType::DroppedPistol: case EntityType::DroppedLMG:
						case EntityType::DroppedLauncher: case EntityType::DroppedBow: case EntityType::DroppedMelee:
						case EntityType::DroppedThrowable: case EntityType::DroppedTool: case EntityType::DroppedMedical:
						case EntityType::DroppedAmmo: case EntityType::DroppedMisc:
							return true;
						default: return false;
					}
				};
				if (entity->has_origin && !is_dynamic_world(entity->type)) {
					goto skip_world_pos_read;
				}

				auto sane = [](const glm::vec3& v) {
					return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z) &&
					       (v.x != 0.0f || v.y != 0.0f || v.z != 0.0f) &&
					       std::abs(v.x) < 10000.0f && std::abs(v.y) < 10000.0f && std::abs(v.z) < 10000.0f;
				};

				const uptr native_component = memory::read<uptr>(entity->base_address + offsets::UnityObject::cached_ptr);
				if (native_component) {
					const uptr native_go = memory::read<uptr>(native_component + offsets::UnityComponent::game_object);
					if (native_go) {
						const uptr components = memory::read<uptr>(native_go + offsets::UnityGameObject::components);
						if (components) {
							const uptr native_transform = memory::read<uptr>(components + offsets::UnityGameObject::component_ptr_in_entry);
							if (native_transform) {
								const uptr access = memory::read<uptr>(native_transform + offsets::UnityTransform::indirect_ptr_off);
								if (access) {
									glm::vec3 world_pos;
									if (try_read_transform_world_pos(access, world_pos) && sane(world_pos)) {
										entity->origin     = world_pos;
										entity->has_origin = true;
									}
								}
							}
						}
					}
				}
			}
			skip_world_pos_read:;
			{
				glm::vec3 from{};
				bool have_from = false;
				if (game::impl::local_player && game::impl::local_player->has_origin) {
					from = game::impl::local_player->origin;
					have_from = true;
				} else if (game::impl::camera_object) {
					// Dump: camera.position @ 0x444 on native camera
					const glm::vec3 cam_pos = memory::read<glm::vec3>(game::impl::camera_object + 0x444);
					if (std::isfinite(cam_pos.x) && (cam_pos.x != 0.0f || cam_pos.y != 0.0f || cam_pos.z != 0.0f)
						&& std::abs(cam_pos.x) < 10000.0f) {
						from = cam_pos;
						have_from = true;
					}
				}
				entity->distance = (have_from && entity->has_origin)
					? glm::distance(from, entity->origin) : 0.0f;
			}

			switch (entity->type)
			{
			case EntityType::Player:
			case EntityType::Scientist:
			case EntityType::Dweller:
				entity->cache_bones();
				break;
			}

			if (entity->type == EntityType::Player ||
			    entity->type == EntityType::Scientist ||
			    entity->type == EntityType::Dweller)
			{
				bool got_model_vel = false;
				const uptr pm = memory::read(entity->base_address + offsets::BasePlayer::playerModel);
				if (pm) {
					for (uptr off : { offsets::PlayerModel::newVelocity, offsets::PlayerModel::velocity }) {
						if (!off) continue;
						const glm::vec3 v = memory::read<glm::vec3>(pm + off);
						if (!std::isfinite(v.x) || !std::isfinite(v.y) || !std::isfinite(v.z))
							continue;
						const float speed = glm::length(v);
						if (speed < 50.0f) {
							entity->velocity = v;
							entity->has_velocity = speed > 0.05f;
							entity->last_velocity_update = std::chrono::steady_clock::now();
							got_model_vel = true;
							break;
						}
					}
				}

				if (!got_model_vel && entity->bones && !entity->bones->empty())
				{
					auto head_it = entity->bones->find(rust::BoneList::head);
					if (head_it != entity->bones->end() && head_it->second.visible)
					{
						const glm::vec3 head_pos = head_it->second.position;
						const auto now_v = std::chrono::steady_clock::now();
						if (entity->has_velocity)
						{
							const bool moved = head_pos != entity->prev_origin;
							if (moved) {
								const auto dt_us = std::chrono::duration_cast<std::chrono::microseconds>(
									now_v - entity->last_velocity_update).count();
								if (dt_us > 100 && dt_us < 1000000)
								{
									const float dt_s = dt_us / 1000000.0f;
									const glm::vec3 raw_v = (head_pos - entity->prev_origin) / dt_s;
									if (glm::length(raw_v) < 50.0f) {
										constexpr float alpha = 0.7f;
										entity->velocity = alpha * raw_v + (1.0f - alpha) * entity->velocity;
									}
									entity->prev_origin = head_pos;
									entity->last_velocity_update = now_v;
								}
							}
						}
						else
						{
							entity->prev_origin = head_pos;
							entity->last_velocity_update = now_v;
							entity->has_velocity = true;
						}
					}
				}
			}


			if (entity->type == EntityType::Player)
			{
				const bool is_near    = entity->has_origin && entity->distance < 100.0f;
				const bool is_visible = entity->has_origin && entity->distance < 350.0f;

				if (entity->base_address != game::impl::local_player->base_address)
				{
					bool state_changed = (chams_enabled != entity->was_chams_enabled);
					bool should_refresh = (chams_enabled && std::chrono::duration_cast<std::chrono::milliseconds>(now - entity->last_material_update).count() > 500);

					if ((state_changed || should_refresh) && is_visible)
					{
						set_material(entity, g_materials[chams_material], chams_enabled);
						entity->last_material_update = now;
						entity->was_chams_enabled = chams_enabled;
					}
				}

				entity->flags = memory::read<i32>(entity->base_address + offsets::BasePlayer::playerFlags);

				entity->is_sleeping = (entity->flags & PlayerFlags::Sleeping) != 0;
				entity->is_wounded = (entity->flags & PlayerFlags::Wounded) != 0;
				entity->is_spectating = (entity->flags & PlayerFlags::Spectating) != 0;
				entity->is_connected = (entity->flags & PlayerFlags::Connected) != 0;
				entity->is_incapacitated = (entity->flags & PlayerFlags::Incapacitated) != 0;
				entity->is_aiming = (entity->flags & PlayerFlags::Aiming) != 0;

				entity->health = memory::read<f32>(entity->base_address + offsets::BaseCombatEntity::_health);
				entity->max_health = memory::read<f32>(entity->base_address + offsets::BaseCombatEntity::_maxHealth);
				if (entity->max_health <= 1.0f)
					entity->max_health = 100.0f;

				const uptr ms = memory::read<uptr>(entity->base_address + offsets::BasePlayer::modelState);
				if (ms) {
					const i32 ms_flags = memory::read<i32>(ms + offsets::model_state::flags);
					entity->is_crouching = (ms_flags & offsets::model_state::Ducked) != 0;
					entity->is_prone = (ms_flags & offsets::model_state::Prone) != 0;
					entity->is_scoped = (ms_flags & offsets::model_state::Aiming) != 0 || entity->is_aiming;
				} else {
					entity->is_crouching = false;
					entity->is_prone = false;
					entity->is_scoped = entity->is_aiming;
				}

				if (entity->name.empty())
				{
					uptr base_display_name = memory::read(ptr + offsets::BasePlayer::username);
					i32 len = memory::read<i32>(base_display_name + 0x10);
					std::wstring player_name = memory::read_wstring<2>(base_display_name + 0x14, len);
					entity->name = std::string(player_name.begin(), player_name.end());
				}

				if (entity->steam_id == 0)
				{
					u64 raw = memory::read<u64>(entity->base_address + offsets::BasePlayer::userID);
					if (raw >= 76561197960265728ULL && raw <= 76561202255233023ULL) {
						entity->steam_id = raw;
					}
				}

				if (entity->steam_id != 0 &&
				    entity->avatar_status == rust::Entity::AvatarStatus::None &&
				    is_near)
				{
					entity->avatar_status = rust::Entity::AvatarStatus::Pending;
					g_avatar_manager.request(entity->steam_id);
				}

				if (entity->avatar_status == rust::Entity::AvatarStatus::Pending)
				{
					auto buffer = g_avatar_manager.pop_result(entity->steam_id);
					if (!buffer.empty())
					{
						if (avatar_manager_t::is_sentinel(buffer)) {
							entity->avatar_status = rust::Entity::AvatarStatus::Failed;
						}
						else if (entity->avatar_texture.load_from_memory(buffer.data(), buffer.size())) {
							entity->avatar_status = rust::Entity::AvatarStatus::Ready;
						}
						else {
							entity->avatar_status = rust::Entity::AvatarStatus::Failed;
						}
					}
				}

				entity->team = memory::read<u64>(entity->base_address + offsets::BasePlayer::team);


				entity->life_state = memory::read<u8>(entity->base_address + offsets::BaseCombatEntity::lifeState);
				entity->player_input = memory::read(entity->base_address + offsets::BasePlayer::playerInput);

				if (entity->base_address == game::impl::local_player->base_address)
				{
					entity->view_angles = entity->player_input ? memory::read<glm::vec3>(entity->player_input + offsets::PlayerInput::bodyAngles) : glm::vec3{};
				}
				else
				{
					uptr native = memory::read(entity->base_address + 0x10);
					if (native)
					{
						uptr game_object = memory::read(native + 0x20);
						if (game_object)
						{
							uptr eyes = unity::get_component_by_id(game_object, 4);
							if (eyes)
							{
								glm::vec4 rotation = memory::read<glm::vec4>(eyes + offsets::PlayerEyes::bodyRotation);
								entity->view_angles = math::quaternion_to_euler(rotation);
							}
						}
					}
				}

				const bool is_local_ent = (entity->base_address == game::impl::local_player->base_address);

				int inv_budget_ms = is_local_ent ? 50 : 250;
				if (!is_local_ent && entity->has_origin) {
					if (entity->distance > 350.0f)      continue;
					else if (entity->distance > 150.0f) inv_budget_ms = 1000;
				}
				const bool inv_due = std::chrono::duration_cast<std::chrono::milliseconds>(
					now - entity->last_inventory_update).count() >= inv_budget_ms;
				if (!inv_due)
					continue;
				entity->last_inventory_update = now;

				entity->get_belt_items();
				entity->belt_items.clear();

				entity->active_item = entity->get_held_item();

				std::string active_short;
				for (i32 i = 0; i < 6; i++) {
					uptr item = entity->held_items[i];
					if (!item) continue;

					std::string shortname = get_item_name_short(item);
					texture_t* tex = nullptr;
					if (!shortname.empty()) {
						auto tex_it = g_item_textures->find(shortname);
						if (tex_it != g_item_textures->end() && tex_it->second)
							tex = tex_it->second.get();
						if (!tex)
							tex = bundle_icons::for_item_shortname(shortname);
					}
					const bool is_active_slot = (item == entity->active_item) && (entity->active_item != 0);
					std::int32_t amt = memory::read<std::int32_t>(item + offsets::Item::amount);
					if (amt < 0 || amt > 10000000) amt = 1;
					entity->belt_items.push_back({ shortname, tex, amt, is_active_slot, item });
				}

				if (entity->active_item)
					active_short = get_item_name_short(entity->active_item);
				entity->item_shortname = active_short;
				entity->item_name = entity->active_item
					? (active_short.empty() ? std::string("Item") : active_short)
					: std::string("Empty");
				entity->item_texture = nullptr;
				if (!active_short.empty()) {
					auto tex_it = g_item_textures->find(active_short);
					if (tex_it != g_item_textures->end() && tex_it->second)
						entity->item_texture = tex_it->second.get();
					if (!entity->item_texture)
						entity->item_texture = bundle_icons::for_item_shortname(active_short);
				}
			}
		}
	}

	void update()
	{
		if (!game::is_in_game()) return;

		auto pending = g_item_texture_manager.drain();
		for (auto& [shortname, bytes] : pending) {
			if (shortname.empty() || bytes.empty()) continue;
			if (g_item_textures->count(shortname)) continue;
			auto tex = std::make_unique<texture_t>();
			if (tex->load_from_memory(bytes.data(), bytes.size()))
				(*g_item_textures)[shortname] = std::move(tex);
		}

		g_avatar_manager.load_default_into(render::steam_avatar);
	}

	rust::Entity* get_local_player()
	{
		return game::impl::local_player;
	}
}
