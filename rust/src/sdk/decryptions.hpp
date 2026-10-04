#pragma once
#include "globals.hpp"
#include "rust/gchandle.hpp"
#include "obfuscation/build_obfuscation.hpp"
#include <bit>
#include <cstring>
#include <memory/memory.hpp>
#include <array>

namespace decryption {

namespace detail {

inline bool heap_ok(uptr p) {
    return p >= 0x100000000ULL && p <= 0x00007FFFFFFFFFFFULL;
}

inline void t_client_entities(uint32_t& v) {
    // BaseNetworkable1 — 25354344
    v -= OB(0x7226E3E6u);
    v ^= OB(0x2BB75F5Eu);
    v = std::rotl(v, 25);
}

inline void t_entity_list(uint32_t& v) {
    // BaseNetworkable2 — 25354344
    v += OB(0x30F40426u);
    v = std::rotl(v, 19);
    v ^= OB(0x41C539DDu);
    v = std::rotl(v, 1);
}

inline void t_cl_active_item(uint32_t& v) {
   
    v ^= OB(0xBB08A3FAu);
    v += OB(0x56329D44u);
    v = std::rotl(v, 27);
    
}

inline void t_player_inventory(uint32_t& v) {
    v -= OB(0x56329D44u);
    v ^= OB(0xA4101D7Au);
    v -= OB(0x56329D44u);
    v ^= OB(0xA4101D7Au);
}

inline void t_player_eyes(uint32_t& v) {
   
    v ^= OB(0x4587203B);
   
    v -= OB(0x2BEF67CE);
    v ^= OB(0x2F601E25);
}

inline void t_local_player(uint32_t& v) { // Same as client entites / bn1 decrypt :)
    v -= OB(0x7226E3E6u);
    v ^= OB(0x2BB75F5Eu);
    v = std::rotl(v, 25);
}

template <typename Transform>
inline uptr decrypt_wrapper(uint64_t wrapper_obj, Transform&& transform, bool require_hidden = false) {
    if (!heap_ok(wrapper_obj))
        return 0;
    const uptr base = static_cast<uptr>(wrapper_obj);

    const uint8_t has_value = memory::read<uint8_t>(base + 0x10);
    if (!has_value) {
        if (require_hidden)
            return 0;
        const uptr raw = memory::read<uptr>(base + 0x18);
        return heap_ok(raw) ? raw : 0;
    }

    uint64_t rax = memory::read<uintptr_t>(base + 0x18);
    if (!rax)
        return 0;
    uint32_t* words = reinterpret_cast<uint32_t*>(&rax);
    for (int i = 0; i < 2; ++i)
        transform(words[i]);
    return gchandle::get_target(rax);
}

}

inline uptr base_networkable_0(uint64_t a1) {
    return detail::decrypt_wrapper(a1, detail::t_client_entities, true);
}

inline uptr base_networkable_1(uint64_t a1) {
    return detail::decrypt_wrapper(a1, detail::t_entity_list, true);
}

inline uptr client_entities(uint64_t a1) {
    return base_networkable_0(a1);
}

inline uptr entity_list(uint64_t a1) {
    return base_networkable_1(a1);
}

inline uptr local_player(uint64_t a1) {
    return detail::decrypt_wrapper(a1, detail::t_local_player, true);
}

inline uint64_t cl_active_item(uint64_t a1) {
    auto words = std::bit_cast<std::array<std::uint32_t, 2>>(a1);
    for (std::size_t i = 0; i < words.size(); ++i)
        detail::t_cl_active_item(words[i]);
    return std::bit_cast<std::uint64_t>(words);
}

inline uptr player_inventory(uint64_t a1) {
    if (!detail::heap_ok(a1))
        return 0;

    if (!memory::read<uint8_t>(static_cast<uptr>(a1) + 0x10))
        return 0;
    return detail::decrypt_wrapper(a1, detail::t_player_inventory);
}

inline uptr player_inventory_alt(uint64_t a1) {

    if (detail::heap_ok(a1) && memory::read<uptr>(static_cast<uptr>(a1)) != 0)
        return static_cast<uptr>(a1);
    return player_inventory(a1);
}

inline uptr player_eyes(uint64_t a1) {
    if (!detail::heap_ok(a1))
        return 0;
    if (!memory::read<uint8_t>(static_cast<uptr>(a1) + 0x10))
        return 0;
    return detail::decrypt_wrapper(a1, detail::t_player_eyes);
}

inline uptr local_player_decrypt(uint64_t a1) {
    return local_player(a1);
}

inline uint32_t decrypt_fov(uint32_t val) {
    val ^= OB(0xD4C0E73Fu);
    val = std::rotl(val, 23);
    val -= OB(0xA397D23u);
    val ^= OB(0x33EBED5Au);
    return val;
}

inline uint32_t encrypt_fov(float input) {
    uint32_t val;
    std::memcpy(&val, &input, sizeof(val));
    val ^= OB(0x33EBED5Au);
    val += OB(0xA397D23u);
    val = std::rotr(val, 23);
    val ^= OB(0xD4C0E73Fu);
    return val;
}

}
