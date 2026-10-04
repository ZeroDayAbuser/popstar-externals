#pragma once
#include "globals.hpp"

namespace offsets {

	namespace il2cpp {
		// Il2CppHandleTable — descriptor = GA + base + type * 0x28
		inline constexpr uptr get_handle = 0xFE00000;
		inline constexpr uptr gchandle_stride = 0x28;
		inline constexpr uptr gchandle_get_target = 0x86D600;
	}

	namespace gc_handles {
		inline constexpr uptr array_rva = il2cpp::get_handle;
	}

	namespace klass_rvas {
		inline constexpr uptr BaseNetworkable = 0x10959AB0; // 25354344
		inline constexpr uptr BasePlayer = 0x10959AA0;
		inline constexpr uptr MainCamera = 0x109B7980;
		inline constexpr uptr PlayerEyes = 0x10958820;
		inline constexpr uptr Item = 0x1095EAE0;
		inline constexpr uptr TOD_Sky = 0x10A089B8;
	}

	namespace base_networkable {
		inline constexpr uptr base_networkable = klass_rvas::BaseNetworkable;
		inline constexpr uptr static_fields = 0xb8;
		inline constexpr uptr wrapper_class_ptr = 0x8;     // client_entities
		inline constexpr uptr parent_static_fields = 0x18; // parent
		inline constexpr uptr entities = 0x20;
		inline constexpr uptr entity_hidden = 0x18;
		inline constexpr uptr hv_offset = 0x18;
		inline constexpr uptr hv_has_value = 0x10;
		inline constexpr uptr buffer_list_array = 0x10;
		inline constexpr uptr buffer_list_size = 0x18;
		inline constexpr uptr il2cpp_array_data_base = 0x20;
		inline constexpr uptr list_dictionary_vals = 0x20; // 05/09/26 LIVE-VERIFIED
		inline constexpr uptr entListBase = buffer_list_array;
		inline constexpr uptr entSize = buffer_list_size;
		inline constexpr uptr prefabID = 0x54;             // BaseNetworkable::prefabID
		inline constexpr uptr prefab_id = 0x54;
		inline constexpr uptr netId = 0x24;                // EntityId int
		inline constexpr uptr parentEntity = 0x38;
		inline constexpr uptr parent_entity = 0x38;
		inline constexpr uptr children = 0x88;
		inline constexpr uptr net = 0x28;
	}

	namespace camera {
		inline constexpr uptr main_camera_c = klass_rvas::MainCamera;
		inline constexpr uptr camera_static = 0xb8;
		inline constexpr uptr camera_object = 0xB0;
		inline constexpr uptr main_camera_transform = 0x30;
		inline constexpr uptr entity = 0x10;
		inline constexpr uptr view_matrix = 0x2fc;
		inline constexpr uptr position = 0x444;
		inline constexpr uptr field_of_view = 0x170;
		inline constexpr uptr culling_mask = 0x43C;
		inline constexpr uptr aspect = 0x4E0;
		inline constexpr uptr near_clip = 0x430;
		inline constexpr uptr far_clip = 0x458;
	}

	namespace base_player {
		inline constexpr uptr display_name = 0x4C8;
		inline constexpr uptr player_rigidbody = 0x6F0;
		inline constexpr uptr player_input = 0x6A0;
		inline constexpr uptr player_inventory = 0x4F0;
		inline constexpr uptr base_movement = 0x518;
		inline constexpr uptr current_team = 0x558;
		inline constexpr uptr cl_active_item = 0x588;
		inline constexpr uptr player_eyes = 0x3C0;
		inline constexpr uptr player_flags = 0x6D8;
		inline constexpr uptr player_model = 0x2F0;
		inline constexpr uptr camera_mode = 0x35C;
		inline constexpr uptr userID = 0x3A0;
		inline constexpr uptr userID_plain = 0x3A0;
		inline constexpr uptr currentGesture = 0x3B0;
		inline constexpr uptr frozen = 0x398;
		inline constexpr uptr held_entity = 0x6F8;
		inline constexpr uptr waterBody = 0x3b0;
		inline constexpr uptr modelState = 0x6E8;
		inline constexpr uptr lastSentTickTime = 0x698;
		inline constexpr uptr lastSentTick = 0x3C0;
		inline constexpr uptr mounted = 0x5E0;
		inline constexpr uptr belt = 0x678;
		inline constexpr uptr _lookingAtEntity = 0x318;
		inline constexpr uptr metabolism = 0x5A0;
		inline constexpr uptr blueprints = 0x7A0;
		inline constexpr uptr modifiers = 0x6A8;             // 05/09/26
		inline constexpr uptr voiceSpeaker = 0x560;          // 05/09/26
		inline constexpr uptr collision = 0x348;             // 05/09/26
		inline constexpr uptr protection = 0x4C0;            // 05/09/26
		inline constexpr uptr velocity = 0x204;              // Vector3 direct
		inline constexpr uptr clothing_blocks_aiming = 0x7BC;
		inline constexpr uptr clothing_move_speed_reduction = 0x7C0;
		inline constexpr uptr clothing_water_speed_bonus = 0x7bc;
		inline constexpr uptr clothing_accuracy_bonus = 0x7c0;
		inline constexpr uptr equipping_blocked = 0x7c4;
		inline constexpr uptr player_loot = 0x48;
		inline constexpr uptr Menu_AssistPlayer_rva = 0x2DD7430;
		inline constexpr uptr Menu_AssistPlayer_rva_c0 = 0x0;
		inline constexpr uptr Menu_AssistPlayer_rva_c1 = 0x0;
		inline constexpr uptr Menu_AssistPlayer_rva_c2 = 0x0;
		inline constexpr uptr Menu_AssistPlayer_rva_c3 = 0x0;
		inline constexpr uptr Menu_AssistPlayer_rva_count = 0x0;
	}

