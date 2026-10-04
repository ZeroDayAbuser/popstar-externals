#pragma once
#include <cstdint>
#include <string>
#include <vector>

using u8 = unsigned __int8;
using u16 = unsigned __int16;
using u32 = unsigned __int32;
using u64 = unsigned __int64;
using uptr = uintptr_t;
using i8 = __int8;
using i16 = __int16;
using i32 = __int32;
using i64 = __int64;
using f32 = float;
using f64 = double;

static std::vector<u32> g_materials = {
	0,       // None
	488366,  // CRT HDR (manpad_crt_hdr)
	202236,  // Red glow (ship_red_glow)
	116328,  // Green glow (GreenGlow)
	698670,  // Pink (WireMaterial_Pink)
	121186,  // Neon (NeonTR_On)
	632820,  // Blue holo (holosight_georeticle_blue)
	632824,  // Green holo (holosight_georeticle_green)
	483552,  // Fireworks glow (Fireworks_Glow)
	719202,  // Purple zone (BRZone_purple)
	949926,  // Lightning bolt (LightningBolt)
	504830,  // Lightning spark (fx_lightning3)
	542536,  // Star trail (StarTrail)
	143648,  // Glass (glass)
	625516,  // Animated blue wire (WireMaterial_Directional_Blue)
	1032242, // Spark (Spark)
	885588,  // Water drops (vfx_water_drops)
	155304,  // Gold (party_hat_7thbirthday_gold)
	525836,  // Roboto mono (RobotoMono_Regular_SDF_NoSoftness)
	394708,  // Shockwave (vfx_shockwave)
	167950,  // Shimmer (vfx_scarecrow_shimmer)
	496238,  // Rainbow (vfx_rainbow)
};

static std::vector<std::string> g_material_names = {
	"None",
	"CRT HDR",
	"Red glow",
	"Green glow",
	"Pink",
	"Neon",
	"Blue holo",
	"Green holo",
	"Fireworks glow",
	"Purple zone",
	"Lightning bolt",
	"Lightning spark",
	"Star trail",
	"Glass",
	"Animated blue wire",
	"Spark",
	"Water drops",
	"Gold",
	"The fuckin roboto font LMFAO",
	"Shockwave",
	"Shimmer",
	"Rainbow"
};