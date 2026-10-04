#pragma once
#include "bones.hpp"
#include <base/animation/animation.hpp>
#include <chrono>
#include <mutex>
#include <optional>
#include <render/texture/texture.hpp>
#include <sdk/globals.hpp>
#include <string>
#include <unordered_map>
#include <utils/md5.hpp>

__forceinline consteval uint32_t get_prefab_id(const char* str) {
	return (uint32_t(md5::compute(str)[0])) | (uint32_t(md5::compute(str)[1]) << 8) | (uint32_t(md5::compute(str)[2]) << 16) | (uint32_t(md5::compute(str)[3]) << 24);
}

enum PlayerFlags : int {
	Unused1 = 1,
	Unused2 = 2,
	IsAdmin = 4,
	ReceivingSnapshot = 8,
	Sleeping = 16,
	Spectating = 32,
	Wounded = 64,
	IsDeveloper = 128,
	Connected = 256,
	VoiceMuted = 512,
	ThirdPersonViewmode = 1024,
	EyesViewmode = 2048,
	ChatMute = 4096,
	NoSprint = 8192,
	Aiming = 16384,
	DisplaySash = 32768,
	Relaxed = 65536,
	SafeZone = 131072,
	ServerFall = 262144,
	Incapacitated = 524288,
	Workbench1 = 1048576,
	Workbench2 = 2097152,
	Workbench3 = 4194304,
	ModifyClan = 8388608,
	BuffCold = 16777216,
	BuffHot = 33554432,
	BuffTea = 67108864,
	LoadingAfterTransfer = 134217728
};

enum EntityPrefabs : u32 {
	invalid = -1,
	player = get_prefab_id("assets/prefabs/player/player.prefab"),
	scientist = get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_oilrig.prefab"),
	boar = get_prefab_id("assets/rust.ai/agents/boar/boar.prefab")
};

enum EntityType
{
	None = -1,

	Player,
	Scientist,

	Dweller,
	Bear,
	Wolf,
	Stag,
	Boar,
	Horse,
	Chicken,
	Shark,

	Stone,
	Sulfur,
	Metal,
	Hemp,
	Wood,

	EliteCrate,
	MilitaryCrate,
	Crate,
	AirDrop,
	LockedCrate,
	AutoTurret,
	FlameTurret,
	ShotgunTrap,
	Corpse,

	WoodenBox, LargeWoodBox, ToolCupboard, Workbench1Alt, Workbench2Alt, Workbench3Alt,
	RepairBench, ResearchTable, SmallStash, SleepingBag, Furnace, Locker,
	LootBarrel, OilBarrel, GreenKeycard, BlueKeycard, RedKeycard,
	FoodCrate, VehicleParts, ToolCrate, SmallCrate, DieselFuel,
	Recycler, PhoneBooth, FuseBox, Refinery, HealthCrate, Elevator,
	Scarecrow, SmallFoodCrate, CarLift, ComputerStation,
	UnderwaterCrate,

	Motorbike, SidecarMotorbike, PedalBike,
	Snowmobile, TomahaSnowmobile,
	Tugboat, Rowboat, RHIB, Kyak,
	SoloSubmarine, DuoSubmarine,
	Minicopter, ScrapHeli, AttackHeli,

	DroppedRifle, DroppedSniper, DroppedSMG, DroppedShotgun,
	DroppedPistol, DroppedLMG, DroppedLauncher, DroppedBow,
	DroppedMelee,
	DroppedThrowable, DroppedTool, DroppedMedical, DroppedAmmo,
	DroppedMisc

};

struct PrefabEntry
{
	std::string_view name{};
	EntityType type{ EntityType::None };
};