	namespace base_combat_entity {
		inline constexpr uptr skeletonProperties = 0x230;
		inline constexpr uptr lifestate = 0x2a8;
		inline constexpr uptr model = 0x1b8;
		inline constexpr uptr _health = 0x2b4;
		inline constexpr uptr _maxHealth = 0x2b8;
		inline constexpr uptr mark_attacker_hostile = 0x2ae;
	}

	namespace base_entity {
		inline constexpr uptr bounds = 0x18c;
		inline constexpr uptr model = 0x1b8;
		inline constexpr uptr flags = 0x1C0;
		inline constexpr uptr triggers = 0xA8;
		inline constexpr uptr isVisible = 0x150;
		inline constexpr uptr isAnimatorVisible = 0x151;
		inline constexpr uptr isShadowVisible = 0x152;
	}

	namespace item_container {
		inline constexpr uptr item_list = 0x48;
		inline constexpr uptr item_list_alt = 0x58;
		inline constexpr uptr item_count = 0x18;
		inline constexpr uptr flags = 0x58;                  // bit0x4=belt, bit0x2=wear
		inline constexpr uptr capacity = 0x58;
		inline constexpr uptr uid = 0x48;
	}

	namespace player_inventory {
		inline constexpr uptr main = 0x60;
		inline constexpr uptr belt = 0x60;
		inline constexpr uptr wear = 0x58;
		inline constexpr uptr loot = 0x48;
	}

	namespace player_eyes {
		inline constexpr uptr viewOffset = 0x40;
		inline constexpr uptr eye_offset = 0x6C;
		inline constexpr uptr bodyRotation = 0x50;
		inline constexpr uptr headAngles = 0x60;             // 04/08/26 AUTO
		inline constexpr uptr worldPosition = 0x0;
		inline constexpr uptr eyeRotation = 0x0;
		inline constexpr uptr unkQuanternion = 0x0;
	}

	namespace player_input {
		inline constexpr uptr state = 0x28;
		inline constexpr uptr rotation = 0x34;
		inline constexpr uptr bodyAngles = 0x44;
		inline constexpr uptr body_angles = 0x44;
	}

	namespace player_model {
		inline constexpr uptr rootBone = 0x98;
		inline constexpr uptr headBone = 0xf8;
		inline constexpr uptr eyeBone = 0x120;
		inline constexpr uptr boneTransforms = 0x98;
		inline constexpr uptr SkinnedMultiMesh = 0x250;
		inline constexpr uptr position = 0x2f8;
		inline constexpr uptr velocity = 0x31c;
		inline constexpr uptr newVelocity = 0x31c;
		inline constexpr uptr new_velocity = 0x31c;
		inline constexpr uptr rotation = 0x328;
		inline constexpr uptr visible = 0xc4;
		inline constexpr uptr is_local_player = 0xc4;
		inline constexpr uptr isNpc = 0x490;
		inline constexpr uptr is_npc = 0x490;
		inline constexpr uptr heldEntity = 0x398;
		inline constexpr uptr GestureConfig = 0x2c8;
		inline constexpr uptr InGesture = 0x0;
		inline constexpr uptr viewMatrix = 0x304;
		inline constexpr uptr collision = 0xD0;
		inline constexpr uptr fullMask = 0x208;
		inline constexpr uptr jawBone = 0x0;
		inline constexpr uptr neckBone = 0x0;
	}

	namespace model {
		inline constexpr uptr boneTransforms = 0x50;
		inline constexpr uptr boneNames = 0x58;
		inline constexpr uptr bone_array_count_off = 0x18;
		inline constexpr uptr bone_array_data_off = 0x20;
		inline constexpr uptr rootBone = 0x28;
		inline constexpr uptr headBone = 0x30;
		inline constexpr uptr eyeBone = 0x38;
		inline constexpr uptr collision = 0x20;
		inline constexpr uptr animator = 0x40;
		inline constexpr uptr skeleton = 0x48;
	}

	namespace local_player_canonical {
		inline constexpr uptr slot_klass_rva = 0x10957BE0;
		inline constexpr uptr entity_field_off = 0x140;
		inline constexpr uptr is_wrapped = 1;
	}

	namespace local_player {
		inline constexpr uptr type_info = local_player_canonical::slot_klass_rva;
		inline constexpr uptr entity = local_player_canonical::entity_field_off;
		inline constexpr uptr base_player_typeinfo = klass_rvas::BasePlayer;
	}

	namespace item {
		inline constexpr uptr info = 0x40;
		inline constexpr uptr uid = 0xA8;
		inline constexpr uptr item_uid_wrong = 0xB8;

