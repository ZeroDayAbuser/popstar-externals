#pragma once
#include <sdk/globals.hpp>
#include <memory/memory.hpp>

#include <cstdint>
#include <cstring>

namespace mvcrypt {

inline constexpr uint32_t rol32(uint32_t v, int n) {
    n &= 31;
    return (v << n) | (v >> ((32 - n) & 31));
}
inline constexpr uint32_t ror32(uint32_t v, int n) {
    n &= 31;
    return (v >> n) | (v << ((32 - n) & 31));
}

inline float u32_to_f(uint32_t t) { float f; std::memcpy(&f, &t, sizeof(f)); return f; }
inline uint32_t f_to_u32(float v) { uint32_t t; std::memcpy(&t, &v, sizeof(t)); return t; }

namespace dec {

inline float at_0x70(uptr addr)  { uint32_t t = memory::read<uint32_t>(addr); t -= 0x2E871D6F; t = ror32(t, 24); t ^= 0x43C64634; return u32_to_f(t); }
inline float at_0x78(uptr addr)  { uint32_t t = memory::read<uint32_t>(addr); t = ror32(t, 20); t -= 0xBE0E6854; t ^= 0x00E16150; return u32_to_f(t); }
inline float at_0x80(uptr addr)  { uint32_t t = memory::read<uint32_t>(addr); t += 0x66A31567; t = ror32(t, 30); t -= 0x94289830; return u32_to_f(t); }
inline float at_0x88(uptr addr)  { uint32_t t = memory::read<uint32_t>(addr); t -= 0x3E69D0FE; t = ror32(t, 13); t -= 0x69B9F5FB; return u32_to_f(t); }
inline float at_0x90(uptr addr)  { uint32_t t = memory::read<uint32_t>(addr); t ^= 0x253A43B9; t = ror32(t, 18); t -= 0x917046AE; t ^= 0xE34C27FF; return u32_to_f(t); }
inline float at_0x98(uptr addr)  { uint32_t t = memory::read<uint32_t>(addr); t -= 0x263D1C7C; t = ror32(t, 25); t -= 0x6667B67E; t = ror32(t, 4);  return u32_to_f(t); }
inline float at_0xA0(uptr addr)  { uint32_t t = memory::read<uint32_t>(addr); t ^= 0xCCDCB78F; t -= 0x0F3D66AB; t = ror32(t, 10); return u32_to_f(t); }
inline float at_0xA8(uptr addr)  { uint32_t t = memory::read<uint32_t>(addr); t -= 0x51E982EF; t ^= 0xAAC96903; t += 0x6F68090D; t = ror32(t, 30); return u32_to_f(t); }
inline float at_0xB0(uptr addr)  { uint32_t t = memory::read<uint32_t>(addr); t ^= 0x15F9C0DC; t -= 0x74C8017B; t ^= 0x33C17945; return u32_to_f(t); }
inline float at_0xC0(uptr addr)  { uint32_t t = memory::read<uint32_t>(addr); t = ror32(t, 24); t -= 0xEB27A6BA; t = ror32(t, 9);  return u32_to_f(t); }
inline float at_0xC8(uptr addr)  { uint32_t t = memory::read<uint32_t>(addr); t = ror32(t, 14); t -= 0xB7D9251D; t ^= 0xC528D434; return u32_to_f(t); }
inline float at_0xD0(uptr addr)  { uint32_t t = memory::read<uint32_t>(addr); t -= 0x0A821062; t = ror32(t, 11); t ^= 0xAACA1CBF; t = ror32(t, 29); return u32_to_f(t); }
inline float at_0xF8(uptr addr)  { uint32_t t = memory::read<uint32_t>(addr); t -= 0x6722D502; t = ror32(t, 6);  t ^= 0x49F14F0F; t += 0x7F1473D1; return u32_to_f(t); }
inline float at_0x100(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t -= 0x28EFB1ED; t = ror32(t, 9);  t -= 0xFCB4C442; t = ror32(t, 27); return u32_to_f(t); }
inline float at_0x108(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t -= 0x632450D6; t = ror32(t, 23); t -= 0xACD124E8; return u32_to_f(t); }
inline float at_0x110(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t -= 0x7D560551; t ^= 0x1EBE64DA; t -= 0x7DB4D0C1; return u32_to_f(t); }
inline float at_0x118(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t += 0x7261233C; t = ror32(t, 15); t -= 0x661436F0; t = ror32(t, 23); return u32_to_f(t); }
inline float at_0x120(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t += 0x50073C5C; t ^= 0x6BBD28DD; t += 0x7FD457FB; t = ror32(t, 13); return u32_to_f(t); }
inline float at_0x198(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t = ror32(t, 31); t -= 0x18ED23D5; t = ror32(t, 19); t -= 0xCFD40643; return u32_to_f(t); }
inline float at_0x1A0(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t -= 0x29606CD6; t = ror32(t, 11); t -= 0x7118C5A4; return u32_to_f(t); }
inline float at_0x1A8(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t = ror32(t, 21); t -= 0xD61639A0; t ^= 0xFBDCA376; t = ror32(t, 22); return u32_to_f(t); }
inline float at_0x1BC(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t -= 0x1DB9F2DC; t = ror32(t, 15); t -= 0x8295ABFC; return u32_to_f(t); }
inline float at_0x1C4(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t -= 0x50645BBE; t ^= 0x66CE9706; t = ror32(t, 15); return u32_to_f(t); }
inline float at_0x1CC(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t = ror32(t, 30); t ^= 0x882E8968; t += 0x5D856C0E; t = ror32(t, 17); return u32_to_f(t); }
inline float at_0x1D4(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t += 0x0DCA1054; t ^= 0x9D65E2F0; t -= 0x51CD7C9C; t = ror32(t, 23); return u32_to_f(t); }
inline float at_0x1DC(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t -= 0x3FED98A4; t = ror32(t, 25); t ^= 0xBB75BABE; return u32_to_f(t); }
inline float at_0x1E4(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t = ror32(t, 15); t -= 0xB84E68F4; return u32_to_f(t); }
inline float at_0x1EC(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t ^= 0x64A5DF7C; t -= 0x1D7E13AC; t = ror32(t, 10); return u32_to_f(t); }
inline float at_0x1F4(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t -= 0x776FC9E4; t ^= 0x4EE7CDAE; t = ror32(t, 11); t -= 0x456EA19C; return u32_to_f(t); }
inline float at_0x1FC(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t -= 0x48CBA4C4; t = ror32(t, 22); t ^= 0xAE876A56; return u32_to_f(t); }
inline float at_0x204(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t = ror32(t, 10); t -= 0xA0B40F1C; t ^= 0xEA5E4234; t = ror32(t, 12); return u32_to_f(t); }
inline float at_0x20C(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t -= 0x2CB61A2A; t = ror32(t, 13); t ^= 0x5878AA54; return u32_to_f(t); }
inline float at_0x214(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t += 0x00000001; t -= 0x0DE2A22F; t ^= 0x783131F8; t = ror32(t, 14); t -= 0x0C4776B8; return u32_to_f(t); }

inline float gravityMultiplier(uptr bm) { return at_0xA8(bm + 0xA8); }
inline float grounded         (uptr bm) { return at_0x1BC(bm + 0x1BC); }
inline float wasGrounded      (uptr bm) { return at_0x1C4(bm + 0x1C4); }
inline float climbing         (uptr bm) { return at_0x1CC(bm + 0x1CC); }
inline float wasClimbing      (uptr bm) { return at_0x1D4(bm + 0x1D4); }
inline float swimming         (uptr bm) { return at_0x1EC(bm + 0x1EC); }
inline float wasSwimming      (uptr bm) { return at_0x1F4(bm + 0x1F4); }
inline float jumping          (uptr bm) { return at_0x1FC(bm + 0x1FC); }
inline float flying           (uptr bm) { return at_0x204(bm + 0x204); }
inline float falling          (uptr bm) { return at_0x20C(bm + 0x20C); }

inline float wasFalling       (uptr bm) { return wasSwimming(bm); }
inline float wasFlying        (uptr bm) { return jumping(bm); }

}

namespace enc {

inline uint32_t at_0x70 (float v) { uint32_t t = f_to_u32(v); t ^= 0x43C64634; t = rol32(t, 24); t += 0x2E871D6F; return t; }
inline uint32_t at_0x78 (float v) { uint32_t t = f_to_u32(v); t ^= 0x00E16150; t += 0xBE0E6854; t = rol32(t, 20); return t; }
inline uint32_t at_0x80 (float v) { uint32_t t = f_to_u32(v); t += 0x94289830; t = rol32(t, 30); t -= 0x66A31567; return t; }
inline uint32_t at_0x88 (float v) { uint32_t t = f_to_u32(v); t += 0x69B9F5FB; t = rol32(t, 13); t += 0x3E69D0FE; return t; }
inline uint32_t at_0x90 (float v) { uint32_t t = f_to_u32(v); t ^= 0xE34C27FF; t += 0x917046AE; t = rol32(t, 18); t ^= 0x253A43B9; return t; }
inline uint32_t at_0x98 (float v) { uint32_t t = f_to_u32(v); t = rol32(t, 4);  t += 0x6667B67E; t = rol32(t, 25); t += 0x263D1C7C; return t; }
inline uint32_t at_0xA0 (float v) { uint32_t t = f_to_u32(v); t = rol32(t, 10); t += 0x0F3D66AB; t ^= 0xCCDCB78F; return t; }
inline uint32_t at_0xA8 (float v) { uint32_t t = f_to_u32(v); t = rol32(t, 30); t -= 0x6F68090D; t ^= 0xAAC96903; t += 0x51E982EF; return t; }
inline uint32_t at_0xB0 (float v) { uint32_t t = f_to_u32(v); t ^= 0x33C17945; t += 0x74C8017B; t ^= 0x15F9C0DC; return t; }
inline uint32_t at_0xC0 (float v) { uint32_t t = f_to_u32(v); t = rol32(t, 9);  t += 0xEB27A6BA; t = rol32(t, 24); return t; }
inline uint32_t at_0xC8 (float v) { uint32_t t = f_to_u32(v); t ^= 0xC528D434; t += 0xB7D9251D; t = rol32(t, 14); return t; }
inline uint32_t at_0xD0 (float v) { uint32_t t = f_to_u32(v); t = rol32(t, 29); t ^= 0xAACA1CBF; t = rol32(t, 11); t += 0x0A821062; return t; }
inline uint32_t at_0xF8 (float v) { uint32_t t = f_to_u32(v); t -= 0x7F1473D1; t ^= 0x49F14F0F; t = rol32(t, 6);  t += 0x6722D502; return t; }
inline uint32_t at_0x100(float v) { uint32_t t = f_to_u32(v); t = rol32(t, 27); t += 0xFCB4C442; t = rol32(t, 9);  t += 0x28EFB1ED; return t; }
inline uint32_t at_0x108(float v) { uint32_t t = f_to_u32(v); t += 0xACD124E8; t = rol32(t, 23); t += 0x632450D6; return t; }
inline uint32_t at_0x110(float v) { uint32_t t = f_to_u32(v); t += 0x7DB4D0C1; t ^= 0x1EBE64DA; t += 0x7D560551; return t; }
inline uint32_t at_0x118(float v) { uint32_t t = f_to_u32(v); t = rol32(t, 23); t += 0x661436F0; t = rol32(t, 15); t -= 0x7261233C; return t; }
inline uint32_t at_0x120(float v) { uint32_t t = f_to_u32(v); t = rol32(t, 13); t -= 0x7FD457FB; t ^= 0x6BBD28DD; t -= 0x50073C5C; return t; }
inline uint32_t at_0x198(float v) { uint32_t t = f_to_u32(v); t += 0xCFD40643; t = rol32(t, 19); t += 0x18ED23D5; t = rol32(t, 31); return t; }
inline uint32_t at_0x1A0(float v) { uint32_t t = f_to_u32(v); t += 0x7118C5A4; t = rol32(t, 11); t += 0x29606CD6; return t; }
inline uint32_t at_0x1A8(float v) { uint32_t t = f_to_u32(v); t = rol32(t, 22); t ^= 0xFBDCA376; t += 0xD61639A0; t = rol32(t, 21); return t; }
inline uint32_t at_0x1BC(float v) { uint32_t t = f_to_u32(v); t += 0x8295ABFC; t = rol32(t, 15); t += 0x1DB9F2DC; return t; }
inline uint32_t at_0x1C4(float v) { uint32_t t = f_to_u32(v); t = rol32(t, 15); t ^= 0x66CE9706; t += 0x50645BBE; return t; }
inline uint32_t at_0x1CC(float v) { uint32_t t = f_to_u32(v); t = rol32(t, 17); t -= 0x5D856C0E; t ^= 0x882E8968; t = rol32(t, 30); return t; }
inline uint32_t at_0x1D4(float v) { uint32_t t = f_to_u32(v); t = rol32(t, 23); t += 0x51CD7C9C; t ^= 0x9D65E2F0; t -= 0x0DCA1054; return t; }
inline uint32_t at_0x1DC(float v) { uint32_t t = f_to_u32(v); t ^= 0xBB75BABE; t = rol32(t, 25); t += 0x3FED98A4; return t; }
inline uint32_t at_0x1E4(float v) { uint32_t t = f_to_u32(v); t += 0xB84E68F4; t = rol32(t, 15); return t; }
inline uint32_t at_0x1EC(float v) { uint32_t t = f_to_u32(v); t = rol32(t, 10); t += 0x1D7E13AC; t ^= 0x64A5DF7C; return t; }
inline uint32_t at_0x1F4(float v) { uint32_t t = f_to_u32(v); t += 0x456EA19C; t = rol32(t, 11); t ^= 0x4EE7CDAE; t += 0x776FC9E4; return t; }
inline uint32_t at_0x1FC(float v) { uint32_t t = f_to_u32(v); t ^= 0xAE876A56; t = rol32(t, 22); t += 0x48CBA4C4; return t; }
inline uint32_t at_0x204(float v) { uint32_t t = f_to_u32(v); t = rol32(t, 12); t ^= 0xEA5E4234; t += 0xA0B40F1C; t = rol32(t, 10); return t; }
inline uint32_t at_0x20C(float v) { uint32_t t = f_to_u32(v); t ^= 0x5878AA54; t = rol32(t, 13); t += 0x2CB61A2A; return t; }
inline uint32_t at_0x214(float v) { uint32_t t = f_to_u32(v); t += 0x0C4776B8; t = rol32(t, 14); t ^= 0x783131F8; t += 0x0DE2A22F; t -= 0x00000001; return t; }

}

namespace dec_vec3 {
inline float at_0x128(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t ^= 0xE1889714; t += 0x37C1D91A; t ^= 0x4B15FF64; return u32_to_f(t); }
inline float at_0x138(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t -= 0x2C5924FD; t = ror32(t, 21); t ^= 0x192F16EA; return u32_to_f(t); }
inline float at_0x148(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t -= 0x61451CE9; t = ror32(t, 15); t ^= 0x7E7BC22C; return u32_to_f(t); }
inline float at_0x158(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t -= 0x37E52B2C; t ^= 0x0B253E71; t = ror32(t, 27); return u32_to_f(t); }
inline float at_0x168(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t -= 0x3738907B; t ^= 0xE3684109; t = ror32(t, 14); t -= 0x285F4AE2; return u32_to_f(t); }
inline float at_0x178(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t += 0x521477D3; t = ror32(t, 21); t -= 0xB424575A; t = ror32(t, 4); return u32_to_f(t); }
inline float at_0x188(uptr addr) { uint32_t t = memory::read<uint32_t>(addr); t ^= 0x63404926; t = ror32(t, 8);  t -= 0x44694313; return u32_to_f(t); }
}

namespace enc_vec3 {
inline uint32_t at_0x128(float v) { uint32_t t = f_to_u32(v); t ^= 0x4B15FF64; t -= 0x37C1D91A; t ^= 0xE1889714; return t; }
inline uint32_t at_0x138(float v) { uint32_t t = f_to_u32(v); t ^= 0x192F16EA; t = rol32(t, 21); t += 0x2C5924FD; return t; }
inline uint32_t at_0x148(float v) { uint32_t t = f_to_u32(v); t ^= 0x7E7BC22C; t = rol32(t, 15); t += 0x61451CE9; return t; }
inline uint32_t at_0x158(float v) { uint32_t t = f_to_u32(v); t = rol32(t, 27); t ^= 0x0B253E71; t += 0x37E52B2C; return t; }
inline uint32_t at_0x168(float v) { uint32_t t = f_to_u32(v); t += 0x285F4AE2; t = rol32(t, 14); t ^= 0xE3684109; t += 0x3738907B; return t; }
inline uint32_t at_0x178(float v) { uint32_t t = f_to_u32(v); t = rol32(t, 4);  t += 0xB424575A; t = rol32(t, 21); t -= 0x521477D3; return t; }
inline uint32_t at_0x188(float v) { uint32_t t = f_to_u32(v); t += 0x44694313; t = rol32(t, 8);  t ^= 0x63404926; return t; }
}

template<uint32_t (*Enc)(float)>
inline void _write_vec3(uptr bm, uint32_t off, float x, float y, float z) {
    memory::write<uint32_t>(bm + off + 0, Enc(x));
    memory::write<uint32_t>(bm + off + 4, Enc(y));
    memory::write<uint32_t>(bm + off + 8, Enc(z));
}
template<float (*Dec)(uptr)>
inline void _read_vec3(uptr bm, uint32_t off, float& x, float& y, float& z) {
    x = Dec(bm + off + 0);
    y = Dec(bm + off + 4);
    z = Dec(bm + off + 8);
}

inline void write_vec3_0x128(uptr bm, float x, float y, float z) { _write_vec3<enc_vec3::at_0x128>(bm, 0x128, x, y, z); }
inline void write_vec3_0x138(uptr bm, float x, float y, float z) { _write_vec3<enc_vec3::at_0x138>(bm, 0x138, x, y, z); }
inline void write_vec3_0x148(uptr bm, float x, float y, float z) { _write_vec3<enc_vec3::at_0x148>(bm, 0x148, x, y, z); }
inline void write_vec3_0x158(uptr bm, float x, float y, float z) { _write_vec3<enc_vec3::at_0x158>(bm, 0x158, x, y, z); }
inline void write_vec3_0x168(uptr bm, float x, float y, float z) { _write_vec3<enc_vec3::at_0x168>(bm, 0x168, x, y, z); }
inline void write_vec3_0x178(uptr bm, float x, float y, float z) { _write_vec3<enc_vec3::at_0x178>(bm, 0x178, x, y, z); }
inline void write_vec3_0x188(uptr bm, float x, float y, float z) { _write_vec3<enc_vec3::at_0x188>(bm, 0x188, x, y, z); }

inline void read_vec3_0x128(uptr bm, float& x, float& y, float& z) { _read_vec3<dec_vec3::at_0x128>(bm, 0x128, x, y, z); }
inline void read_vec3_0x138(uptr bm, float& x, float& y, float& z) { _read_vec3<dec_vec3::at_0x138>(bm, 0x138, x, y, z); }
inline void read_vec3_0x148(uptr bm, float& x, float& y, float& z) { _read_vec3<dec_vec3::at_0x148>(bm, 0x148, x, y, z); }
inline void read_vec3_0x158(uptr bm, float& x, float& y, float& z) { _read_vec3<dec_vec3::at_0x158>(bm, 0x158, x, y, z); }
inline void read_vec3_0x168(uptr bm, float& x, float& y, float& z) { _read_vec3<dec_vec3::at_0x168>(bm, 0x168, x, y, z); }
inline void read_vec3_0x178(uptr bm, float& x, float& y, float& z) { _read_vec3<dec_vec3::at_0x178>(bm, 0x178, x, y, z); }
inline void read_vec3_0x188(uptr bm, float& x, float& y, float& z) { _read_vec3<dec_vec3::at_0x188>(bm, 0x188, x, y, z); }

inline void write_gravityMultiplier(uptr bm, float v) { memory::write<uint32_t>(bm + 0xA8,  enc::at_0xA8 (v)); }
inline void write_grounded         (uptr bm, bool b)  { memory::write<uint32_t>(bm + 0x1BC, enc::at_0x1BC(b ? 1.0f : 0.0f)); }
inline void write_wasGrounded      (uptr bm, bool b)  { memory::write<uint32_t>(bm + 0x1C4, enc::at_0x1C4(b ? 1.0f : 0.0f)); }
inline void write_climbing         (uptr bm, bool b)  { memory::write<uint32_t>(bm + 0x1CC, enc::at_0x1CC(b ? 1.0f : 0.0f)); }
inline void write_wasClimbing      (uptr bm, bool b)  { memory::write<uint32_t>(bm + 0x1D4, enc::at_0x1D4(b ? 1.0f : 0.0f)); }
inline void write_swimming         (uptr bm, bool b)  { memory::write<uint32_t>(bm + 0x1EC, enc::at_0x1EC(b ? 1.0f : 0.0f)); }
inline void write_wasSwimming      (uptr bm, bool b)  { memory::write<uint32_t>(bm + 0x1F4, enc::at_0x1F4(b ? 1.0f : 0.0f)); }
inline void write_jumping          (uptr bm, bool b)  { memory::write<uint32_t>(bm + 0x1FC, enc::at_0x1FC(b ? 1.0f : 0.0f)); }
inline void write_flying           (uptr bm, bool b)  { memory::write<uint32_t>(bm + 0x204, enc::at_0x204(b ? 1.0f : 0.0f)); }
inline void write_falling          (uptr bm, bool b)  { memory::write<uint32_t>(bm + 0x20C, enc::at_0x20C(b ? 1.0f : 0.0f)); }

inline void write_wasFalling       (uptr bm, bool b)  { write_wasSwimming(bm, b); }
inline void write_wasFlying        (uptr bm, bool b)  { write_jumping(bm, b); }

inline void write_groundAngle      (uptr bm, float v) { memory::write<uint32_t>(bm + 0x70,  enc::at_0x70(v)); }
inline void write_groundAngleNew78 (uptr bm, float v) { memory::write<uint32_t>(bm + 0x78,  enc::at_0x78(v)); }

inline void write_groundAngleNew   (uptr bm, float v) { memory::write<uint32_t>(bm + 0x108, enc::at_0x108(v)); }
inline void write_sprintForced     (uptr bm, float v) { memory::write<uint32_t>(bm + 0x1A8, enc::at_0x1A8(v)); }
inline void write_maxVelocity      (uptr bm, float v) { memory::write<uint32_t>(bm + 0x214, enc::at_0x214(v)); }

inline void write_target_movement(uptr bm, float x, float y, float z) {
    write_vec3_0x128(bm, x, y, z);
}
inline void read_target_movement(uptr bm, float& x, float& y, float& z) {
    read_vec3_0x128(bm, x, y, z);
}

}
