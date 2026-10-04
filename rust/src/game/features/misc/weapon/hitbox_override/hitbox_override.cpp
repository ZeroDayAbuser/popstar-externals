#include "hitbox_override.hpp"
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>
#include <game/game.hpp>
#include <game/cache/cache.hpp>
#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <game/features/feature_util.hpp>
#include <string_encryption.hpp>
#include <array>
#include <cstdint>
#include <print>
#include <utils/debug.hpp>
#include <string>
#include <vector>

namespace features::misc::hitbox_override {

    static const char* const BONE_FOR_DROPDOWN[] = {
        "head", "neck", "spine4", "spine2", "pelvis", "l_hip",
    };

    static std::string read_il2cpp_string(uptr addr)
    {
        if (!addr) return {};
        auto len = memory::read<std::int32_t>(addr + 0x10);
        if (len <= 0 || len > 128) return {};
        auto ws = memory::read_wstring<2>(addr + 0x14, static_cast<std::size_t>(len));
        std::string out;
        out.reserve(static_cast<std::size_t>(len));
        for (auto c : ws) {
            if (c > 0 && c < 128) out.push_back(static_cast<char>(c));
        }
        return out;
    }

    static bool looks_ptr(uptr p)
    {
        return p > 0x10000ULL && p < 0x00007FFFFFFFFFFFULL && (p & 1) == 0;
    }

    // Probe BoneProperty-like object for an Il2CppString name matching dropdown.
    static int match_bone_name(uptr obj)
    {
        if (!looks_ptr(obj)) return -1;
        constexpr uptr kNameOffs[] = { 0x10, 0x14, 0x18, 0x20, 0x28, 0x30, 0x38 };
        for (uptr off : kNameOffs) {
            const uptr sp = memory::read<uptr>(obj + off);
            if (!looks_ptr(sp)) continue;
            const auto name = read_il2cpp_string(sp);
            if (name.empty()) continue;
            for (int t = 0; t < 6; ++t) {
                if (name == BONE_FOR_DROPDOWN[t])
                    return t;
            }
        }
        return -1;
    }

    struct Slot {
        uptr          addr;     // absolute address of value pointer in dict entry
        uptr          original; // original BoneProperty*
    };