		inline constexpr uptr held_entity = 0x70;
		inline constexpr uptr held_entity_alt = 0xE0;
		inline constexpr uptr amount = 0x68;
		inline constexpr uptr position = 0xEC;
		inline constexpr uptr client_ammo_count = 0xb8;
		inline constexpr uptr world_ent = 0x38;
		inline constexpr uptr contents = 0xa0;
		inline constexpr uptr condition = 0x68;
		inline constexpr uptr max_condition = 0xc4;
		inline constexpr uptr parent = 0x70;
		inline constexpr uptr _condition = 0x0;
		inline constexpr uptr _maxCondition = 0x0;
		inline constexpr uptr health = 0x0;
		inline constexpr uptr maxHealth = 0x0;
	}

	namespace item_definition {
		inline constexpr uptr itemid = 0x20;
		inline constexpr uptr short_name = 0x28;
		inline constexpr uptr display_name = 0x40;
		inline constexpr uptr icon_sprite = 0x50;
		inline constexpr uptr category = 0x58;
		inline constexpr uptr stackable = 0x78;
		inline constexpr uptr rarity = 0x94;
		inline constexpr uptr condition = 0xb8;
		inline constexpr uptr itemModWearable = 0x1B0;
	}

	namespace held_entity {
		inline constexpr uptr view_model = 0x200;
		inline constexpr uptr worldModelAnimator = 0x218;
		inline constexpr uptr item_owner = 0x250;
		inline constexpr uptr handBone = 0x260;
		inline constexpr uptr holdInfo = 0x270;
		inline constexpr uptr owner_item_uid = 0x2e0;
	}

	namespace base_projectile {
		inline constexpr uptr gravityModifier = 0x38;
		inline constexpr uptr gravity_modifier = 0x38;
		inline constexpr uptr drag = 0x34;
		inline constexpr uptr projectileVelScale = 0x394;
		inline constexpr uptr velocity_scale = 0x394;
		inline constexpr uptr automatic = 0x398;
		inline constexpr uptr reloadTime = 0x3D8;
		inline constexpr uptr reload_time = 0x3D8;
		inline constexpr uptr isReloading = 0x484;
		inline constexpr uptr is_reloading = 0x484;
		inline constexpr uptr primaryMagazine = 0x3E0;
		inline constexpr uptr magazine = 0x3E0;
		inline constexpr uptr aimSway = 0x400;
		inline constexpr uptr aim_sway = 0x400;
		inline constexpr uptr aimSwaySpeed = 0x404;
		inline constexpr uptr aim_sway_speed = 0x404;
		inline constexpr uptr recoilProp = 0x408;
		inline constexpr uptr recoil = 0x408;
		inline constexpr uptr aimCone = 0x418;
		inline constexpr uptr aim_cone = 0x418;
		inline constexpr uptr hipAimCone = 0x41C;
		inline constexpr uptr aimconePenaltyPerShot = 0x420;
		inline constexpr uptr aimcone_penalty_per_shot = 0x420;
		inline constexpr uptr aimConePenaltyMax = 0x424;
		inline constexpr uptr aimcone_penalty_max = 0x424;
		inline constexpr uptr aimconePenaltyRecoverTime = 0x428;
		inline constexpr uptr aimconePenaltyRecoverDelay = 0x42C;
		inline constexpr uptr stancePenaltyScale = 0x430;
		inline constexpr uptr stance_penalty_scale = 0x430;
		inline constexpr uptr isBurstWeapon = 0x43F;
		inline constexpr uptr is_burst_weapon = 0x43F;
		inline constexpr uptr numShotsFired = 0x43c;
		inline constexpr uptr internalBurstAimConeScale = 0x44C;
		inline constexpr uptr internal_burst_aim_cone_scale = 0x44C;
		inline constexpr uptr sightAimConeScale = 0x474;
		inline constexpr uptr sight_aim_cone_scale = 0x474;
		inline constexpr uptr hipAimConeScale = 0x47C;
		inline constexpr uptr hip_aim_cone_scale = 0x47C;
		inline constexpr uptr viewmodel_instance = 0x1d8;
		inline constexpr uptr damageScale = 0x374;
		inline constexpr uptr MuzzlePoint = 0x3b8;
		inline constexpr uptr aimconeCurve = 0x410;
		inline constexpr uptr sightAimConeOffset = 0x478;
		inline constexpr uptr hipAimConeOffset = 0x480;
		inline constexpr uptr camera_punches = 0x0;
	}

	namespace projectile_list {
		inline constexpr uptr projectile_klass_rva = 0x10985298;
		inline constexpr uptr klass_static_fields = 0xb8;
		inline constexpr uptr instance_in_static = 0x20;
		inline constexpr uptr vals_in_hashset = 0x10;
		inline constexpr uptr count_in_hashset = 0x18;
	}

	namespace list_component_projectile {
		inline constexpr uptr list_component_c = 0x10985298;
		inline constexpr uptr static_fields = 0xb8;
		inline constexpr uptr instance = 0x20;
		inline constexpr uptr buffer = 0x10;
		inline constexpr uptr values_array = 0x10;
		inline constexpr uptr count = 0x18;
		inline constexpr uptr vals_in_hashset = 0x10;
		inline constexpr uptr parent_static = 0x10;
	}

	namespace base_player_static {
		inline constexpr uptr typeinfo = klass_rvas::BasePlayer;
		inline constexpr uptr static_fields = 0xb8;
		inline constexpr uptr visiblePlayerList = 0x158;
	}