inline std::unordered_map<u32, PrefabEntry>* prefabs = new std::unordered_map<u32, PrefabEntry>(
{
	{ get_prefab_id("assets/prefabs/player/player.prefab"), { "Player", EntityType::Player } },
	{ get_prefab_id("UKN Static Bots"), { "Player", EntityType::Player } },
	{ get_prefab_id("assets/prefabs/player/player_corpse.prefab"), { "Corpse", EntityType::Corpse } },
	{ get_prefab_id("assets/prefabs/npc/patrol helicopter/patrolhelicopter.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_arena.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_cargo.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_cargo_turret_any.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_cargo_turret_lr300.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_ch47_gunner.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_excavator.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_full_any.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_full_lr300.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_full_mp5.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_full_pistol.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_full_shotgun.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_heavy.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_junkpile_pistol.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_oilrig.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_patrol.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_patrol_arctic.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_peacekeeper.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_roam.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_roam_nvg_variant.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/scientist/scientistnpc_roamtethered.prefab"), { "Scientist", EntityType::Scientist } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/tunneldweller/npc_tunneldweller.prefab"), { "Dweller", EntityType::Dweller } },
	{ get_prefab_id("assets/rust.ai/agents/npcplayer/humannpc/underwaterdweller/npc_underwaterdweller.prefab"), { "Dweller", EntityType::Dweller } },
	{ get_prefab_id("assets/bundled/prefabs/autospawn/resource/ores/metal-ore.prefab"), { "Metal", EntityType::Metal } },
	{ get_prefab_id("assets/bundled/prefabs/autospawn/resource/ores/stone-ore.prefab"), { "Stone", EntityType::Stone } },
	{ get_prefab_id("assets/bundled/prefabs/autospawn/resource/ores/sulfur-ore.prefab"), { "Sulfur", EntityType::Sulfur } },
	{ get_prefab_id("assets/bundled/prefabs/autospawn/collectable/hemp/hemp-collectable.prefab"), { "Hemp", EntityType::Hemp } },
	{ get_prefab_id("assets/bundled/prefabs/autospawn/resource/wood_log_pile/wood-pile.prefab"), { "Wood", EntityType::Wood } },
	{ get_prefab_id("assets/bundled/prefabs/autospawn/collectable/wood/wood-collectable.prefab"), { "Wood", EntityType::Wood } },
	{ get_prefab_id("assets/rust.ai/agents/bear/bear.prefab"), { "Bear", EntityType::Bear } },
	{ get_prefab_id("assets/rust.ai/agents/bear/polarbear.prefab"), { "Polar Bear", EntityType::Bear } },
	{ get_prefab_id("assets/rust.ai/agents/wolf/wolf.prefab"), { "Wolf", EntityType::Wolf } },
	{ get_prefab_id("assets/rust.ai/agents/stag/stag.prefab"), { "Stag", EntityType::Stag } },
	{ get_prefab_id("assets/rust.ai/agents/boar/boar.prefab"), { "Boar", EntityType::Boar } },
	{ get_prefab_id("assets/rust.ai/agents/chicken/chicken.prefab"), { "Chicken", EntityType::Chicken } },
	{ get_prefab_id("assets/rust.ai/agents/fish/simpleshark.prefab"), { "Shark", EntityType::Shark } },
	{ get_prefab_id("assets/bundled/prefabs/radtown/crate_elite.prefab"), { "Elite crate", EntityType::EliteCrate } },
	{ get_prefab_id("assets/bundled/prefabs/radtown/crate_normal.prefab"), { "Military crate", EntityType::MilitaryCrate } },
	{ get_prefab_id("assets/bundled/prefabs/radtown/crate_normal_2.prefab"), { "Crate", EntityType::Crate } },
	{ get_prefab_id("assets/prefabs/deployable/chinooklockedcrate/codelockedhackablecrate.prefab"), { "Locked crate", EntityType::LockedCrate } },
	{ get_prefab_id("assets/prefabs/misc/supply drop/supply_drop.prefab"), { "Air drop", EntityType::AirDrop } },
	{ get_prefab_id("assets/prefabs/npc/autoturret/autoturret_deployed.prefab"), { "Turret", EntityType::AutoTurret } },
	{ get_prefab_id("assets/prefabs/npc/flame turret/flameturret.deployed.prefab"), { "Flame turret", EntityType::FlameTurret } },
	{ get_prefab_id("assets/prefabs/deployable/single shot trap/guntrap.deployed.prefab"), { "Shotgun trap", EntityType::ShotgunTrap } },
	{ get_prefab_id("assets/prefabs/misc/item drop/item_drop_backpack.prefab"), { "Corpse", EntityType::Corpse } },

	{ get_prefab_id("assets/prefabs/deployable/woodenbox/woodbox_deployed.prefab"), { "Wooden box", EntityType::WoodenBox } },
	{ get_prefab_id("assets/prefabs/deployable/large wood storage/box.wooden.large.prefab"), { "Large wood box", EntityType::LargeWoodBox } },
	{ get_prefab_id("assets/prefabs/deployable/tool cupboard/cupboard.tool.deployed.prefab"), { "Tool cupboard", EntityType::ToolCupboard } },
	{ get_prefab_id("assets/prefabs/deployable/tier 1 workbench/workbench1.deployed.prefab"), { "Tier 1 workbench", EntityType::Workbench1Alt } },
	{ get_prefab_id("assets/prefabs/deployable/tier 2 workbench/workbench2.deployed.prefab"), { "Tier 2 workbench", EntityType::Workbench2Alt } },
	{ get_prefab_id("assets/prefabs/deployable/tier 3 workbench/workbench3.deployed.prefab"), { "Tier 3 workbench", EntityType::Workbench3Alt } },
	{ get_prefab_id("assets/prefabs/deployable/repair bench/repairbench_deployed.prefab"), { "Repair bench", EntityType::RepairBench } },
	{ get_prefab_id("assets/prefabs/deployable/research table/researchtable_deployed.prefab"), { "Research table", EntityType::ResearchTable } },
	{ get_prefab_id("assets/prefabs/deployable/small stash/small_stash_deployed.prefab"), { "Small stash", EntityType::SmallStash } },
	{ get_prefab_id("assets/prefabs/deployable/sleeping bag/sleepingbag_leather_deployed.prefab"), { "Sleeping bag", EntityType::SleepingBag } },
	{ get_prefab_id("assets/prefabs/deployable/furnace/furnace.prefab"), { "Furnace", EntityType::Furnace } },
	{ get_prefab_id("assets/prefabs/deployable/locker/locker.deployed.prefab"), { "Locker", EntityType::Locker } },
	{ get_prefab_id("assets/bundled/prefabs/radtown/loot_barrel_2.prefab"), { "Loot barrel", EntityType::LootBarrel } },
	{ get_prefab_id("assets/bundled/prefabs/radtown/loot_barrel_1.prefab"), { "Loot barrel", EntityType::LootBarrel } },
	{ get_prefab_id("assets/bundled/prefabs/radtown/oil_barrel.prefab"), { "Oil barrel", EntityType::OilBarrel } },
	{ get_prefab_id("assets/prefabs/misc/keycard/keycard_green.prefab"), { "Green keycard", EntityType::GreenKeycard } },
	{ get_prefab_id("assets/prefabs/misc/keycard/keycard_blue.prefab"), { "Blue keycard", EntityType::BlueKeycard } },
	{ get_prefab_id("assets/prefabs/misc/keycard/keycard_red.prefab"), { "Red keycard", EntityType::RedKeycard } },
	{ get_prefab_id("assets/bundled/prefabs/radtown/crate_normal_2_food.prefab"), { "Food crate", EntityType::FoodCrate } },
	{ get_prefab_id("assets/bundled/prefabs/radtown/vehicle_parts.prefab"), { "Vehicle parts", EntityType::VehicleParts } },
	{ get_prefab_id("assets/bundled/prefabs/radtown/crate_tools.prefab"), { "Tool crate", EntityType::ToolCrate } },
	{ get_prefab_id("assets/bundled/prefabs/radtown/crate_basic.prefab"), { "Small crate", EntityType::SmallCrate } },
	{ get_prefab_id("assets/content/structures/excavator/prefabs/diesel_collectable.prefab"), { "Diesel fuel", EntityType::DieselFuel } },
	{ get_prefab_id("assets/bundled/prefabs/static/recycler_static.prefab"), { "Recycler", EntityType::Recycler } },
	{ get_prefab_id("assets/bundled/prefabs/autospawn/phonebooth/phonebooth.static.prefab"), { "Phone booth", EntityType::PhoneBooth } },
	{ get_prefab_id("assets/prefabs/io/electric/switches/fusebox/fusebox.prefab"), { "Fuse box", EntityType::FuseBox } },
	{ get_prefab_id("assets/bundled/prefabs/radtown/crate_mine.prefab"), { "Crate", EntityType::Crate } },
	{ get_prefab_id("assets/bundled/prefabs/static/small_refinery_static.prefab"), { "Refinery", EntityType::Refinery } },
	{ get_prefab_id("assets/bundled/prefabs/radtown/crate_normal_2_medical.prefab"), { "Health crate", EntityType::HealthCrate } },
	{ get_prefab_id("assets/prefabs/deployable/elevator/static/elevator_lift.static.prefab"), { "Elevator", EntityType::Elevator } },
	{ get_prefab_id("assets/prefabs/npc/scarecrow/scarecrow.prefab"), { "Scarecrow", EntityType::Scarecrow } },
	{ get_prefab_id("assets/bundled/prefabs/radtown/underwater_labs/crate_food_1.prefab"), { "Small food crate", EntityType::SmallFoodCrate } },
	{ get_prefab_id("assets/bundled/prefabs/static/modularcarlift.static.prefab"), { "Car lift", EntityType::CarLift } },
	{ get_prefab_id("assets/prefabs/deployable/computerstation/computerstation.static.prefab"), { "Computer station", EntityType::ComputerStation } },

	{ get_prefab_id("assets/content/vehicles/bikes/motorbike.prefab"), { "Motorbike", EntityType::Motorbike } },
	{ get_prefab_id("assets/content/vehicles/bikes/motorbike_sidecar.prefab"), { "Sidecar motorbike", EntityType::SidecarMotorbike } },
	{ get_prefab_id("assets/content/vehicles/bikes/pedalbike.prefab"), { "Pedal bike", EntityType::PedalBike } },

	{ get_prefab_id("assets/content/vehicles/snowmobiles/snowmobile.prefab"), { "Snowmobile", EntityType::Snowmobile } },
	{ get_prefab_id("assets/content/vehicles/snowmobiles/tomahasnowmobile.prefab"), { "Tomaha snowmobile", EntityType::TomahaSnowmobile } },

	{ get_prefab_id("assets/content/vehicles/boats/tugboat/tugboat.prefab"), { "Tugboat", EntityType::Tugboat } },
	{ get_prefab_id("assets/content/vehicles/boats/rowboat/rowboat.prefab"), { "Rowboat", EntityType::Rowboat } },
	{ get_prefab_id("assets/content/vehicles/boats/rhib/rhib.prefab"), { "RHIB", EntityType::RHIB } },
	{ get_prefab_id("assets/content/vehicles/boats/kyak/kyak.prefab"), { "Kyak", EntityType::Kyak } },

	{ get_prefab_id("assets/content/vehicles/submarine/submarinesolo.entity.prefab"), { "Solo submarine", EntityType::SoloSubmarine } },
	{ get_prefab_id("assets/content/vehicles/submarine/submarineduo.entity.prefab"), { "Duo submarine", EntityType::DuoSubmarine } },

	{ get_prefab_id("assets/content/vehicles/minicopter/minicopter.entity.prefab"), { "Minicopter", EntityType::Minicopter } },
	{ get_prefab_id("assets/content/vehicles/scrap heli carrier/scraptransporthelicopter.prefab"), { "Scrap heli", EntityType::ScrapHeli } },
	{ get_prefab_id("assets/content/vehicles/attackhelicopter/attackhelicopter.entity.prefab"), { "Attack heli", EntityType::AttackHeli } },

	{ 545786656u,  { "Dropped item", EntityType::DroppedMisc } },
	{ 3255145925u, { "Dropped item", EntityType::DroppedMisc } },
});

inline std::unordered_map<u32, std::string>* weapon_held_shortnames = new std::unordered_map<u32, std::string>(
{
	{ 0x32542451u, "rifle.lr300" },
	{ 0x97b99b77u, "smg.mp5" },
	{ 0x75f12c79u, "rifle.ak" },
	{ 0xc15a0047u, "smg.thompson" },
	{ 0xc966667fu, "rifle.semiauto" },
	{ 0xfc0a3d6du, "rifle.semiauto" },
	{ 0xfdd8471fu, "rifle.m16a2" },
	{ 0xa90efc69u, "bow.hunting" },

	{ get_prefab_id("assets/prefabs/weapons/thompson/world/v_thompson_world.prefab"),     "smg.thompson" },
	{ get_prefab_id("assets/prefabs/weapons/ak47u/world/v_ak_world.prefab"),              "rifle.ak" },
	{ get_prefab_id("assets/prefabs/weapons/lr-300 assault rifle/world/v_lr300_world.prefab"), "rifle.lr300" },
	{ get_prefab_id("assets/prefabs/weapons/m39 rifle/world/v_m39_world.prefab"),         "rifle.m39" },
	{ get_prefab_id("assets/prefabs/weapons/m92 pistol/world/v_m92_world.prefab"),        "pistol.m92" },
	{ get_prefab_id("assets/prefabs/weapons/mp5/world/v_mp5_world.prefab"),               "smg.mp5" },
	{ get_prefab_id("assets/prefabs/weapons/python revolver/world/v_python_world.prefab"), "pistol.python" },
	{ get_prefab_id("assets/prefabs/weapons/semi auto pistol/world/v_semiautopistol_world.prefab"), "pistol.semiauto" },
	{ get_prefab_id("assets/prefabs/weapons/semi-automatic rifle/world/v_semiautorifle_world.prefab"), "rifle.semiauto" },
	{ get_prefab_id("assets/prefabs/weapons/bolt action rifle/world/v_bolt_world.prefab"), "rifle.bolt" },
	{ get_prefab_id("assets/prefabs/weapons/l96/world/v_l96_world.prefab"),               "rifle.l96" },
	{ get_prefab_id("assets/prefabs/weapons/hmlmg/world/v_hmlmg_world.prefab"),           "lmg.m249" },
	{ get_prefab_id("assets/prefabs/weapons/hmlmg/world/v_lmg_world.prefab"),             "lmg.m249" },
	{ get_prefab_id("assets/prefabs/weapons/custom smg/world/v_custom_smg_world.prefab"),  "smg.2" },
	{ get_prefab_id("assets/prefabs/weapons/double barrel shotgun/world/v_doubleshotgun_world.prefab"), "shotgun.double" },
	{ get_prefab_id("assets/prefabs/weapons/pump shotgun/world/v_shotgun_world.prefab"),  "shotgun.pump" },
	{ get_prefab_id("assets/prefabs/weapons/spas12 shotgun/world/v_spas12_world.prefab"), "shotgun.spas12" },
	{ get_prefab_id("assets/prefabs/weapons/waterpipe shotgun/world/v_waterpipe_world.prefab"), "shotgun.waterpipe" },
	{ get_prefab_id("assets/prefabs/weapons/eoka pistol/world/v_eoka_world.prefab"),      "pistol.eoka" },
	{ get_prefab_id("assets/prefabs/weapons/nailgun/world/v_nailgun_world.prefab"),       "pistol.nailgun" },
	{ get_prefab_id("assets/prefabs/weapons/revolver/world/v_revolver_world.prefab"),     "pistol.revolver" },
	{ get_prefab_id("assets/prefabs/weapons/crossbow/world/v_crossbow_world.prefab"),     "crossbow" },
	{ get_prefab_id("assets/prefabs/weapons/bow/world/v_bow_world.prefab"),               "bow.hunting" },
	{ get_prefab_id("assets/prefabs/weapons/compound bow/world/v_compoundbow_world.prefab"), "bow.compound" },
	{ get_prefab_id("assets/prefabs/weapons/flamethrower/world/v_flamethrower_world.prefab"), "flamethrower" },
	{ get_prefab_id("assets/prefabs/weapons/grenade launcher/world/v_grenadelauncher_world.prefab"), "multiplegrenadelauncher" },
	{ get_prefab_id("assets/prefabs/weapons/rocket launcher/world/v_rocketlauncher_world.prefab"), "rocket.launcher" },
});

namespace rust
{
	struct BoundingBox {
		glm::vec2 position{};
		glm::vec2 size{};
		bool valid{ false };
	};

	struct CachedBone {
		BoneList id;
		uptr transform_ptr;
		i32 index;
		std::vector<i32> hierarchy;
	};

	class Entity
	{
	public:
		Entity() {
			bones = new std::unordered_map<BoneList, BoneData>();
			original_materials = new std::unordered_map<uptr, std::vector<u32>>();
			frame_matrix_cache = new std::unordered_map<i32, BoneMatrix3x4>();
		}
		Entity(uptr address, EntityPrefabs prefab_id = EntityPrefabs::invalid, EntityType type = EntityType::None)
			: base_address(address), prefab_id(prefab_id), type(type) {
			bones = new std::unordered_map<BoneList, BoneData>();
			original_materials = new std::unordered_map<uptr, std::vector<u32>>();
			frame_matrix_cache = new std::unordered_map<i32, BoneMatrix3x4>();
		}

		Entity(const Entity&) = delete;
		Entity& operator=(const Entity&) = delete;

		~Entity() {
			delete bones;
			delete original_materials;
			delete frame_matrix_cache;
		}

		uptr base_address{};
		EntityPrefabs prefab_id{ EntityPrefabs::invalid };
		EntityType type{ EntityType::None };

		gui::Animation alpha_anim{ 1.0f };
		gui::Animation inventory_anim{ 0.0f };

		i32 flags{};

		f32 distance{};
		std::string name{};
		std::string prefab_name{};
		glm::vec3 origin{ 0.f, 0.f, 0.f };
		bool has_origin{ false };
		bool static_origin_cached{ false };
		glm::vec3 static_origin{ 0.f, 0.f, 0.f };

		glm::vec3 velocity{ 0.f, 0.f, 0.f };
		glm::vec3 prev_origin{ 0.f, 0.f, 0.f };
		std::chrono::steady_clock::time_point last_velocity_update{};
		bool has_velocity{ false };
		f32 health{};
		f32 max_health{};
		u64 team{};
		u64 steam_id{};

		enum class AvatarStatus {
			None,
			Pending,
			Downloading,
			Ready,
			Failed
		};

		AvatarStatus avatar_status{ AvatarStatus::None };
		texture_t avatar_texture{};

		u8 life_state{ 0 };
		i32 not_alive_ticks{ 0 };

		i32 missed_ticks{ 0 };

		bool is_sleeping{ false };
		bool is_wounded{ false };
		bool is_spectating{ false };
		bool is_connected{ false };
		bool is_incapacitated{ false };
		bool is_aiming{ false };
		bool is_crouching{ false };
		bool is_prone{ false };
		bool is_scoped{ false };

		std::unordered_map<BoneList, BoneData>* bones{};

		std::vector<CachedBone> cached_bones{};
		uptr cached_matrix_base{ 0 };
		uptr cached_model_ptr{ 0 };
		uptr cached_transforms_ptr{ 0 };
		uptr cached_shared_transform_data{ 0 };
		uptr cached_tr_data_off{ 0x28 };
		uptr cached_tr_index_off{ 0x30 };
		i32 cb_max_index{ 0 };
		bool bones_initialized{ false };
		std::chrono::steady_clock::time_point last_bone_update{};
		std::chrono::steady_clock::time_point all_reject_start{};
		i32 bone_read_failures{ 0 };
		i32 stale_cache_ticks{ 0 };
		i32 partial_fail_ticks{ 0 };
		i32 transient_fail_ticks{ 0 };

		void cache_bones();

		std::unordered_map<BoneList, BoneData> snapshot_visible_bones() const {
			std::unordered_map<BoneList, BoneData> out;
			if (!bones) return out;
			out.reserve(bones->size());
			for (const auto& [id, bd] : *bones) {
				if (bd.visible) out.emplace(id, bd);
			}
			return out;
		}

		std::optional<BoneData> get_visible_bone(BoneList id) const {
			if (!bones) return std::nullopt;
			auto it = bones->find(id);
			if (it == bones->end() || !it->second.visible) return std::nullopt;
			return it->second;
		}

		struct BoneSnap {
			static constexpr int kMax = 128;
			glm::vec3 pos[kMax]{};
			bool      has[kMax]{};

			__forceinline bool             valid(BoneList id) const { return has[static_cast<int>(id)]; }
			__forceinline const glm::vec3& at(BoneList id)    const { return pos[static_cast<int>(id)]; }
		};

		void snapshot_bones(BoneSnap& out) const {
			std::memset(out.has, 0, sizeof(out.has));
			if (!bones) return;
			for (const auto& [id, bd] : *bones) {
				const int idx = static_cast<int>(id);
				if (idx < 0 || idx >= BoneSnap::kMax) continue;
				if (!bd.visible) continue;
				out.pos[idx] = bd.position;
				out.has[idx] = true;
			}
		}

		BoundingBox bounding_box{};
		std::unordered_map<uptr, std::vector<u32>>* original_materials{};
		std::unordered_map<i32, BoneMatrix3x4>* frame_matrix_cache{};
		std::chrono::steady_clock::time_point last_material_update{};
		bool was_chams_enabled{ false };

		uptr player_input{};
		glm::vec3 view_angles{};

		uptr held_items[6]{};
		i32 held_items_count{ 0 };
		uptr cached_inventory{ 0 };
		std::chrono::steady_clock::time_point last_inventory_update{};
		struct BeltItem {
			std::string shortname;
			texture_t* texture;
			int amount;
			bool is_active;
			uptr item_ptr;
		};
		std::vector<BeltItem> belt_items{};
		uptr active_item{};
		std::string item_name{};
		std::string item_shortname{};
		texture_t* item_texture{ nullptr };

		uptr get_container_contents(uptr container_offset);
		void get_belt_items();
		uptr get_held_item();

		bool is_alive() { return life_state == 0 || life_state > 1; }
	};
}