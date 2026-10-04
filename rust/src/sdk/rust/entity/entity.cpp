#include "entity.hpp"
#include <algorithm>
#include <cstring>
#include <immintrin.h>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <unordered_map>
#include <vector>

#include <print>
#include <utils/debug.hpp>
#include <glm/gtc/quaternion.hpp>

#include <sdk/decryptions.hpp>
#include <sdk/unity/unity.hpp>
#include <game/game.hpp>

namespace rust
{
	void Entity::cache_bones()
	{
		if (!base_address) return;

		int budget_ms = 8;
		if (has_origin) {
			if      (distance > 250.0f) budget_ms = 100;
			else if (distance > 100.0f) budget_ms = 33;
		}

		const auto now = std::chrono::steady_clock::now();
		if (bones_initialized) {
			if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_bone_update).count() < budget_ms)
				return;
		}

		last_bone_update = now;
		if (!bones) return;

		auto soft_fail  = [this]() { ++transient_fail_ticks; };
		auto soft_reset = [this]() { transient_fail_ticks = 0; };

		if (bones_initialized) {
			const uptr current_model = memory::read<uptr>(base_address + offsets::BaseEntity::model);
			if (current_model != cached_model_ptr) {
				bones_initialized = false;
				if (bones) bones->clear();
				soft_fail();
			} else if (current_model) {
				const uptr current_transforms = memory::read<uptr>(current_model + offsets::Model::boneTransforms);
				if (current_transforms && current_transforms != cached_transforms_ptr) {
					bones_initialized = false;
					soft_fail();
				} else if (!cached_bones.empty() && cached_shared_transform_data) {
					const uptr first_xfm = cached_bones.front().transform_ptr;
					const uptr live_shared = memory::read<uptr>(first_xfm + cached_tr_data_off);
					if (live_shared && live_shared != cached_shared_transform_data) {
						bones_initialized = false;
						soft_fail();
					} else {
						const uptr live_matrix_base = memory::read<uptr>(cached_shared_transform_data + 0x18);
						if (live_matrix_base && live_matrix_base != cached_matrix_base) {
							bones_initialized = false;
							soft_fail();
						}
					}
				}
			}
		}

		if (!bones_initialized)
		{
			cached_bones.clear();

			const uptr model = memory::read<uptr>(base_address + offsets::BaseEntity::model);
			if (!model) { soft_fail(); return; }
			cached_model_ptr = model;

			const uptr transforms = memory::read<uptr>(model + offsets::Model::boneTransforms);
			if (!transforms) { soft_fail(); return; }
			cached_transforms_ptr = transforms;

			const i32 bone_count = memory::read<i32>(transforms + 0x18);
			const uptr bone_array_ptr = transforms + 0x20;

			if (bone_count <= 0 || bone_count > 512) { soft_fail(); return; }

			std::vector<uptr> entity_bone_ptrs(bone_count);
			const std::size_t array_size = bone_count * sizeof(uptr);
			if (!memory::read_memory_raw(bone_array_ptr, entity_bone_ptrs.data(), array_size)) { soft_fail(); return; }

			constexpr BoneList bones_to_cache[] = {
				BoneList::pelvis,
				BoneList::l_hip, BoneList::l_knee, BoneList::l_foot,
				BoneList::r_hip, BoneList::r_knee, BoneList::r_foot,
				BoneList::spine1, BoneList::spine2, BoneList::spine3, BoneList::spine4,
				BoneList::neck, BoneList::head,
				BoneList::l_clavicle, BoneList::l_upperarm, BoneList::l_forearm, BoneList::l_hand,
				BoneList::r_clavicle, BoneList::r_upperarm, BoneList::r_forearm, BoneList::r_hand
			};

			struct TrOff { uptr data; uptr index; };
			constexpr TrOff kTrCandidates[] = {
				{ 0x28, 0x30 },
				{ offsets::UnityTransform::transform_data_ptr, offsets::UnityTransform::transform_highest_index },
			};

			auto looks_heap_ptr = [](uptr p) -> bool {
				return p > 0x10000ULL && p < 0x00007FFFFFFFFFFFULL && (p & 0x7) == 0;
			};

			constexpr size_t kRequested = sizeof(bones_to_cache) / sizeof(bones_to_cache[0]);
			constexpr size_t kMinAccepted = (kRequested * 17) / 20;
			constexpr i32 buffer_entries{ 4096 };

			bool inited = false;
			for (const TrOff cand : kTrCandidates)
			{
				std::vector<CachedBone> trial;
				trial.reserve(kRequested);
				uptr shared_transform_data = 0;
				int valid_indices = 0;

				for (auto id : bones_to_cache)
				{
					if (static_cast<i32>(id) >= bone_count) continue;

					const uptr ent_bone = entity_bone_ptrs[static_cast<i32>(id)];
					if (!ent_bone) continue;

					const uptr transform_ptr = memory::read<uptr>(ent_bone + 0x10);
					if (!transform_ptr) continue;

					CachedBone cb{};
					cb.id = id;
					cb.transform_ptr = transform_ptr;
					cb.index = memory::read<i32>(transform_ptr + cand.index);

					const uptr transform_data = memory::read<uptr>(transform_ptr + cand.data);
					if (!looks_heap_ptr(transform_data)) continue;
					if (cb.index < 0 || cb.index >= buffer_entries) continue;

					shared_transform_data = transform_data;
					++valid_indices;
					trial.push_back(cb);
				}

				if (trial.size() < kMinAccepted || !shared_transform_data || valid_indices < (int)kMinAccepted)
					continue;

				const uptr index_base = memory::read<uptr>(shared_transform_data + 0x20);
				const uptr matrix_base = memory::read<uptr>(shared_transform_data + 0x18);
				if (!looks_heap_ptr(index_base) || !looks_heap_ptr(matrix_base))
					continue;

				i32 current_max_index = 0;
				for (auto& cb : trial)
				{
					if (cb.index > current_max_index) current_max_index = cb.index;

					i32 transform_index = memory::read<i32>(index_base + cb.index * sizeof(i32));
					int depth = 0;
					while (transform_index >= 0 && transform_index < buffer_entries && depth++ < 128)
					{
						cb.hierarchy.push_back(transform_index);
						if (transform_index > current_max_index) current_max_index = transform_index;
						transform_index = memory::read<i32>(index_base + transform_index * sizeof(i32));
					}
				}

				cached_bones = std::move(trial);
				cached_matrix_base = matrix_base;
				cached_shared_transform_data = shared_transform_data;
				cached_tr_data_off = cand.data;
				cached_tr_index_off = cand.index;
				cb_max_index = current_max_index;
				bones_initialized = true;
				inited = true;
				break;
			}

			if (!inited) {
				cached_bones.clear();
				soft_fail();
				return;
			}
		}

		if (!bones_initialized || !cached_matrix_base) { soft_fail(); return; }

		constexpr i32 buffer_entries{ 4096 };

		struct AoSTransform {
			glm::vec3 position;   float _pad0;
			glm::quat rotation;
			glm::vec3 scale;      float _pad1;
		};
		static_assert(sizeof(AoSTransform) == 0x30, "AoS transform must be 48 bytes");

		auto sane_world_pos = [](const glm::vec3& v) -> bool {
			if (!std::isfinite(v.x) || !std::isfinite(v.y) || !std::isfinite(v.z))
				return false;
			constexpr float kMapBound = 5000.0f;
			return std::abs(v.x) < kMapBound
			    && std::abs(v.y) < kMapBound
			    && std::abs(v.z) < kMapBound;
		};

		auto sane_quat = [](const glm::quat& q) -> bool {
			if (!std::isfinite(q.x) || !std::isfinite(q.y) ||
			    !std::isfinite(q.z) || !std::isfinite(q.w)) return false;
			const float len2 = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w;
			return len2 > 0.90f && len2 < 1.10f;
		};

		auto sane_scale = [](const glm::vec3& s) -> bool {
			if (!std::isfinite(s.x) || !std::isfinite(s.y) || !std::isfinite(s.z)) return false;
			return std::abs(s.x) < 100.0f && std::abs(s.y) < 100.0f && std::abs(s.z) < 100.0f;
		};

		const size_t entries_needed = static_cast<size_t>(cb_max_index) + 1;
		const size_t snap_bytes = entries_needed * sizeof(AoSTransform);
		thread_local std::vector<AoSTransform> snap;
		snap.resize(entries_needed);

		if (!memory::read_memory_raw(cached_matrix_base, snap.data(), snap_bytes)) {
			// Keep last bone positions — clearing here is what makes ESP
			// flicker hard while swimming (matrix reads fail more often).
			if (++bone_read_failures >= 8) {
				bones_initialized = false;
				bone_read_failures = 0;
			}
			soft_fail();
			return;
		}
		bone_read_failures = 0;

		auto safe_get = [&](i32 idx) -> const AoSTransform* {
			if (idx < 0 || idx >= (i32)entries_needed) return nullptr;
			return &snap[idx];
		};

		std::unordered_map<BoneList, BoneData> staged;
		staged.reserve(cached_bones.size());

		int attempted = 0, accepted = 0;
		for (const auto& cb : cached_bones)
		{
			const AoSTransform* bone_xfm = safe_get(cb.index);
			if (!bone_xfm) continue;
			if (!sane_quat(bone_xfm->rotation) || !sane_scale(bone_xfm->scale)) continue;
			glm::vec3 world_pos = bone_xfm->position;

			bool ok = true;
			for (i32 parent_idx : cb.hierarchy)
			{
				const AoSTransform* p = safe_get(parent_idx);
				if (!p) { ok = false; break; }
				if (!sane_quat(p->rotation) || !sane_scale(p->scale)) { ok = false; break; }
				if (!sane_world_pos(p->position)) { ok = false; break; }

				const glm::quat r       = glm::normalize(p->rotation);
				const glm::vec3 scaled  = world_pos * p->scale;
				const glm::vec3 rotated = r * scaled;
				world_pos               = p->position + rotated;
			}
			++attempted;

			if (ok && sane_world_pos(world_pos) && has_origin)
			{
				const glm::vec3 d = world_pos - origin;
				// 100m — swimming / stale PlayerModel.position used to push
				// bones just past the old 50m gate every other tick.
				if (d.x*d.x + d.y*d.y + d.z*d.z > 10000.0f) ok = false;
			}

			if (ok && sane_world_pos(world_pos))
			{
				staged[cb.id] = { world_pos, true, now };
				++accepted;
			}
		}

		const size_t min_publish = (cached_bones.size() * 3) / 4;
		const bool full_enough = (accepted >= (int)min_publish);
		if (bones) {
			for (auto& [id, bd] : staged) (*bones)[id] = bd;

			constexpr auto kBoneStaleTtl = std::chrono::milliseconds(500);
			for (auto& [id, bd] : *bones) {
				if (!bd.visible) continue;
				if (now - bd.last_seen > kBoneStaleTtl) bd.visible = false;
			}
		}
		if (full_enough) soft_reset(); else soft_fail();

		if (attempted >= 8 && accepted == 0) {
			if (all_reject_start.time_since_epoch().count() == 0)
				all_reject_start = now;
			else if (std::chrono::duration_cast<std::chrono::milliseconds>(now - all_reject_start).count() >= 1500) {
				bones_initialized = false;
				all_reject_start = {};
			}
		} else {
			all_reject_start = {};
		}
		stale_cache_ticks = 0;
		partial_fail_ticks = 0;
	}

	uptr Entity::get_container_contents(uptr container_offset)
	{
		if (!cached_inventory)
		{
			cached_inventory = unity::get_component_by_name(base_address, "PlayerInventory");
			if (!cached_inventory) return 0;
		}

		uptr container = memory::read(cached_inventory + container_offset);
		if (!container) return 0;

		uptr item_list = memory::read(container + offsets::ItemContainer::list);
		if (!item_list) return 0;

		return memory::read(item_list + 0x10);
	}

	void Entity::get_belt_items()
	{
		constexpr i32 belt_slots = 6;

		held_items_count = 0;
		for (i32 i = 0; i < belt_slots; i++) held_items[i] = 0;

		const bool is_local = (game::impl::local_player &&
		                       game::impl::local_player->base_address == base_address);
		static bool s_logged_no_component = false;
		static bool s_logged_no_container = false;
		static bool s_logged_no_item_list = false;
		static bool s_logged_no_contents  = false;
		static bool s_logged_success      = false;

		if (!cached_inventory)
		{
			cached_inventory = unity::get_component_by_name(base_address, "PlayerInventory");
			if (!cached_inventory) {
				if (is_local && !s_logged_no_component) {
					s_logged_no_component = true;
					DBG("[belt] local get_component_by_name('PlayerInventory') = 0 "
					             "— class name likely obfuscated in metadata");
				}
				return;
			}
		}

		uptr container = memory::read(cached_inventory + offsets::PlayerInventory::containerBelt);
		if (!container) {
			if (is_local && !s_logged_no_container) {
				s_logged_no_container = true;
				DBG("[belt] local containerBelt (@inv+0x{:x}) = 0",
				             (uptr)offsets::PlayerInventory::containerBelt);
			}
			cached_inventory = 0;
			return;
		}

		uptr item_list = memory::read(container + offsets::ItemContainer::list);
		if (!item_list) {
			if (is_local && !s_logged_no_item_list) {
				s_logged_no_item_list = true;
				DBG("[belt] local ItemContainer::list (@container+0x{:x}) = 0",
				             (uptr)offsets::ItemContainer::list);
			}
			return;
		}

		uptr contents = memory::read(item_list + 0x10);
		if (!contents) {
			if (is_local && !s_logged_no_contents) {
				s_logged_no_contents = true;
				DBG("[belt] local list contents (@list+0x10) = 0");
			}
			return;
		}

		i32 stored = memory::read<i32>(item_list + offsets::ItemContainer::itemCount);
		i32 count = (stored > 0 && stored <= belt_slots) ? stored : belt_slots;

		uptr items[belt_slots]{};
		if (!memory::read_memory_raw(contents + 0x20, items, count * sizeof(uptr)))
			return;

		for (i32 i = 0; i < count; i++) {
			held_items[i] = items[i];
			if (items[i]) held_items_count++;
		}

		if (is_local && !s_logged_success && held_items_count > 0) {
			s_logged_success = true;
			DBG("[belt] local belt resolved: container=0x{:x} list=0x{:x} contents=0x{:x} "
			             "stored={} items=[0x{:x} 0x{:x} 0x{:x} 0x{:x} 0x{:x} 0x{:x}]",
			             container, item_list, contents, stored,
			             items[0], items[1], items[2], items[3], items[4], items[5]);
		}
	}

	uptr Entity::get_held_item()
	{
		constexpr u32 kEntityFlagHeldActive = 0x400;

		auto held_from_item = [](uptr item) -> uptr {
			if (!item) return 0;
			uptr held = memory::read<uptr>(item + offsets::Item::heldEntity);
			if (!held && offsets::Item::held_entity_alt)
				held = memory::read<uptr>(item + offsets::Item::held_entity_alt);
			return held;
		};

		for (const auto& item : held_items)
		{
			uptr held_entity = held_from_item(item);
			if (!held_entity) continue;

			u32 entity_flags = memory::read<u32>(held_entity + offsets::BaseEntity::flags);
			if (entity_flags & kEntityFlagHeldActive)
				return item;
		}

		if (base_address) {
			const u64 enc = memory::read<u64>(base_address + offsets::BasePlayer::clActiveItem);
			if (enc) {
				const u64 uid = decryption::cl_active_item(enc);
				if (uid) {
					for (const auto& item : held_items) {
						if (!item) continue;
						if (memory::read<u64>(item + offsets::Item::uid) == uid)
							return item;
					}
				}
			}
		}

		return 0;
	}
}