	namespace base_player_class {
		inline constexpr uptr typeinfo = klass_rvas::BasePlayer;
	}

	namespace local_player_static {
		inline constexpr uptr typeinfo = local_player_canonical::slot_klass_rva;
		inline constexpr uptr static_fields = 0xb8;
	}

	namespace tod_sky {
		// Chain 05/09/26: { 0xB8, 0x20, 0x10, 0x20 }
		inline constexpr uptr tod_sky_c = klass_rvas::TOD_Sky;
		inline constexpr uptr static_fields = 0xb8;
		inline constexpr uptr instance = 0xD8;
		inline constexpr uptr instances = 0xD8;
		inline constexpr uptr list_items = 0x10;
		inline constexpr uptr list_size = 0x18;
		inline constexpr uptr array_items = 0x20;
		inline constexpr uptr cycle = 0x40;
		inline constexpr uptr atmosphere = 0x50;
		inline constexpr uptr day = 0x58;
		inline constexpr uptr night = 0x60;
		inline constexpr uptr stars = 0x78;
		inline constexpr uptr clouds = 0x80;
		inline constexpr uptr ambient = 0x98;
		inline constexpr uptr dayAmbient = 0x40;
		inline constexpr uptr nightAmbient = 0x48;
		inline constexpr uptr world = 0x48;
		inline constexpr uptr sun = 0x68;
		inline constexpr uptr moon = 0x70;
		inline constexpr uptr light = 0x88;
		inline constexpr uptr fog = 0x90;
		inline constexpr uptr reflection = 0xa0;
	}

	namespace hackable_locked_crate {
		inline constexpr uptr timerText = 0x400;
		inline constexpr uptr hackSeconds = 0x410;
	}

	namespace bn_chain {
		inline constexpr uptr base_networkable = klass_rvas::BaseNetworkable;
		inline constexpr uptr static_fields = 0xb8;
		inline constexpr uptr wrapper_class_ptr = 0x8;
		inline constexpr uptr parent_static_fields = 0x18;
		inline constexpr uptr entities = 0x20;
	}

	namespace growable_entity {
		inline constexpr uptr state = 0x358;
	}

	namespace collectible_entity {
		inline constexpr uptr item_list = 0x208;
	}

	namespace dropped_item {
		inline constexpr uptr world_item = 0x208;
	}

	namespace chams {
		inline constexpr uptr multi_mesh = 0x250;
		inline constexpr uptr renderers = 0x58;
	}

	namespace game_manager {
		inline constexpr uptr static_class = 0x10951450;
		inline constexpr uptr static_fields = 0xb8;
		inline constexpr uptr client = 0x8;
		inline constexpr uptr pool = 0x10;
	}

	namespace prefab_pool_collection {
		inline constexpr uptr storage = 0x10;
	}

	namespace prefab_pool {
		inline constexpr uptr stack = 0x10;
	}

	namespace rpc {
		inline constexpr uptr camera_update_hook = 0x11a11be0;
		inline constexpr uptr camera_update_hook_action = 0x20;
		inline constexpr uptr convar_server = 0xE26BCC0;
		inline constexpr uptr received_data_from_server = 0x27D3800;
		inline constexpr uptr server_rpc = 0x604ee20;
		inline constexpr uptr create_prefab = 0x758c410;
		inline constexpr uptr on_view_mode_changed = 0x6b14340;
		inline constexpr uptr behaviour_set_enabled = 0xe77ec70;
		inline constexpr uptr camera_man = 0xFC570A8;
		inline constexpr uptr camera_speed = 0x24;
		inline constexpr uptr singleton_instance = 0x8;
		inline constexpr uptr loading_screen = 0x0;
		inline constexpr uptr loading_screen_panel = 0x30;
		inline constexpr uptr mixer_snapshot_manager = 0x0;
		inline constexpr uptr mixer_default_snapshot = 0x20;
		inline constexpr uptr mixer_loading_snapshot = 0x30;
		inline constexpr uptr gameobject_is_active = 0x56;
		inline constexpr uptr item_icon = 0x10A4FEF8;
		inline constexpr uptr container_loot_start_times = 0x0;
		inline constexpr uptr menu_assist_player = 0x2DD7430;
		inline constexpr uptr item_command = 0x3132670;
		inline constexpr uptr looking_at_entity = 0x320;
		inline constexpr uptr player_loot = 0x48;
		inline constexpr uptr loot_containers = 0x38;
	}

	namespace physx {
		inline constexpr uptr sdk_offset = 0x217F8E8;
		inline constexpr uptr physx_sdk = 0x217F8E8;
	}

