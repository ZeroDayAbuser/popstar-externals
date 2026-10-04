#include "aimbot.hpp"
#include <app/winapp.hpp>
#include <cmath>
#include <game/cache/cache.hpp>
#include <game/features/feature_util.hpp>
#include <game/features/physx/physx.hpp>
#include <game/game.hpp>
#include <memory/memory.hpp>
#include <render/render.hpp>
#include <sdk/math/math.hpp>
#include <sdk/offsets.hpp>
#include <sdk/rust/entity/entity.hpp>
#include <settings/settings.hpp>

#include <algorithm>
#include <limits>
#include <random>
#include <sdk/decryptions.hpp>
#include <sdk/rust/entity/weapon.hpp>
#include <sdk/unity/unity.hpp>
#include <string_encryption.hpp>
#include <glm/gtc/quaternion.hpp>
#include <Windows.h>

namespace features::aimbot
{
	namespace {
		static bool try_read_transform_world_pos(uptr access, glm::vec3& out)
		{
			auto finite3 = [](const glm::vec3& v) {
				return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
			};
			auto finite4 = [](const glm::quat& q) {
				return std::isfinite(q.x) && std::isfinite(q.y) && std::isfinite(q.z) && std::isfinite(q.w);
			};

			if (offsets::UnityTransform::parent_off &&
			    offsets::UnityTransform::local_pos_off &&
			    offsets::UnityTransform::world_rot_off)
			{
			glm::vec3 accum{ 0.0f };
			glm::quat accum_rot{ 1.0f, 0.0f, 0.0f, 0.0f };
			constexpr int kMaxDepth = 8;
			uptr node = access;
				for (int depth = 0; depth < kMaxDepth && node; ++depth) {
					const glm::vec3 lp = memory::read<glm::vec3>(node + offsets::UnityTransform::local_pos_off);
					if (!finite3(lp)) return false;
					accum = accum_rot * lp + accum;

					const uptr parent = memory::read<uptr>(node + offsets::UnityTransform::parent_off);
					if (!parent) { out = accum; return true; }

					const glm::quat pr = memory::read<glm::quat>(parent + offsets::UnityTransform::world_rot_off);
					if (!finite4(pr)) return false;
				accum_rot = pr;
				node = parent;
			}
			return false;
		}

			const glm::vec3 fallback = memory::read<glm::vec3>(access + offsets::UnityTransform::world_pos_off);
			if (!finite3(fallback)) return false;
			out = fallback;
			return true;
		}
	} // namespace

#define ADD_LOG(strfmt) do { \
	auto _m = std::format("aimbot: L{} - {}", __LINE__, strfmt); \
	if (debug_info.empty() || debug_info.back() != _m) debug_info.push_back(std::move(_m)); \
} while (0)
#define ADD_LOG_RET(strfmt) do { \
	auto _m = std::format("aimbot: L{} - {}", __LINE__, strfmt); \
	if (debug_info.empty() || debug_info.back() != _m) debug_info.push_back(std::move(_m)); \
	return; \
} while (0)

	static f32 visualized_fov = 100.0f;
	static glm::vec2 target_position_2d = {};

	static bool s_autoshoot_lmb = false;
	static void set_autoshoot_lmb(bool down)
	{
		if (down == s_autoshoot_lmb) return;
		INPUT inp{};
		inp.type = INPUT_MOUSE;
		inp.mi.dwFlags = down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
		::SendInput(1, &inp, sizeof(INPUT));
		s_autoshoot_lmb = down;
	}

	struct AutoShootGuard {
		bool fire{ false };
		~AutoShootGuard() { set_autoshoot_lmb(fire); }
	};

	struct WeaponBallistics {
		float muzzle_velocity;
		float gravity_modifier;
		float drag;
	};

	static float read_active_muzzle_velocity()
	{
		auto* local = game::impl::local_player;
		if (!local) return 0.0f;

		const uintptr_t base_projectile = features::held_weapon_or_zero();
		if (!base_projectile) return 0.0f;

		const uintptr_t magazine = memory::read<uintptr_t>(base_projectile + offsets::BaseProjectile::primaryMagazine);
		if (!magazine) return 0.0f;

		const uintptr_t ammo_def = memory::read<uintptr_t>(magazine + offsets::Magazine::ammoType);
		if (!ammo_def) return 0.0f;

		static std::unordered_map<uintptr_t, uintptr_t> s_mod_for_ammo;
		uintptr_t item_mod = 0;
		auto it = s_mod_for_ammo.find(ammo_def);

		auto test = unity::get_component_by_name(ammo_def, "ItemModProjectile");
		
		if (test) {
			item_mod = test;
		}
		else return 0.0f;

		if (!item_mod) return 0.0f;
		float v = memory::read<f32>(item_mod + offsets::ItemModProjectile::projectileVelocity);
		if (!std::isfinite(v) || v < 1.0f || v > 2000.0f) return 0.0f;

		if (offsets::BaseProjectile::projectileVelScale) {
			const float scale = memory::read<f32>(base_projectile + offsets::BaseProjectile::projectileVelScale);
			if (std::isfinite(scale) && scale > 0.05f && scale < 5.0f)
				v *= scale;
		}

		if (local->item_shortname == xs("bow.compound")) {
			const float bonus = memory::read<f32>(
				base_projectile + offsets::CompoundBowWeapon::stringBonusVelocity);
			if (bonus > 0.0f && bonus < 200.0f) v += bonus;
		}

		return v;
	}

	static std::string read_def_short(uptr def)
	{
		if (!def) return {};
		uptr str_obj = memory::read<uptr>(def + offsets::ItemDefinition::shortName);
		if (!str_obj) return {};
		i32 len = memory::read<i32>(str_obj + 0x10);
		if (len <= 0 || len > 256) return {};
		std::wstring wide(static_cast<std::size_t>(len), L'\0');
		if (!memory::read_memory_raw(str_obj + 0x14, wide.data(),
		                             static_cast<std::size_t>(len) * sizeof(wchar_t)))
			return {};
		return std::string(wide.begin(), wide.end());
	}

	static std::string read_active_ammo_shortname()
	{
		const uptr held = features::held_weapon_or_zero();
		
		if (!held) return {};
		uptr mag = memory::read<uptr>(held + offsets::BaseProjectile::primaryMagazine);
		
		if (!mag) return {};
		uptr ammo_def = memory::read<uptr>(mag + offsets::Magazine::ammoType);
		
		return read_def_short(ammo_def);
	}

	static WeaponBallistics get_ballistics(const std::string& weapon,
	                                       const std::string& ammo)
	{
		static const std::unordered_map<std::string, WeaponBallistics> kProfiles = {
			{ xs("rifle.bolt"),               { 656.0f,  1.00f,  0.60f } },
			{ xs("rifle.l96"),                { 1125.0f, 1.00f,  0.60f } },
			{ xs("rifle.ak"),                 { 375.0f,  1.00f,  0.60f } },
			{ xs("rifle.ak.ice"),             { 375.0f,  1.00f,  0.60f } },
			{ xs("rifle.ak.diver"),           { 375.0f,  1.00f,  0.60f } },
			{ xs("rifle.lr300"),              { 562.0f,  1.00f,  0.60f } },
			{ xs("rifle.m39"),                { 469.0f,  1.00f,  0.60f } },
			{ xs("rifle.semiauto"),           { 375.0f,  1.00f,  0.60f } },
			{ xs("rifle.sks"),                { 375.0f,  1.00f,  0.60f } },
			{ xs("lmg.m249"),                 { 487.0f,  1.00f,  0.60f } },
			{ xs("hmlmg"),                    { 525.0f,  1.00f,  0.60f } },
			{ xs("minigun"),                  { 525.0f,  1.00f,  0.60f } },
			{ xs("smg.thompson"),             { 300.0f,  1.00f,  0.70f } },
			{ xs("smg.mp5"),                  { 270.0f,  1.00f,  0.70f } },
			{ xs("smg.2"),                    { 240.0f,  1.00f,  0.70f } },
			{ xs("pistol.semiauto"),          { 300.0f,  1.00f,  0.70f } },
			{ xs("pistol.m92"),               { 300.0f,  1.00f,  0.70f } },
			{ xs("pistol.python"),            { 300.0f,  1.00f,  0.70f } },
			{ xs("pistol.revolver"),          { 300.0f,  1.00f,  0.70f } },
			{ xs("pistol.nailgun"),           { 100.0f,  1.00f,  0.005f} },
			{ xs("pistol.eoka"),              { 227.0f,  1.00f,  1.00f } },
			{ xs("shotgun.double"),           { 225.0f,  1.00f,  1.00f } },
			{ xs("shotgun.pump"),             { 225.0f,  1.00f,  1.00f } },
			{ xs("shotgun.spas12"),           { 225.0f,  1.00f,  1.00f } },
			{ xs("shotgun.m4"),               { 225.0f,  1.00f,  1.00f } },
			{ xs("shotgun.waterpipe"),        { 225.0f,  1.00f,  1.00f } },
			{ xs("bow.hunting"),              {  50.0f,  0.75f,  0.005f} },
			{ xs("bow.compound"),             {  61.5f,  0.75f,  0.005f} },
			{ xs("crossbow"),                 {  75.0f,  0.75f,  0.005f} },
			{ xs("multiplegrenadelauncher"),  {  60.0f,  1.50f,  0.30f } },
			{ xs("rocket.launcher"),          {  70.0f,  0.50f,  0.10f } },

			{ xs("rifle.ak|ammo.rifle"),                  { 375.0f, 1.00f,  0.60f } },
			{ xs("rifle.ak|ammo.rifle.hv"),               { 450.0f, 1.00f,  1.00f } },
			{ xs("rifle.ak|ammo.rifle.explosive"),        { 225.0f, 1.25f,  0.60f } },
			{ xs("rifle.ak|ammo.rifle.incendiary"),       { 225.0f, 1.00f,  0.60f } },
			{ xs("rifle.ak.diver|ammo.rifle"),            { 375.0f, 1.00f,  0.60f } },
			{ xs("rifle.ak.diver|ammo.rifle.hv"),         { 450.0f, 1.00f,  1.00f } },
			{ xs("rifle.ak.diver|ammo.rifle.explosive"),  { 225.0f, 1.25f,  0.60f } },
			{ xs("rifle.ak.diver|ammo.rifle.incendiary"), { 225.0f, 1.00f,  0.60f } },
			{ xs("rifle.lr300|ammo.rifle"),               { 375.0f, 1.00f,  0.60f } },
			{ xs("rifle.lr300|ammo.rifle.hv"),            { 450.0f, 1.00f,  1.00f } },
			{ xs("rifle.lr300|ammo.rifle.explosive"),     { 225.0f, 1.25f,  0.60f } },
			{ xs("rifle.lr300|ammo.rifle.incendiary"),    { 225.0f, 1.00f,  0.60f } },
			{ xs("rifle.bolt|ammo.rifle"),                { 656.0f, 1.00f,  0.60f } },
			{ xs("rifle.bolt|ammo.rifle.hv"),             { 788.0f, 1.00f,  1.00f } },
			{ xs("rifle.bolt|ammo.rifle.explosive"),      { 394.0f, 1.25f,  0.60f } },
			{ xs("rifle.bolt|ammo.rifle.incendiary"),     { 394.0f, 1.00f,  0.60f } },
			{ xs("rifle.l96|ammo.rifle"),                 { 1250.0f, 1.00f, 0.60f } },
			{ xs("rifle.l96|ammo.rifle.hv"),              { 1150.0f, 1.00f, 1.00f } },
			{ xs("rifle.l96|ammo.rifle.explosive"),       { 2025.0f, 1.25f, 0.60f } },
			{ xs("rifle.l96|ammo.rifle.incendiary"),      { 2025.0f, 1.00f, 0.60f } },
			{ xs("rifle.m39|ammo.rifle"),                 { 469.0f, 1.00f,  0.60f } },
			{ xs("rifle.m39|ammo.rifle.hv"),              { 563.0f, 1.00f,  1.00f } },
			{ xs("rifle.m39|ammo.rifle.explosive"),       { 281.0f, 1.25f,  0.60f } },
			{ xs("rifle.m39|ammo.rifle.incendiary"),      { 281.0f, 1.00f,  0.60f } },
			{ xs("rifle.semiauto|ammo.rifle"),            { 375.0f, 1.00f,  0.60f } },
			{ xs("rifle.semiauto|ammo.rifle.hv"),         { 450.0f, 1.00f,  1.00f } },
			{ xs("rifle.semiauto|ammo.rifle.explosive"),  { 225.0f, 1.25f,  0.60f } },
			{ xs("rifle.semiauto|ammo.rifle.incendiary"), { 225.0f, 1.00f,  0.60f } },
			{ xs("lmg.m249|ammo.rifle"),                  { 487.5f, 1.00f,  0.60f } },
			{ xs("lmg.m249|ammo.rifle.hv"),               { 585.0f, 1.00f,  1.00f } },
			{ xs("lmg.m249|ammo.rifle.explosive"),        { 293.0f, 1.25f,  0.60f } },
			{ xs("lmg.m249|ammo.rifle.incendiary"),       { 293.0f, 1.00f,  0.60f } },
			{ xs("smg.mp5|ammo.pistol"),                  { 300.0f, 1.00f,  0.70f } },
			{ xs("smg.mp5|ammo.pistol.hv"),               { 400.0f, 1.00f,  1.00f } },
			{ xs("smg.mp5|ammo.pistol.fire"),             { 225.0f, 1.00f,  0.70f } },
			{ xs("smg.2|ammo.pistol"),                    { 300.0f, 1.00f,  0.70f } },
			{ xs("smg.2|ammo.pistol.hv"),                 { 400.0f, 1.00f,  1.00f } },
			{ xs("smg.2|ammo.pistol.fire"),               { 225.0f, 1.00f,  0.70f } },
			{ xs("smg.thompson|ammo.pistol"),             { 300.0f, 1.00f,  0.70f } },
			{ xs("smg.thompson|ammo.pistol.hv"),          { 400.0f, 1.00f,  1.00f } },
			{ xs("smg.thompson|ammo.pistol.fire"),        { 300.0f, 1.00f,  0.60f } },
			{ xs("pistol.m92|ammo.pistol"),               { 300.0f, 1.00f,  0.70f } },
			{ xs("pistol.m92|ammo.pistol.hv"),            { 400.0f, 1.00f,  1.00f } },
			{ xs("pistol.m92|ammo.pistol.fire"),          { 225.0f, 1.00f,  0.70f } },
			{ xs("pistol.python|ammo.pistol"),            { 300.0f, 1.00f,  0.70f } },
			{ xs("pistol.python|ammo.pistol.hv"),         { 400.0f, 1.00f,  1.00f } },
			{ xs("pistol.python|ammo.pistol.fire"),       { 300.0f, 1.00f,  0.60f } },
			{ xs("pistol.revolver|ammo.pistol"),          { 300.0f, 1.00f,  0.70f } },
			{ xs("pistol.revolver|ammo.pistol.hv"),       { 400.0f, 1.00f,  1.00f } },
			{ xs("pistol.revolver|ammo.pistol.fire"),     { 225.0f, 1.00f,  0.70f } },
			{ xs("pistol.semiauto|ammo.pistol"),          { 300.0f, 1.00f,  0.70f } },
			{ xs("pistol.semiauto|ammo.pistol.hv"),       { 400.0f, 1.00f,  1.00f } },
			{ xs("pistol.semiauto|ammo.pistol.fire"),     { 225.0f, 1.00f,  0.70f } },
			{ xs("shotgun.double|ammo.shotgun"),          { 225.0f, 1.00f,  1.00f } },
			{ xs("shotgun.pump|ammo.shotgun"),            { 225.0f, 1.00f,  1.00f } },
			{ xs("shotgun.spas12|ammo.shotgun"),          { 225.0f, 1.00f,  1.00f } },
			{ xs("shotgun.waterpipe|ammo.shotgun"),       { 225.0f, 1.00f,  1.00f } },
			{ xs("shotgun.double|ammo.shotgun.slug"),     { 225.0f, 1.00f,  0.60f } },
			{ xs("shotgun.double|ammo.shotgun.fire"),     { 100.0f, 1.00f,  1.00f } },
			{ xs("shotgun.double|ammo.handmade.shell"),   { 100.0f, 1.00f,  1.00f } },
			{ xs("shotgun.pump|ammo.shotgun.slug"),       { 225.0f, 1.00f,  0.60f } },
			{ xs("shotgun.pump|ammo.shotgun.fire"),       { 100.0f, 1.00f,  1.00f } },
			{ xs("shotgun.pump|ammo.handmade.shell"),     { 100.0f, 1.00f,  1.00f } },
			{ xs("shotgun.spas12|ammo.shotgun.slug"),     { 225.0f, 1.00f,  0.60f } },
			{ xs("shotgun.spas12|ammo.shotgun.fire"),     { 100.0f, 1.00f,  1.00f } },
			{ xs("shotgun.spas12|ammo.handmade.shell"),   { 100.0f, 1.00f,  1.00f } },
			{ xs("shotgun.waterpipe|ammo.shotgun.slug"),  { 225.0f, 1.00f,  0.60f } },
			{ xs("shotgun.waterpipe|ammo.handmade.shell"),{ 100.0f, 1.00f,  1.00f } },
			{ xs("pistol.eoka|ammo.shotgun"),             { 225.0f, 1.00f,  1.00f } },
			{ xs("pistol.eoka|ammo.shotgun.slug"),        { 225.0f, 1.00f,  0.60f } },
			{ xs("pistol.eoka|ammo.shotgun.fire"),        { 100.0f, 1.00f,  1.00f } },
			{ xs("pistol.eoka|ammo.handmade.shell"),      { 100.0f, 1.00f,  1.00f } },
			{ xs("legacy bow|arrow.wooden"),              {  50.0f, 0.75f,  0.005f} },
			{ xs("legacy bow|arrow.hv"),                  {  80.0f, 0.50f,  0.005f} },
			{ xs("legacy bow|arrow.bone"),                {  45.0f, 0.75f,  0.010f} },
			{ xs("legacy bow|arrow.fire"),                {  40.0f, 1.00f,  0.010f} },
			{ xs("bow.hunting|arrow.wooden"),             {  50.0f, 0.75f,  0.005f} },
			{ xs("bow.hunting|arrow.hv"),                 {  80.0f, 0.50f,  0.005f} },
			{ xs("bow.hunting|arrow.bone"),               {  45.0f, 0.75f,  0.010f} },
			{ xs("bow.hunting|arrow.fire"),               {  40.0f, 1.00f,  0.010f} },
			{ xs("bow.compound|arrow.wooden"),            { 100.0f, 0.75f,  0.005f} },
			{ xs("bow.compound|arrow.hv"),                { 160.0f, 0.50f,  0.005f} },
			{ xs("bow.compound|arrow.bone"),              {  90.0f, 0.75f,  0.010f} },
			{ xs("bow.compound|arrow.fire"),              {  80.0f, 1.00f,  0.010f} },
			{ xs("crossbow|arrow.wooden"),                {  75.0f, 0.75f,  0.005f} },
			{ xs("crossbow|arrow.hv"),                    { 120.0f, 0.50f,  0.005f} },
			{ xs("crossbow|arrow.bone"),                  {  68.0f, 0.75f,  0.010f} },
			{ xs("crossbow|arrow.fire"),                  {  60.0f, 1.00f,  0.010f} },
			{ xs("crossbowbowless|arrow.wooden"),         {  75.0f, 0.75f,  0.005f} },
			{ xs("crossbowbowless|arrow.hv"),             { 120.0f, 0.50f,  0.005f} },
			{ xs("crossbowbowless|arrow.bone"),           {  68.0f, 0.75f,  0.010f} },
			{ xs("crossbowbowless|arrow.fire"),           {  60.0f, 1.00f,  0.010f} },
			{ xs("pitchfork"),                            {  25.0f, 3.00f,  1.00f } },
			{ xs("spear.stone"),                          {  25.0f, 3.00f,  1.00f } },
			{ xs("spear.wooden"),                         {  25.0f, 3.00f,  1.00f } },
			{ xs("axe.salvaged"),                         {  25.0f, 3.00f,  1.00f } },
			{ xs("hatchet"),                              {  25.0f, 3.00f,  1.00f } },
			{ xs("pickaxe"),                              {  25.0f, 3.00f,  1.00f } },
			{ xs("stone.pickaxe"),                        {  22.0f, 3.00f,  1.00f } },
			{ xs("stonehatchet"),                         {  22.0f, 3.00f,  1.00f } },
			{ xs("hammer.salvaged"),                      {  22.0f, 3.00f,  1.00f } },
			{ xs("icepick.salvaged"),                     {  25.0f, 3.00f,  1.00f } },
			{ xs("machete"),                              {  28.0f, 3.00f,  1.00f } },
			{ xs("knife.bone"),                           {  22.0f, 3.00f,  1.00f } },
			{ xs("knife.combat"),                         {  25.0f, 3.00f,  1.00f } },
			{ xs("knife.butcher"),                        {  22.0f, 3.00f,  1.00f } },
			{ xs("candy_cane.club"),                      {  25.0f, 3.00f,  1.00f } },
			{ xs("mace"),                                 {  22.0f, 3.00f,  1.00f } },
			{ xs("mace.baseballbat"),                     {  22.0f, 3.00f,  1.00f } },
			{ xs("bone.club"),                            {  22.0f, 3.00f,  1.00f } },
			{ xs("skull.club"),                           {  25.0f, 3.00f,  1.00f } },
			{ xs("skullclub"),                            {  25.0f, 3.00f,  1.00f } },
			{ xs("salvaged.cleaver"),                     {  22.0f, 3.00f,  1.00f } },
			{ xs("salvaged.sword"),                       {  25.0f, 3.00f,  1.00f } },
			{ xs("longsword"),                            {  25.0f, 3.00f,  1.00f } },
			{ xs("paddle"),                               {  22.0f, 3.00f,  1.00f } },
			{ xs("rock"),                                 {  20.0f, 3.00f,  1.00f } },
			{ xs("torch"),                                {  20.0f, 3.00f,  1.00f } },
			{ xs("pistol.nailgun|ammo.nailgun.nails"),    {  50.0f, 0.75f,  0.005f} },
			{ xs("snowballgun|ammo.snowballgun"),         {  50.0f, 1.00f,  1.60f } },
		};
		static const WeaponBallistics kDefault{ 300.0f, 1.00f, 0.60f };

		if (!ammo.empty()) {
			auto it = kProfiles.find(weapon + xs("|") + ammo);
			if (it != kProfiles.end()) return it->second;
		}
		auto it = kProfiles.find(weapon);
		return it != kProfiles.end() ? it->second : kDefault;
	}

	static glm::vec3 solve_aim_via_sim(const glm::vec3& shoot_from,
	                                   const glm::vec3& target_pos,
	                                   float muzzle_v, float gravity_mod, float drag)
	{
		const glm::vec3 delta = target_pos - shoot_from;
		const float horiz_dist = std::sqrt(delta.x * delta.x + delta.z * delta.z);
		if (horiz_dist < 1e-3f || muzzle_v < 1.0f) return target_pos;

		const glm::vec3 horiz_unit(delta.x / horiz_dist, 0.0f, delta.z / horiz_dist);
		float lo = -0.6f;
		float hi = 1.2f;
		float best_theta = std::atan2(delta.y, horiz_dist);
		float best_err = 1e9f;
		const bool use_px = features::physx::is_ready();

		for (int i = 0; i < 10; ++i) {
			const float theta = 0.5f * (lo + hi);
			const glm::vec3 aim_dir = glm::normalize(horiz_unit * std::cos(theta) + glm::vec3(0.0f, std::sin(theta), 0.0f));
			const glm::vec3 start_vel = aim_dir * muzzle_v;

			float y_at = target_pos.y;
			bool blocked = false;
			if (use_px) {
				const auto tr = features::physx::trace_ballistic(shoot_from, start_vel, gravity_mod, drag, 4.0f);
				float closest = 1e9f;
				glm::vec3 at = tr.point;
				for (const auto& p : tr.path) {
					const float hx = p.x - shoot_from.x;
					const float hz = p.z - shoot_from.z;
					const float h = std::sqrt(hx * hx + hz * hz);
					const float err = std::abs(h - horiz_dist);
					if (err < closest) {
						closest = err;
						at = p;
					}
				}
				if (tr.hit && glm::length(tr.point - target_pos) > 1.35f &&
					glm::length(tr.point - shoot_from) + 0.5f < horiz_dist)
					blocked = true;
				y_at = at.y;
			} else {
				constexpr float G = 9.81f;
				const float t = horiz_dist / std::max(muzzle_v * std::cos(theta), 1.0f);
				y_at = shoot_from.y + start_vel.y * t - 0.5f * G * gravity_mod * t * t;
			}

			const float y_err = y_at - target_pos.y;
			if (std::abs(y_err) < best_err) {
				best_err = std::abs(y_err);
				best_theta = theta;
			}
			if (blocked || y_err > 0.0f)
				hi = theta;
			else
				lo = theta;
		}

		const glm::vec3 aim_dir = glm::normalize(horiz_unit * std::cos(best_theta) + glm::vec3(0.0f, std::sin(best_theta), 0.0f));
		return shoot_from + aim_dir * std::max(glm::length(delta), horiz_dist);
	}
	static float compute_tof(float horiz_dist, float v0, float drag, float gravity_mod, float delta_y)
	{
		const float g = 9.81f * std::max(gravity_mod, 0.01f);
		const float v2 = v0 * v0;
		const float k = 0.5f * g * horiz_dist * horiz_dist / std::max(v2, 1.0f);

		float theta = std::atan2(delta_y, horiz_dist);
		const float disc = horiz_dist * horiz_dist - 4.0f * k * (delta_y + k);
		if (disc >= 0.0f && k > 1e-6f)
			theta = std::atan((horiz_dist - std::sqrt(std::max(disc, 0.0f))) / (2.0f * k));

		const float v_horiz = v0 * std::cos(theta);

		float tof = horiz_dist / std::max(v_horiz, 1.0f);

		if (drag > 1e-5f)
		{
			const float k_d = drag * 0.025f;
			const float ratio = 1.0f - horiz_dist * k_d / std::max(v_horiz, 1.0f);
			if (ratio > 0.05f)
				tof = -std::log(ratio) / k_d;
			else
				tof = horiz_dist / std::max(v_horiz * 0.5f, 1.0f);
		}

		return tof;
	}
	static glm::vec3 predict_target(const glm::vec3& shoot_from,
	                                rust::Entity* target,
	                                const glm::vec3& aim_point,
	                                float lead_strength,
	                                float drop_strength,
	                                int ping_ms)
	{
	if (!target) return aim_point;

	const std::string& weapon = game::impl::local_player ? game::impl::local_player->item_shortname : std::string{};
	const std::string  ammo   = read_active_ammo_shortname();
	
	WeaponBallistics b = get_ballistics(weapon, ammo);

	const bool is_bow = (weapon.find(xs("bow")) != std::string::npos ||
	                     weapon == xs("crossbow") || weapon == xs("crossbowbowless") ||
	                     weapon.find(xs("legacy")) != std::string::npos);
	const bool is_slow = is_bow || b.muzzle_velocity < 150.0f ||
	                     weapon.find(xs("nailgun")) != std::string::npos ||
	                     weapon.find(xs("grenade")) != std::string::npos ||
	                     weapon.find(xs("rocket")) != std::string::npos;

	const float live_v = read_active_muzzle_velocity();
	if (live_v > 1.0f && live_v < 2000.0f) {
		if (!is_bow || live_v >= b.muzzle_velocity * 0.75f)
			b.muzzle_velocity = live_v;
	}

	if (b.muzzle_velocity <= 0.0f) return aim_point;

	const float ping_s = 0;//std::max(0.0f, ping_ms / 2000.0f); 
		const float gravity_mod = b.gravity_modifier * drop_strength;

		glm::vec3 target_vel = target->velocity;
		bool have_vel = target->has_velocity && glm::length(target_vel) > 0.05f;
		if (!have_vel && target->base_address) {
			const uptr pm = memory::read(target->base_address + offsets::BasePlayer::playerModel);
			if (pm) {
				for (uptr off : { offsets::PlayerModel::newVelocity, offsets::PlayerModel::velocity }) {
					if (!off) continue;
					const glm::vec3 v = memory::read<glm::vec3>(pm + off);
					if (!std::isfinite(v.x) || !std::isfinite(v.y) || !std::isfinite(v.z))
						continue;
					const float speed = glm::length(v);
					if (speed > 0.05f && speed < 50.0f) {
						target_vel = v;
						have_vel = true;
						break;
					}
				}
			}
		}

		const int iterations = is_slow ? 5 : 2;

		glm::vec3 predicted = aim_point;
		for (int i = 0; i < iterations; ++i) {
			const glm::vec3 delta = predicted - shoot_from;
			const float horiz_dist = std::sqrt(delta.x * delta.x + delta.z * delta.z);

			const float tof = compute_tof(
				horiz_dist, b.muzzle_velocity, b.drag, gravity_mod, delta.y);

			const float lead_t = std::max(0.0f, tof) + ping_s;

			predicted = aim_point;
			if (have_vel)
				predicted += target_vel * lead_t * lead_strength;
		}

		return solve_aim_via_sim(shoot_from, predicted, b.muzzle_velocity, gravity_mod, b.drag);
	}

	constexpr const char* weapon_group_names[] = {
		"Rifles", "Snipers", "Shotguns", "Pistols", "Bows", "LMGs", "SMGs"
	};

	static std::string get_group_name(rust::WeaponTier tier)
	{
		switch (tier)
		{
		case rust::WeaponTier::Rifle: return "Rifles";
		case rust::WeaponTier::Sniper: return "Snipers";
		case rust::WeaponTier::Shotgun: return "Shotguns";
		case rust::WeaponTier::Pistol: return "Pistols";
		case rust::WeaponTier::Bow: return "Bows";
		case rust::WeaponTier::LMG: return "LMGs";
		case rust::WeaponTier::SMG: return "SMGs";
		default: return "";
		}
	}

	static int settings_group_index(rust::WeaponTier tier)
	{
		switch (tier)
		{
		case rust::WeaponTier::Rifle:   return 0;
		case rust::WeaponTier::Sniper:  return 1;
		case rust::WeaponTier::Shotgun: return 2;
		case rust::WeaponTier::Pistol:  return 3;
		case rust::WeaponTier::Bow:     return 4;
		case rust::WeaponTier::LMG:     return 5;
		case rust::WeaponTier::SMG:     return 6;
		default: return -1;
		}
	}

	static ActiveConfig get_config_from_group(const Settings::AimbotSettings::WeaponGroupSettings& group)
	{
		const int fov_size = settings.aimbot.visualization.fov_circle_size;

	return {
		group.main.enabled,
		group.main.silent_aim,
		group.prerequisites.scale_by_distance,
		fov_size,
		group.humanization.smoothing,
		static_cast<float>(group.main.silent_aim_hit_chance),
		static_cast<float>(group.main.max_distance),
		group.filters.skip_teammates,
		group.filters.skip_scientists,
		group.filters.skip_dwellers,
		group.filters.skip_sleepers,
		group.filters.skip_wounded,
		group.filters.skip_animals
	};
	}

	ActiveConfig get_active_config()
	{
		auto local = game::impl::local_player;
		if (!local)
		{
			ADD_LOG("no local player");
			return get_config_from_group(settings.aimbot.general);
		}

		if (local->item_shortname.empty())
		{
			ADD_LOG("using general (no item)");
			return get_config_from_group(settings.aimbot.general);
		}

		if (!rust::item_display_names)
			return get_config_from_group(settings.aimbot.general);

		auto it = rust::item_display_names->find(local->item_shortname);
		if (it != rust::item_display_names->end())
		{
			int gi = settings_group_index(it->second.tier);
			if (gi >= 0)
			{
				const auto& group = settings.aimbot.groups[gi];
				if (group.main.enabled)
				{
					ADD_LOG(std::format("using {}", get_group_name(it->second.tier)));
					return get_config_from_group(group);
				}
			}
		}

		ADD_LOG("using general (fb)");
		return get_config_from_group(settings.aimbot.general);
	}

	static bool finite_nonzero(const glm::vec3& v)
	{
		if (!std::isfinite(v.x) || !std::isfinite(v.y) || !std::isfinite(v.z)) return false;
		return v.x != 0.0f || v.y != 0.0f || v.z != 0.0f;
	}

	glm::vec3 get_bone_position(rust::Entity* entity, rust::BoneList bone_id)
	{
		if (auto bone = entity->get_visible_bone(bone_id))
			if (finite_nonzero(bone->position))
				return bone->position;

		if (entity->bones) {
			auto it = entity->bones->find(bone_id);
			if (it != entity->bones->end() && finite_nonzero(it->second.position))
				return it->second.position;
		}

		constexpr float nan = std::numeric_limits<float>::quiet_NaN();
		return { nan, nan, nan };
	}

	glm::vec3 get_local_eye_position()
	{
		auto* lp = game::impl::local_player;
		if (!lp || !lp->base_address) return {};

		if (game::impl::camera_object) {
			const glm::vec3 cam = memory::read<glm::vec3>(game::impl::camera_object + offsets::camera::position);
			if (finite_nonzero(cam) && std::abs(cam.x) < 10000.0f && std::abs(cam.y) < 10000.0f)
				return cam;
		}

		if (auto bone = lp->get_visible_bone(rust::BoneList::head)) {
			if (finite_nonzero(bone->position))
				return bone->position;
		}
		if (lp->bones) {
			auto it = lp->bones->find(rust::BoneList::head);
			if (it != lp->bones->end() && finite_nonzero(it->second.position))
				return it->second.position;
		}

		const uptr eyes_wrap = memory::read(lp->base_address + offsets::BasePlayer::playerEyes);
		if (eyes_wrap) {
			uptr eyes = decryption::player_eyes(eyes_wrap);
			if (!eyes && eyes_wrap > 0x100000000ULL && (eyes_wrap & 1) == 0)
				eyes = eyes_wrap;
			if (eyes) {
				if (offsets::PlayerEyes::worldPosition) {
					const glm::vec3 wp = memory::read<glm::vec3>(eyes + offsets::PlayerEyes::worldPosition);
					if (finite_nonzero(wp))
						return wp;
				}
				if (offsets::PlayerEyes::viewOffset) {
					const glm::vec3 vo = memory::read<glm::vec3>(eyes + offsets::PlayerEyes::viewOffset);
					if (std::isfinite(vo.y) && vo.y > 0.1f && vo.y < 2.5f)
						return lp->origin + vo;
				}
			}
		}

		float eye_h = 1.55f;
		const uptr ms = memory::read(lp->base_address + offsets::BasePlayer::modelState);
		if (ms) {
			const int flags = memory::read<int>(ms + offsets::ModelState::flags);
			if (flags & (offsets::ModelState::Crawling | offsets::ModelState::Prone))
				eye_h = 0.45f;
			else if (flags & offsets::ModelState::Ducked)
				eye_h = 1.05f;
		}
		return lp->origin + glm::vec3{ 0.0f, eye_h, 0.0f };
	}

	bool valid_position(glm::vec3 input)
	{
		if (!std::isfinite(input.x) || !std::isfinite(input.y) || !std::isfinite(input.z))
			return false;
		return input.x != 0.0f || input.y != 0.0f || input.z != 0.0f;
	}

	static rust::BoneList resolve_hit_bone()
	{
		auto* local = game::impl::local_player;
		const Settings::AimbotSettings::WeaponGroupSettings* group = &settings.aimbot.general;
		if (local && !local->item_shortname.empty() && rust::item_display_names) {
			auto it = rust::item_display_names->find(local->item_shortname);
			if (it != rust::item_display_names->end()) {
				int gi = settings_group_index(it->second.tier);
				if (gi >= 0 && settings.aimbot.groups[gi].main.enabled)
					group = &settings.aimbot.groups[gi];
			}
		}
		const int idx = group->main.target_bone;

		switch (idx)
		{
			case 0: return rust::BoneList::head;
			case 1: return rust::BoneList::neck;
			case 2: return rust::BoneList::spine4;
			case 3: return rust::BoneList::spine2;
			case 4: return rust::BoneList::pelvis;
			case 5: return rust::BoneList::r_knee;
			default: return rust::BoneList::head;
		}
	}

	static const Settings::AimbotSettings::WeaponGroupSettings& active_group()
	{
		auto* local = game::impl::local_player;
		if (local && !local->item_shortname.empty() && rust::item_display_names) {
			auto it = rust::item_display_names->find(local->item_shortname);
			if (it != rust::item_display_names->end()) {
				int gi = settings_group_index(it->second.tier);
				if (gi >= 0 && settings.aimbot.groups[gi].main.enabled)
					return settings.aimbot.groups[gi];
			}
		}
		return settings.aimbot.general;
	}

	static glm::vec3 apply_prediction(const glm::vec3& shoot_from, rust::Entity* entity, glm::vec3 bone)
	{
		const auto& g = active_group();
		if (!g.main.prediction)
			return bone;
		return predict_target(shoot_from, entity, bone, g.main.prediction_strength, g.main.prediction_drop, g.main.prediction_ping_comp);
	}

	f32 get_field_of_view(glm::vec2 target_position)
	{
		const glm::vec2 center = winapp::impl::window_size * 0.5f;

		return glm::distance(target_position, center);
	}

	rust::Entity* get_best_target()
	{
		ActiveConfig config = get_active_config();
		rust::Entity* best_target{ nullptr };

		f32 best_field_of_view = config.field_of_view;
		f32 best_world_distance = std::numeric_limits<f32>::max();
		i32 best_priority = 99;
		visualized_fov = config.field_of_view;

		const bool smart_target = settings.aimbot.general.main.smart_target;
		const bool auto_shoot   = settings.aimbot.general.main.auto_shoot;
		const bool target_360 = settings.aimbot.general.main.target_360 || auto_shoot;
		const bool prefer_world_distance = smart_target || target_360;

		auto is_animal = [](EntityType type) -> bool {
			return type == EntityType::Bear || type == EntityType::Wolf || type == EntityType::Stag ||
				type == EntityType::Boar || type == EntityType::Horse || type == EntityType::Chicken ||
				type == EntityType::Shark;
			};

		cache::for_each_entity<rust::Entity>([&](rust::Entity* entity) {
			if (!entity->base_address)
				return;

			if (entity->base_address == game::impl::local_player->base_address)
				return;

		if (config.skip_teammates &&
			entity->type == EntityType::Player &&
			entity->team != 0 &&
			entity->team == game::impl::local_player->team)
			return;

			if (entity->distance > config.max_distance && config.max_distance < 500.0f)
				return;

			i32 priority = 99;
			if (entity->type == EntityType::Player) {
				if (entity->is_sleeping && config.skip_sleepers) return;
				if (entity->is_wounded && config.skip_wounded) return;
				priority = 0;
			}
			else if (entity->type == EntityType::Scientist) {
				if (config.skip_scientists) return;
				priority = 1;
			}
			else if (entity->type == EntityType::Dweller) {
				if (config.skip_dwellers) return;
				priority = 1;
			}
			else if (is_animal(entity->type)) {
				if (config.skip_animals) return;
				priority = 2;
			}
			else {
				return;
			}

			glm::vec3 target_bone = get_bone_position(entity, resolve_hit_bone());
			if (!valid_position(target_bone))
				return;

			const glm::vec3 shoot_from = get_local_eye_position();
			if (valid_position(shoot_from)) {
				target_bone = apply_prediction(shoot_from, entity, target_bone);
				if (!valid_position(target_bone))
					return;
			}
			if (settings.visuals.players.vischeck && features::physx::is_ready() && valid_position(shoot_from)) {
				if (!features::physx::is_visible(shoot_from, target_bone))
					return;
			}

			glm::vec2 out{};
			const bool on_screen = math::world_to_screen(target_bone, &out);
			if (!on_screen && !target_360)
				return;

			const f32 field_of_view = on_screen
				? get_field_of_view(out)
				: std::numeric_limits<f32>::max();

			f32 target_fov = config.field_of_view;
			if (config.distance_fov_scaling && entity->distance > 20.0f)
			{
				target_fov = (config.field_of_view * 20.0f) / entity->distance;
				if (target_fov < 1.0f) target_fov = 1.0f;
			}

			if (target_360 || field_of_view <= target_fov)
			{
				if (priority < best_priority)
				{
					best_priority = priority;
					best_target = entity;
					best_field_of_view = field_of_view;
					best_world_distance = entity->distance;
					visualized_fov = target_fov;
				}
				else if (priority == best_priority)
				{
					if (prefer_world_distance)
					{
						if (entity->distance < best_world_distance)
						{
							best_target = entity;
							best_field_of_view = field_of_view;
							best_world_distance = entity->distance;
							visualized_fov = target_fov;
						}
					}
					else if (field_of_view <= best_field_of_view)
					{
						best_target = entity;
						best_field_of_view = field_of_view;
						best_world_distance = entity->distance;
						visualized_fov = target_fov;
					}
				}
			}
			});

		current_target = best_target;
		return best_target;
	}

	glm::vec2 calculate_angle(const glm::vec3& local_head_pos, const glm::vec3& target_position)
	{
		const glm::vec3 a = math::calc_angle(local_head_pos, target_position);
		return glm::vec2(a.x, a.y);
	}

	glm::vec4 to_quat(glm::vec3 euler) {
		const float half_pitch = euler.x * (3.14159265f / 180.0f) * 0.5f;
		const float half_yaw = euler.y * (3.14159265f / 180.0f) * 0.5f;
		const float cosine_pitch = std::cos(half_pitch);
		const float sine_pitch = std::sin(half_pitch);
		const float cosine_yaw = std::cos(half_yaw);
		const float sine_yaw = std::sin(half_yaw);

		glm::vec4 rotation{
			sine_pitch * cosine_yaw,
			cosine_pitch * sine_yaw,
			-(sine_pitch * sine_yaw),
			cosine_pitch * cosine_yaw
		};

		const float length = std::sqrt(
			rotation.x * rotation.x +
			rotation.y * rotation.y +
			rotation.z * rotation.z +
			rotation.w * rotation.w);
		if (length > 0.001f)
		{
			rotation.x /= length;
			rotation.y /= length;
			rotation.z /= length;
			rotation.w /= length;
		}
		return rotation;
	}

	void on_tick()
	{
		AutoShootGuard autoshoot_guard{};

		std::unique_lock<std::recursive_mutex> entity_lock(cache::cache_mut, std::try_to_lock);
		if (!entity_lock.owns_lock()) return;

		std::unique_lock<std::shared_mutex> lock(debug_mut);
		debug_info.swap(safe_debug_info);
		debug_info.clear();

		current_target = nullptr;

		if (!game::is_in_game())
			ADD_LOG_RET("awaiting map");

		if (!game::impl::local_player)
			ADD_LOG_RET("invalid local player");

		ActiveConfig config = get_active_config();
		if (!config.enabled)
			ADD_LOG_RET("config disabled");

		glm::vec3 local_head_position = get_local_eye_position();
		if (!valid_position(local_head_position))
			ADD_LOG_RET(std::format("invalid local position [{}, {}, {}]", local_head_position.x, local_head_position.y, local_head_position.z));

		rust::Entity* target = get_best_target();
		if (!target)
		{
			target_position_2d = {};

			ADD_LOG("no target");
			return;
		}

		ADD_LOG(std::format("target: {} [{}m]", target->name, static_cast<i32>(target->distance)));

		glm::vec3 target_bone = get_bone_position(target, resolve_hit_bone());
		if (!valid_position(target_bone))
			ADD_LOG_RET("no target position");

		{
			target_bone = apply_prediction(local_head_position, target, target_bone);
			if (!valid_position(target_bone))
				ADD_LOG_RET("no target position (post-predict)");
		}

		const bool auto_shoot = settings.aimbot.general.main.auto_shoot;
		const bool target_360 = settings.aimbot.general.main.target_360 || auto_shoot;

		glm::vec2 out{};
		const bool on_screen = math::world_to_screen(target_bone, &out);
		if (!on_screen && !target_360)
			ADD_LOG_RET("can't see");
		if (on_screen)
			target_position_2d = out;
		else
			target_position_2d = {};

		glm::vec2 target_angle = calculate_angle(local_head_position, target_bone);
		if (target_angle.x == 0.0f && target_angle.y == 0.0f)
			ADD_LOG_RET("invalid math operation");

		uptr input = memory::read(game::impl::local_player->base_address + offsets::BasePlayer::playerInput);
		if (!input)
			ADD_LOG_RET("no player input");

		glm::vec2 current_angles = memory::read<glm::vec2>(input + offsets::PlayerInput::bodyAngles);

		glm::vec2 delta = target_angle - current_angles;
		while (delta.x > 180.0f)  delta.x -= 360.0f;
		while (delta.x < -180.0f) delta.x += 360.0f;
		while (delta.y > 180.0f)  delta.y -= 360.0f;
		while (delta.y < -180.0f) delta.y += 360.0f;
		delta.x = std::clamp(delta.x, -89.0f, 89.0f);

		const f32 divisor = std::max(config.smoothing, 1.0f);
		glm::vec2 smoothed_target_angle = current_angles + (delta / divisor);

		const int aim_vk = []{ int v = settings.aimbot.aim_key.key; return v ? v : VK_RBUTTON; }();
		const bool aim_held = (GetAsyncKeyState(aim_vk) & 0x8000) != 0;

		glm::vec3 view_fwd{};
		math::angle_vectors(game::impl::local_player->view_angles, &view_fwd, nullptr);
		const glm::vec3 to_tgt = target_bone - local_head_position;
		const float to_len = glm::length(to_tgt);
		const bool behind = (to_len > 0.01f) && (glm::dot(view_fwd, to_tgt / to_len) < 0.2f);
		const bool use_silent = config.silent_aim;
		uptr player_eyes = unity::get_component_by_name(
			game::impl::local_player->base_address, "PlayerEyes");
		if (!player_eyes) {
			const uptr eyes_wrap = memory::read(
				game::impl::local_player->base_address + offsets::BasePlayer::playerEyes);
			if (eyes_wrap) {
				player_eyes = decryption::player_eyes(eyes_wrap);
				if (!player_eyes && eyes_wrap > 0x100000000ULL && (eyes_wrap & 1) == 0)
					player_eyes = eyes_wrap;
			}
		}
		if (!player_eyes)
			ADD_LOG_RET("no player eyes");
		if (aim_held) {
			if (use_silent)
			{
				bool pass_roll = true;
				const int chance = static_cast<int>(config.silent_hit_chance);
				if (chance < 100 && !auto_shoot) {
					static std::mt19937 rng{ std::random_device{}() };
					std::uniform_int_distribution<int> dist(1, 100);
					pass_roll = (dist(rng) <= chance);
				}

				if (!pass_roll)
					ADD_LOG_RET("hit chance roll fail");
				if (!player_eyes)
					ADD_LOG_RET("no player eyes");
				
				
				memory::write<glm::vec4>(player_eyes + offsets::PlayerEyes::bodyRotation,
					to_quat(glm::vec3{ target_angle.x, target_angle.y, 0.0f }));
				ADD_LOG(auto_shoot ? "wrote angle [silent+autoshoot]" : "wrote angle [silent]");
			}
			else if (glm::length(delta) >= 0.01f) {
				memory::write<glm::vec2>(input + offsets::PlayerInput::bodyAngles, smoothed_target_angle);
				ADD_LOG("wrote angle [not silent]");
			}

			if (auto_shoot) {
				if (settings.aimbot.weapons.instant_bow) {
					if (uptr held = features::held_weapon_or_zero()) {
						if (offsets::bow_weapon::attackReady)
							memory::write<bool>(held + offsets::bow_weapon::attackReady, true);
					}
				}
				autoshoot_guard.fire = true;
			}
		}

		ADD_LOG_RET("aimbot ret");
	}

	void on_render()
	{
		std::unique_lock<std::recursive_mutex> entity_lock(cache::cache_mut, std::try_to_lock);
		if (!entity_lock.owns_lock()) return;

		f32 offset{ 0.0f };
		for (auto& item : safe_debug_info)
		{
			glm::vec2 position = glm::vec2{ winapp::impl::window_size.x - 5.0f, 5.0f + offset };
			render::add_text(render::Fonts::Arial14px, item, position, Color::white(), render::TextFlagsDropShadow, glm::vec2{ 1.0f, 0.0f });

			offset += 14.0f;
		}

		ActiveConfig config = get_active_config();

		bool path_on = config.enabled && settings.aimbot.visualization.prediction_path;
		if (path_on && game::impl::local_player)
		{
			const std::string& weapon = game::impl::local_player->item_shortname;
			const std::string  ammo   = read_active_ammo_shortname();
			WeaponBallistics b = get_ballistics(weapon, ammo);
			const float live_v = read_active_muzzle_velocity();
			if (live_v > 1.0f) b.muzzle_velocity = live_v;

			const glm::vec3 shoot_from = get_local_eye_position();
			if (b.muzzle_velocity > 1.0f && valid_position(shoot_from))
			{
				glm::vec3 view_fwd{};
				glm::vec3 view_right{};
				math::angle_vectors(game::impl::local_player->view_angles, &view_fwd, &view_right);

				glm::vec3 fire_dir = view_fwd;
				bool used_silent_dir = false;

				const int silent_vk = []{ int v = settings.aimbot.aim_key.key; return v ? v : VK_RBUTTON; }();
				const bool silent_active = config.silent_aim &&
					((GetAsyncKeyState(silent_vk) & 0x8000) != 0);
				if (silent_active && current_target)
				{
					glm::vec3 target_bone = get_bone_position(current_target, resolve_hit_bone());
					if (valid_position(target_bone))
					{
						{
							target_bone = apply_prediction(shoot_from, current_target, target_bone);
						}
						if (valid_position(target_bone))
						{
							const glm::vec3 delta = target_bone - shoot_from;
							const float len = glm::length(delta);
							if (len > 0.01f)
							{
								fire_dir = delta / len;
								used_silent_dir = true;
							}
						}
					}
				}

				glm::vec3 muzzle = shoot_from
					+ view_fwd   * 0.35f
					+ view_right * 0.12f
					+ glm::vec3(0.0f, -0.18f, 0.0f);
				bool got_muzzle = false;
				{
					uptr item_ptr = game::impl::local_player->active_item;
					uptr held = item_ptr ? memory::read<uptr>(item_ptr + offsets::Item::heldEntity) : 0;
					if (held) {
						uptr native = memory::read<uptr>(held + offsets::UnityObject::cached_ptr);
						uptr go     = native ? memory::read<uptr>(native + offsets::UnityComponent::game_object) : 0;
						uptr comps  = go ? memory::read<uptr>(go + offsets::UnityGameObject::components) : 0;
						uptr ntrans = comps ? memory::read<uptr>(comps + offsets::UnityGameObject::component_ptr_in_entry) : 0;
						uptr access = ntrans ? memory::read<uptr>(ntrans + offsets::UnityTransform::indirect_ptr_off) : 0;
						if (access) {
							glm::vec3 wpos;
							if (try_read_transform_world_pos(access, wpos) &&
								(wpos.x != 0.0f || wpos.y != 0.0f || wpos.z != 0.0f))
							{
								muzzle = wpos + view_fwd * 0.40f;
								got_muzzle = true;
							}
						}
					}
				}
				if (!got_muzzle && game::impl::local_player->bones)
				{
					auto it = game::impl::local_player->bones->find(rust::BoneList::r_hand);
					if (it != game::impl::local_player->bones->end() && it->second.visible)
						muzzle = it->second.position + view_fwd * 0.35f;
				}

				const Color path_color = settings.aimbot.visualization.prediction_path_color;
				const glm::vec3 vel = fire_dir * b.muzzle_velocity;
				std::vector<glm::vec3> world_path;
				if (features::physx::is_ready())
				{
					const auto tr = features::physx::trace_ballistic(muzzle, vel, b.gravity_modifier, b.drag, 2.5f);
					world_path = tr.path;
				}
				else
				{
					constexpr float dt = 0.05f;
					constexpr float G = 9.81f;
					glm::vec3 pos = muzzle;
					glm::vec3 v = vel;
					world_path.push_back(pos);
					for (float t = 0.0f; t < 2.5f; t += dt)
					{
						pos += v * dt;
						v.y -= G * b.gravity_modifier * dt;
						v -= v * b.drag * dt;
						world_path.push_back(pos);
						if (pos.y < muzzle.y - 200.0f) break;
					}
				}

				std::vector<glm::vec2> segment;
				segment.reserve(world_path.size());
				for (const auto& p : world_path)
				{
					glm::vec2 sp{};
					if (math::world_to_screen(p, &sp))
						segment.push_back(sp);
					else if (segment.size() >= 2)
					{
						render::add_polyline(segment, path_color, 1.5f);
						segment.clear();
					}
					else
						segment.clear();
				}
				if (segment.size() >= 2)
					render::add_polyline(segment, path_color, 1.5f);
			}
		}

		bool fov_circle = settings.aimbot.visualization.fov_circle;
		if (!fov_circle)
			return;

		Color color = settings.aimbot.visualization.fov_circle_color;

		bool target_line = settings.aimbot.visualization.fov_circle_target_line;
		Color target_line_color = settings.aimbot.visualization.fov_circle_target_line_color;

		const float fov_px = config.enabled
			? visualized_fov
			: static_cast<float>(settings.aimbot.visualization.fov_circle_size);
		static gui::Animation anim{ fov_px };
		anim.update(fov_px);

		glm::vec2 center = winapp::impl::window_size * 0.5f;

		if (target_line && config.enabled && target_position_2d.x != 0.0f && target_position_2d.y != 0.0f)
		{
			render::add_line(center, target_position_2d, target_line_color);
		}

		render::add_circle(center, anim, color, 1.5f);
	}
}