    void tick()
    {
        using cache::g_cache_epoch;

        static std::vector<Slot> slots;
        static std::array<uptr, 6> target_obj{};
        static bool populated = false;
        static int  last_selection = -1;
        static bool dirty = false;
        static std::uint32_t seen_epoch = 0;

        const std::uint32_t cur_epoch = g_cache_epoch.load(std::memory_order_acquire);
        if (cur_epoch != seen_epoch) {
            if (dirty) {
                for (const auto& s : slots)
                    memory::write<uptr>(s.addr, s.original);
                dirty = false;
            }
            slots.clear();
            target_obj.fill(0);
            populated = false;
            last_selection = -1;
            seen_epoch = cur_epoch;
        }

        const bool enabled = settings.aimbot.weapons.hitbox_override;

        if (!enabled) {
            if (dirty) {
                for (const auto& s : slots)
                    memory::write<uptr>(s.addr, s.original);
                dirty = false;
                last_selection = -1;
            }
            return;
        }

        if (offsets::skeleton_properties::quickLookup == 0) {
            static int s = 0;
            if ((++s % 300) == 1) DBG("[hitbox] quickLookup offset missing");
            return;
        }

        if (!populated) {
            const uptr local = features::local_base_or_zero();
            if (!local) return;

            const uptr skel = memory::read<uptr>(local + offsets::BaseCombatEntity::skeletonProperties);
            if (!skel) {
                static int s = 0;
                if ((++s % 300) == 1) DBG("[hitbox] skeletonProperties=0");
                return;
            }

            const uptr dict = memory::read<uptr>(skel + offsets::skeleton_properties::quickLookup);
            if (!dict) {
                static int s = 0;
                if ((++s % 300) == 1) DBG("[hitbox] quickLookup dict=0");
                return;
            }

            uptr ents = memory::read<uptr>(dict + 0x18);
            auto length = memory::read<std::int32_t>(dict + 0x20);
            if (!ents || length <= 0 || length > 4096) {
                ents = memory::read<uptr>(dict + offsets::item_icon::dict_pointer);
                length = memory::read<std::int32_t>(ents ? ents + offsets::item_icon::dict_count : 0);
            }
            if (!ents || length <= 0 || length > 4096) {
                static int s = 0;
                if ((++s % 300) == 1) DBG("[hitbox] bone dict walk failed");
                return;
            }

            slots.clear();
            target_obj.fill(0);
            int named = 0;

            for (std::int32_t i = 0; i < length; ++i) {
                const uptr entry = ents + 0x20 + static_cast<uptr>(i) * 0x18;
                const auto key = memory::read<std::uint32_t>(entry + 0x08);
                const uptr val = memory::read<uptr>(entry + 0x10);
                if (!key || !looks_ptr(val)) continue;

                slots.push_back({ entry + 0x10, val });

                const int idx = match_bone_name(val);
                if (idx >= 0 && !target_obj[idx]) {
                    target_obj[idx] = val;
                    ++named;
                }
            }

            // Fallback: if names weren't on the value object, try StringPool for name->hash,
            // then match dict keys. Only used when typeinfo is alive.
            if (named < 3 && offsets::string_pool::typeinfo) {
                const uptr sp_klass = memory::read<uptr>(game::impl::game_assembly + offsets::string_pool::typeinfo);
                if (looks_ptr(sp_klass)) {
                    const uptr sp_sf = memory::read<uptr>(sp_klass + offsets::string_pool::static_fields);
                    const uptr tn = sp_sf ? memory::read<uptr>(sp_sf + offsets::string_pool::toNumber) : 0;
                    if (looks_ptr(tn)) {
                        uptr sp_ents = memory::read<uptr>(tn + 0x18);
                        auto sp_len = memory::read<std::int32_t>(tn + 0x20);
                        if (looks_ptr(sp_ents) && sp_len > 0 && sp_len < 200000) {
                            std::array<std::uint32_t, 6> want{};
                            for (std::int32_t i = 0; i < sp_len; ++i) {
                                const uptr e = sp_ents + 0x20 + static_cast<uptr>(i) * 0x18;
                                const uptr key_ptr = memory::read<uptr>(e + 0x08);
                                const auto h = memory::read<std::uint32_t>(e + 0x10);
                                if (!key_ptr || !h) continue;
                                const auto name = read_il2cpp_string(key_ptr);
                                for (int t = 0; t < 6; ++t) {
                                    if (!want[t] && name == BONE_FOR_DROPDOWN[t])
                                        want[t] = h;
                                }
                            }
                            // Map hash -> value from skeleton dict
                            for (std::int32_t i = 0; i < length; ++i) {
                                const uptr entry = ents + 0x20 + static_cast<uptr>(i) * 0x18;
                                const auto key = memory::read<std::uint32_t>(entry + 0x08);
                                const uptr val = memory::read<uptr>(entry + 0x10);
                                for (int t = 0; t < 6; ++t) {
                                    if (want[t] && key == want[t] && !target_obj[t] && looks_ptr(val)) {
                                        target_obj[t] = val;
                                        ++named;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (slots.empty() || !target_obj[0]) {
                static int s = 0;
                if ((++s % 300) == 1)
                    DBG("[hitbox] populate fail slots={} named={} (need head at least)",
                        slots.size(), named);
                return;
            }

            populated = true;
            DBG("[hitbox] ready slots={} named={} head=0x{:x}",
                slots.size(), named, target_obj[0]);
        }

        const int sel = settings.aimbot.weapons.hitbox_override_bone;
        if (sel < 0 || sel >= 6) return;

        uptr dest = target_obj[sel];
        if (!dest) dest = target_obj[0]; // fall back to head
        if (!dest) return;

        if (dirty && sel == last_selection) return;

        for (const auto& s : slots)
            memory::write<uptr>(s.addr, dest);
        dirty = true;
        last_selection = sel;
    }
}