	namespace base_movement {
		inline constexpr uptr adminCheat = 0x20;
		inline constexpr uptr adminSpeed = 0x0;
		inline constexpr uptr Owner = 0x30;
		inline constexpr uptr Ducking = 0x0;
		inline constexpr uptr Crawling = 0x0;
		inline constexpr uptr Running = 0x0;
		inline constexpr uptr Grounded = 0x1C4;
		inline constexpr uptr gravityMultiplier = 0xB0;
		inline constexpr uptr maxVelocity = 0x100;
		inline constexpr uptr climbing = 0x1CC;
		inline constexpr uptr flying = 0x20C;
		inline constexpr uptr grounded = 0x1C4;
		inline constexpr uptr swimming = 0x1DC;
		inline constexpr uptr falling = 0x1FC;
		inline constexpr uptr wasFalling = 0x204;
		inline constexpr uptr wasFlying = 0x214;
		inline constexpr uptr InheritedVelocity = 0x30;
		inline constexpr uptr TargetMovement = 0x130;
		inline constexpr uptr target_movement = 0x44;
		inline constexpr uptr velocity = 0x130;
		inline constexpr uptr groundAngle = 0x108;
		inline constexpr uptr ground_angles = 0x108;
		inline constexpr uptr groundAngleNew = 0x110;
		inline constexpr uptr ground_angles_new = 0x110;
		inline constexpr uptr sprintForced = 0x1B0;
	}

	namespace model_state {
		inline constexpr uptr flags = 0x5C;
		inline constexpr uptr waterLevel = 0x70;
		inline constexpr uptr lookDir = 0x3C;

		inline constexpr int Ducked    = 1;
		inline constexpr int Jumped    = 2;
		inline constexpr int OnGround  = 4;
		inline constexpr int Sleeping  = 8;
		inline constexpr int Sprinting = 16;
		inline constexpr int OnLadder  = 32;
		inline constexpr int Flying    = 64;
		inline constexpr int Aiming    = 128;
		inline constexpr int Prone     = 256;
		inline constexpr int Mounted   = 512;
		inline constexpr int Crawling  = 4096;
	}

	namespace recoil_properties {
		inline constexpr uptr recoilYawMin = 0x18;
		inline constexpr uptr recoilYawMax = 0x1c;
		inline constexpr uptr recoilPitchMin = 0x20;
		inline constexpr uptr recoilPitchMax = 0x24;
		inline constexpr uptr clampPitch = 0x38;
		inline constexpr uptr pitchCurve = 0x40;
		inline constexpr uptr yawCurve = 0x48;
		inline constexpr uptr useCurves = 0x50;
		inline constexpr uptr overrideAimconeWithCurve = 0x5c;
		inline constexpr uptr newRecoilOverride = 0x80;
		inline constexpr uptr new_recoil = 0x80;
	}

	namespace bow_weapon {
		inline constexpr uptr attackReady = 0x4d8;
		inline constexpr uptr wasAiming = 0x4e0;
	}

	namespace compound_bow_weapon {
		inline constexpr uptr holdProgress = 0x4E8;
		inline constexpr uptr stringHoldDurationMax = 0x4E8;
		inline constexpr uptr stringBonusVelocity = 0x530;
		inline constexpr uptr movement_penalty = 0x530;
	}

	namespace skinned_multi_mesh {
		inline constexpr uptr renderer_list = 0x58;
		inline constexpr uptr renderers = 0x58;
		inline constexpr uptr renderer_material_array = 0x140;
	}

	namespace tod_cycle_parameters {
		inline constexpr uptr hour = 0x10;
	}

	namespace tod_night_parameters {
		inline constexpr uptr moon_color = 0x10;
		inline constexpr uptr cloud_color = 0x38;
		inline constexpr uptr ambient_color = 0x48;
		inline constexpr uptr ambient_multiplier = 0x5c;
	}

	namespace tod_ambient_parameters {
		inline constexpr uptr saturation = 0x14;
	}

	namespace tod_atmosphere_parameters {
		inline constexpr uptr rayleigh_multiplier = 0x10;
	}

	namespace tod_day_parameters {
		inline constexpr uptr sky_color = 0x28;
	}

	namespace tod_star_parameters {
		inline constexpr uptr size = 0x10;
		inline constexpr uptr brightness = 0x14;
	}

	namespace tod_cloud_parameters {
		inline constexpr uptr opacity = 0x14;
		inline constexpr uptr coverage = 0x18;
		inline constexpr uptr brightness = 0x30;
	}

	namespace unity_object {
		inline constexpr uptr cached_ptr = 0x10;
	}

	namespace unity_component {
		inline constexpr uptr game_object = 0x20;
	}

	namespace unity_game_object {
		inline constexpr uptr components = 0x20;
		inline constexpr uptr component_count = 0x30;
		inline constexpr uptr layer = 0x40;
		inline constexpr uptr tag = 0x44;
		inline constexpr uptr component_stride = 0x10;
		inline constexpr uptr component_ptr_in_entry = 0x8;
	}

	namespace unity_transform {
		inline constexpr uptr indirect_ptr_off = 0x28;       // tf → TransformData* (09/07/26)
		inline constexpr uptr transform_access = 0x28;
		inline constexpr uptr transform_index = 0x30;
		inline constexpr uptr world_pos_off = 0x90;          // td → Vector3

		inline constexpr uptr transform_data_ptr = 0x28;     // was 0x38 — ObjClass bug fixed
		inline constexpr uptr transform_highest_index = 0x40;
		inline constexpr uptr transform_data_offset = 0x18;
		inline constexpr uptr bone_list = 0x20;
		inline constexpr uptr bone_transform = 0x10;
		inline constexpr uptr parent_off = 0x0;
		inline constexpr uptr local_pos_off = 0x0;
		inline constexpr uptr world_rot_off = 0x0;
	}

	namespace unity_transform_hierarchy {
		inline constexpr uptr positions = 0x18;
		inline constexpr uptr parent_indices = 0x20;
	}

	namespace player_tick {
	}

	namespace player_walk_movement {
		inline constexpr uptr capsule = 0xF8;
		inline constexpr uptr ladder = 0xE0;
		inline constexpr uptr modify = 0x1C0;
	}

	namespace input_state {
	}

	namespace input_message {
		inline constexpr uptr buttons = 0x2c;
		inline constexpr uptr mouseDelta = 0x20;
		inline constexpr uptr aimAngles = 0x14;
	}

	namespace buttons_static {

		inline constexpr uptr Sprint = 0x238;
	}

	namespace buttons_con_button {
		inline constexpr uptr IsDown = 0x14;
	}

	namespace convar_graphics {
		inline constexpr uptr typeinfo = 0x10951438;
		inline constexpr uptr static_fields = 0xb8;
		inline constexpr uptr fov = 0x648;
	}

	inline constexpr uptr StaticFieldsBase = 0xb8;

	namespace magazine {
		inline constexpr uptr definition = 0x10;
		inline constexpr uptr Capacity = 0x18;
		inline constexpr uptr Contents = 0x1c;
		inline constexpr uptr ammoType = 0x20;
	}

	namespace item_mod_projectile {
		inline constexpr uptr projectileObject = 0x20;
		inline constexpr uptr ammoType = 0x30;
		inline constexpr uptr projectileSpread = 0x3c;
		inline constexpr uptr projectileVelocity = 0x40;
		inline constexpr uptr projectileVelocitySpread = 0x44;
		inline constexpr uptr useCurve = 0x48;
		inline constexpr uptr spreadScalar = 0x50;
		inline constexpr uptr category = 0x68;
	}

	namespace attack_entity {
		inline constexpr uptr deployDelay = 0x2d8;
		inline constexpr uptr repeatDelay = 0x2EC;
		inline constexpr uptr animationDelay = 0x2e0;
		inline constexpr uptr nextAttackTime = 0x340;
		inline constexpr uptr timeSinceDeploy = 0x358;
	}

	namespace base_melee {
		inline constexpr uptr maxDistance = 0x3A0;
		inline constexpr uptr attackRadius = 0x3A4;
	}

	namespace flint_strike_weapon {
		inline constexpr uptr successFraction = 0x4C0;
		inline constexpr uptr success_fraction = 0x4C0;
		inline constexpr uptr strikes = 0x4d4;
		inline constexpr uptr strikeRecoil = 0x4C8;
		inline constexpr uptr offset2 = 0x4D0;
		inline constexpr uptr didSparkThisFrame = 0x4D0;
		inline constexpr uptr _did_spark_this_frame = 0x4D0;
	}

	namespace base_view_model {
		inline constexpr uptr BaseViewModel_C = 0xFC00EA0;
		inline constexpr uptr animationEvents = 0xd8;
		inline constexpr uptr list = 0x78;
		inline constexpr uptr MuzzlePoint = 0x58;
		inline constexpr uptr viewmodelType = 0x50;
		inline constexpr uptr baseSkinPieces = 0x78;
		inline constexpr uptr model = 0x90;
		inline constexpr uptr lower = 0xD8;
		inline constexpr uptr viewmodel_lower = 0xD8;
		inline constexpr uptr viewmodel_sway = 0x130;
		inline constexpr uptr viewmodel_bob = 0xB8;
		inline constexpr uptr ironSights = 0xc8;
		inline constexpr uptr useViewModelCamera = 0x40;
		inline constexpr uptr animator = 0x130;
	}

	namespace view_model {
		inline constexpr uptr instance = 0x28;
		inline constexpr uptr baseViewModel = 0x28;
	}

	namespace viewmodel_lower {
		inline constexpr uptr lowerOnSprint = 0x20;
		inline constexpr uptr lowerWhenCantAttack = 0x21;
		inline constexpr uptr shouldLower = 0x28;
		inline constexpr uptr rotateAngle = 0x2c;
	}

	namespace hit_test {
		inline constexpr uptr type = 0x94;
		inline constexpr uptr didhit = 0x20;
		inline constexpr uptr hit_point = 0x60;
		inline constexpr uptr hitentity = 0xC8;
		inline constexpr uptr hit_material = 0x28;
		inline constexpr uptr hitpart = 0x10;
		inline constexpr uptr attack_ray = 0x9C;
		inline constexpr uptr max_distance = 0x98;
		inline constexpr uptr ray_hit = 0x30;
		inline constexpr uptr game_object = 0xC0;
		inline constexpr uptr collider = 0xD0;
		inline constexpr uptr ignore_entity = 0x78;
		inline constexpr uptr hit_transform = 0x70;
		inline constexpr uptr hit_normal = 0x14;
		inline constexpr uptr hittransform = 0x70;
		inline constexpr uptr damage_properties = 0xE0;
	}

	namespace projectile {
		inline constexpr uptr initialVelocity = 0x28;
		inline constexpr uptr drag = 0x34;
		inline constexpr uptr gravityModifier = 0x38;
		inline constexpr uptr currentThickness = 0x3c;
		inline constexpr uptr thickness = 0x3c;
		inline constexpr uptr initialDistance = 0x44;
		inline constexpr uptr initialOrientation = 0x48;
		inline constexpr uptr remainInWorld = 0x54;
		inline constexpr uptr stickProbability = 0x58;
		inline constexpr uptr breakProbability = 0x5c;
		inline constexpr uptr conditionLoss = 0x60;
		inline constexpr uptr ricochetChance = 0x64;
		inline constexpr uptr penetrationPower = 0x68;
		inline constexpr uptr waterIntegrityLoss = 0x70;
		inline constexpr uptr damageDistances = 0x80;
		inline constexpr uptr damageMultipliers = 0x88;
		inline constexpr uptr damageTypes = 0x90;
		inline constexpr uptr clientEffectPrefab = 0xe8;
		inline constexpr uptr swimScale = 0xf0;
		inline constexpr uptr swimSpeed = 0xfc;
		inline constexpr uptr mod = 0x118;
		inline constexpr uptr ownerPlayer = 0x1F0;
		inline constexpr uptr positionDirect = 0x0;
		inline constexpr uptr sourceWeapon = 0x108;
		inline constexpr uptr sourceProjectilePrefab = 0x108;
		inline constexpr uptr currentVelocity = 0x170;
		inline constexpr uptr currentPosition = 0x164;
		inline constexpr uptr sentPosition = 0x188;
		inline constexpr uptr previousPosition = 0x194;
		inline constexpr uptr previousVelocity = 0x1A0;
		inline constexpr uptr sentRay = 0x1c0;
		inline constexpr uptr hitTest = 0x110;
		inline constexpr uptr sourceProjectile = 0x1e8;
		inline constexpr uptr projectile_id = 0x130;
		inline constexpr uptr launch_time = 0x140;
		inline constexpr uptr max_distance = 0x1AC;
		inline constexpr uptr traveled_distance = 0x17c;
		inline constexpr uptr traveled_time = 0x180;
		inline constexpr uptr integrity = 0x13c;
	}

	namespace skeleton_properties {
		inline constexpr uptr quickLookup = 0x28;
		inline constexpr uptr boneReference = 0x18;
		inline constexpr uptr bones = 0x20;
	}

	namespace list_component_player_model {
		inline constexpr uptr list_component_c = 0xFC74CC8;
		inline constexpr uptr static_fields = 0xb8;
		inline constexpr uptr instance = 0x8;
		inline constexpr uptr parent_static = 0x10;
		inline constexpr uptr buffer = 0x10;
		inline constexpr uptr count = 0x18;
	}

	namespace camera_update_hook_static {
		inline constexpr uptr typeinfo = 0x11a11be0;
		inline constexpr uptr static_fields = 0xb8;
		inline constexpr uptr action = 0x20;
	}

	namespace convar_player_static {
	}

	namespace convar_server_static {
		inline constexpr uptr typeinfo = 0xE26BCC0;
		inline constexpr uptr static_fields = 0xb8;
	}

	namespace string_pool {
		inline constexpr uptr typeinfo = 0x119b9500;
		inline constexpr uptr static_fields = 0xb8;
		inline constexpr uptr static_fields_slot = 0x0;
		inline constexpr uptr sf_runtime = 0x22B87042340;
		inline constexpr uptr toNumber = 0x38;
	}

	namespace progress_bar {
		inline constexpr uptr typeinfo = 0x11a3bdb0;
		inline constexpr uptr static_fields = 0xb8;
		inline constexpr uptr instance = 0x8;
		inline constexpr uptr timeCounter = 0x34;
	}

	namespace item_icon {
		inline constexpr uptr typeinfo = 0x10A4FEF8;
		inline constexpr uptr static_fields = 0xb8;
		inline constexpr uptr containerLootStartTimes = 0x8;
		inline constexpr uptr dict_pointer = 0x18;
		inline constexpr uptr dict_count = 0x18;
		inline constexpr uptr dict_data = 0x20;
		inline constexpr uptr dict_stride = 0x18;
		inline constexpr uptr entry_timer = 0x10;
		inline constexpr uptr item = 0x198;
	}

	namespace flash_bang_overlay {
		inline constexpr uptr typeinfo = 0x119b1d18;
		inline constexpr uptr instance = 0x8;
		inline constexpr uptr flashLength = 0x58;
	}

	namespace server_admin_ugc_entry {
		inline constexpr uptr ReceivedDataFromServer_rva = 0x17CC5D0;
	}

	namespace steam_client_wrapper {
		inline constexpr uptr singleton_typeinfo = 0x1157F2C0;
		inline constexpr uptr instance = 0x8;
		inline constexpr uptr default_avatar = 0x20;
		inline constexpr uptr get_avatar_texture = 0x1A79E00;
	}

	namespace texture2d {
		inline constexpr uptr get_native_texture_ptr = 0xD931C50;
	}

	namespace game_physics {
		inline constexpr uptr LineOfSightInternal = 0x70d6b90;
		inline constexpr uptr trace = 0x70a11b0;
		inline constexpr uptr trace_all_unordered = 0x70b8050;
		inline constexpr uptr verify = 0x70b5a70;
	}

	namespace BasePlayer               = base_player;
	namespace BaseCombatEntity         = base_combat_entity;
	namespace BaseEntity               = base_entity;
	namespace BaseMovement             = base_movement;
	namespace BaseProjectile           = base_projectile;
	namespace BaseNetworkable          = base_networkable;
	namespace BaseMelee                = base_melee;
	namespace BaseViewModel            = base_view_model;
	namespace BasePlayer_Static        = base_player_static;
	namespace BasePlayer_Class         = base_player_class;
	namespace Item                     = item;
	namespace ItemDefinition           = item_definition;
	namespace ItemContainer            = item_container;
	namespace ItemModProjectile        = item_mod_projectile;
	namespace ItemIcon                 = item_icon;
	namespace PlayerInventory          = player_inventory;
	namespace PlayerModel              = player_model;
	namespace PlayerInput              = player_input;
	namespace PlayerEyes               = player_eyes;
	namespace HeldEntity               = held_entity;
	namespace Magazine                 = magazine;
	namespace ModelState               = model_state;
	namespace Model                    = model;
	namespace UnityObject              = unity_object;
	namespace UnityComponent           = unity_component;
	namespace UnityGameObject          = unity_game_object;
	namespace UnityTransform           = unity_transform;
	namespace SkinnedMultiMesh         = skinned_multi_mesh;
	namespace SkeletonProperties       = skeleton_properties;
	namespace RecoilProperties         = recoil_properties;
	namespace CompoundBowWeapon        = compound_bow_weapon;
	namespace AttackEntity             = attack_entity;
	namespace FlintStrikeWeapon        = flint_strike_weapon;
	namespace ViewmodelLower           = viewmodel_lower;
	namespace ListComponentProjectile  = list_component_projectile;
	namespace ListComponent_Projectile = list_component_projectile;
	namespace ListComponent_PlayerModel = list_component_player_model;
	namespace Projectile               = projectile;
	namespace LocalPlayer              = local_player;
	namespace LocalPlayer_Static       = local_player_static;
	namespace CameraUpdateHook_Static  = camera_update_hook_static;
	namespace ConvarServer_Static      = convar_server_static;
	namespace Buttons_Static           = buttons_static;
	namespace Buttons_ConButton        = buttons_con_button;
	namespace StringPool               = string_pool;
	namespace HackableLockedCrate      = hackable_locked_crate;
	namespace ProgressBar              = progress_bar;
	namespace FlashBangOverlay         = flash_bang_overlay;
	namespace InputMessage             = input_message;
	namespace main_camera              = camera;
	namespace TOD_Sky                  = tod_sky;
	namespace TOD_NightParameters      = tod_night_parameters;
	namespace TOD_AmbientParameters    = tod_ambient_parameters;
	namespace TOD_AtmosphereParameters = tod_atmosphere_parameters;
	namespace TOD_CycleParameters      = tod_cycle_parameters;
	namespace TOD_DayParameters        = tod_day_parameters;
	namespace TOD_StarParameters       = tod_star_parameters;
	namespace TOD_CloudParameters      = tod_cloud_parameters;

	namespace base_networkable {
		inline constexpr uptr typeinfo = klass_rvas::BaseNetworkable;
	}
	namespace base_combat_entity {
		inline constexpr uptr lifeState = lifestate;
	}
	namespace base_player {
		inline constexpr uptr playerModel     = player_model;
		inline constexpr uptr playerFlags     = player_flags;
		inline constexpr uptr playerInput     = player_input;
		inline constexpr uptr playerInventory = player_inventory;
		inline constexpr uptr playerEyes      = player_eyes;
		inline constexpr uptr clActiveItem    = cl_active_item;
		inline constexpr uptr baseMovement    = base_movement;
		inline constexpr uptr currentTeam     = current_team;
		inline constexpr uptr displayName     = display_name;
		inline constexpr uptr username        = display_name;
		inline constexpr uptr team            = current_team;
	}
	namespace camera {
		inline constexpr uptr typeinfo       = main_camera_c;
		inline constexpr uptr instance       = camera_object;
		inline constexpr uptr buffer         = 0;
		inline constexpr uptr static_fields  = camera_static;
	}
	namespace item {
		inline constexpr uptr heldEntity     = held_entity;
		inline constexpr uptr itemDefinition = info;
	}
	namespace item_definition {
		inline constexpr uptr shortName = short_name;
	}
	namespace item_container {
		inline constexpr uptr list      = item_list;
		inline constexpr uptr itemCount = item_count;
	}
	namespace player_inventory {
		inline constexpr uptr containerMain = main;
		inline constexpr uptr containerBelt = belt;
		inline constexpr uptr containerWear = wear;
		inline constexpr uptr container2    = loot;
	}
	namespace held_entity {
		inline constexpr uptr item = owner_item_uid;
	}
	namespace skinned_multi_mesh {
		inline constexpr uptr rendererMaterialArray = renderer_material_array;
		inline constexpr uptr rendererList          = renderer_list;
	}
	namespace tod_sky {
		inline constexpr uptr TOD_Sky_C = tod_sky_c;
		inline constexpr uptr step_1 = static_fields;
		inline constexpr uptr step_2 = instances;
		inline constexpr uptr step_3 = list_items;
		inline constexpr uptr step_4 = array_items;
	}
	namespace local_player_static {
		inline constexpr uptr Entity = local_player::entity;
	}
	namespace list_component_projectile {
		inline constexpr uptr ListComponent_C = list_component_c;
	}

}
