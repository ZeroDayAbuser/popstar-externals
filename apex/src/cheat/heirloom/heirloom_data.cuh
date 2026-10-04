#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace heirloom
{
	struct definition_t
	{
		const char* legend;
		const char* name;
		const char* weapon_class;
		const char* offhand_class;
		const char* view_model_path;
		int skin_index;
		int camo_index;
		int weapon_name_index;
		int view_model_index;
		std::uint32_t melee_guid;
	};

	struct skin_cosmetic_t
	{
		const char* name;
		std::uint32_t guid;
		int skin_index;
		int camo_index;
	};
	inline auto& catalog( )
	{
		static std::vector<definition_t> k_list = {
        // Legend-specific heirlooms
        // { legend, heirloom, weapon_class, offhand_class, view_model_path, skin, camo, weapon_name_index, view_model_index, melee_guid }
        { "Wraith", "Hope's Dusk", "mp_weapon_wraith_kunai_primary", "melee_wraith_kunai",
          "mdl/techart/mshop/weapons/class/heirloom/wraith/v18_kunai/heirloom_wraith_v18_kunai_v.rmdl",
          0, 0, 0, 0 },
        { "Wraith", "Hope's Dawn", "mp_weapon_wraith_kunai_rt01_primary", "melee_wraith_kunai_rt01",
          "mdl/techart/mshop/weapons/class/heirloom/wraith/v18_kunai/heirloom_wraith_v18_kunai_rt01_v.rmdl",
          0, 0, 0, 0 },
        { "Bloodhound", "Raven's Bite", "mp_weapon_bloodhound_axe_primary", "melee_bloodhound_axe",
          "mdl/techart/mshop/weapons/class/heirloom/bloodhound/v19_axe/heirloom_bloodhound_v19_axe_v.rmdl",
          0, 0, 0, 0 },
        { "Bloodhound", "Winter's Bane", "mp_weapon_bloodhound_axe_primary", "melee_bloodhound_axe",
          "mdl/techart/mshop/weapons/class/heirloom/bloodhound/v19_axe/heirloom_bloodhound_v19_axe_v.rmdl",
          0, 0, 0, 0 },
        { "Lifeline", "Shock Sticks", "mp_weapon_lifeline_baton_primary", "melee_lifeline_baton",
          "mdl/techart/mshop/weapons/class/heirloom/lifeline/v19_drumstick/lifeline_v19_drumstick_v.rmdl",
          0, 0, 0, 0 },
        { "Pathfinder", "Boxing Gloves", "mp_weapon_pathfinder_gloves_primary", "melee_pathfinder_gloves",
          "mdl/techart/mshop/weapons/class/heirloom/pathfinder/v19_glove/heirloom_pathfinder_v19_glove_v.rmdl",
          0, 0, 0, 0 },
        { "Pathfinder", "Belva Bruisers", "mp_weapon_pathfinder_gloves_primary", "melee_pathfinder_gloves",
          "mdl/techart/mshop/weapons/class/heirloom/pathfinder/v19_glove/heirloom_pathfinder_v19_glove_v.rmdl",
          0, 0, 0, 0 },
        { "Octane", "Butterfly Knife", "mp_weapon_octane_knife_primary", "melee_octane_knife",
          "mdl/techart/mshop/weapons/class/heirloom/octane/v19_balisong/heirloom_octane_v19_balisong_v.rmdl",
          0, 0, 0, 0 },
        { "Octane", "Octane's Prototype", "mp_weapon_octane_knife_primary", "melee_octane_knife",
          "mdl/techart/mshop/weapons/class/heirloom/octane/v19_balisong/heirloom_octane_v19_balisong_v.rmdl",
          0, 0, 0, 0 },
        { "Caustic", "Death Hammer", "mp_weapon_caustic_hammer_primary", "melee_caustic_hammer",
          "mdl/techart/mshop/weapons/class/heirloom/caustic/v20_sledge/heirloom_caustic_v20_sledge_v.rmdl",
          0, 0, 0, 0 },
        { "Gibraltar", "War Club", "mp_weapon_gibraltar_club_primary", "melee_gibraltar_club",
          "mdl/techart/mshop/weapons/class/heirloom/gibraltar/v20_patu/heirloom_gibraltar_v20_patu_v.rmdl",
          0, 0, 0, 0 },
        { "Mirage", "Too Much Witt", "mp_weapon_mirage_statue_primary", "melee_mirage_statue",
          "mdl/techart/mshop/weapons/class/heirloom/mirage/v20_trophy/heirloom_mirage_v20_trophy_v.rmdl",
          0, 0, 0, 0 },
        { "Bangalore", "Cold Steel", "mp_weapon_bangalore_heirloom_primary", "melee_bangalore_heirloom",
          "mdl/techart/mshop/weapons/class/heirloom/bangalore/v20_khukuri/heirloom_bangalore_v20_khukuri_v.rmdl",
          0, 0, 0, 0 },
        { "Bangalore", "Emerald Edge", "mp_weapon_bangalore_heirloom_primary", "melee_bangalore_heirloom",
          "mdl/techart/mshop/weapons/class/heirloom/bangalore/v20_khukuri/heirloom_bangalore_v20_khukuri_v.rmdl",
          0, 0, 0, 0 },
        { "Revenant", "Dead Man's Curve", "mp_weapon_revenant_scythe_primary", "melee_revenant_scythe",
          "mdl/techart/mshop/weapons/class/heirloom/revenant/v21_scythe/heirloom_revenant_v21_scythe_v.rmdl",
          0, 0, 0, 0 },
        { "Revenant", "Death Grip", "mp_weapon_revenant_scythe_primary", "melee_revenant_scythe",
          "mdl/techart/mshop/weapons/class/heirloom/revenant/v21_scythe/heirloom_revenant_v21_scythe_v.rmdl",
          0, 0, 0, 0 },
        { "Crypto", "Biwon Blade", "mp_weapon_crypto_heirloom_primary", "melee_crypto_heirloom",
          "mdl/techart/mshop/weapons/class/heirloom/crypto/v21_sword/heirloom_crypto_v21_sword_v.rmdl",
          0, 0, 0, 0 },
        { "Crypto", "Durumi Blade", "mp_weapon_crypto_heirloom_primary", "melee_crypto_heirloom",
          "mdl/techart/mshop/weapons/class/heirloom/crypto/v21_sword/heirloom_crypto_v21_sword_v.rmdl",
          0, 0, 0, 0 },
        { "Ash", "Strongest Link", "mp_weapon_ash_heirloom_primary", "melee_ash_heirloom",
          "mdl/techart/mshop/weapons/class/heirloom/ash/v22_nunchuck/heirloom_ash_v22_nunchuck_v.rmdl",
          0, 0, 0, 0 },
        { "Rampart", "Problem Solver", "mp_weapon_rampart_wrench_primary", "melee_rampart_wrench",
          "mdl/techart/mshop/weapons/class/heirloom/rampart/v21_wrench/heirloom_rampart_v21_wrench_v.rmdl",
          0, 0, 0, 0 },
        { "Horizon", "Gravity Maw", "mp_weapon_horizon_heirloom_primary", "melee_horizon_heirloom",
          "mdl/techart/mshop/weapons/class/heirloom/horizon/v23_mace/heirloom_horizon_v23_mace_v.rmdl",
          0, 0, 0, 0 },
        { "Fuse", "Razor's Edge", "mp_weapon_fuse_heirloom_primary", "melee_fuse_heirloom",
          "mdl/techart/mshop/weapons/class/heirloom/fuse/v23_guitar/heirloom_fuse_v23_guitar_v.rmdl",
          0, 0, 0, 0 },
        { "Seer", "Showstoppers", "mp_weapon_seer_heirloom_primary", "melee_seer_heirloom",
          "mdl/techart/mshop/weapons/class/heirloom/seer/v22_shotel/heirloom_seer_v22_shotel_v.rmdl",
          0, 0, 0, 0 },
        { "Wattson", "Energy Reader", "mp_weapon_wattson_gadget_primary", "melee_wattson_gadget",
          "mdl/techart/mshop/weapons/class/heirloom/wattson/v21_scanner/heirloom_wattson_v21_scanner_v.rmdl",
          0, 0, 0, 0 },
        { "Valkyrie", "Suzaku", "mp_weapon_valkyrie_spear_primary", "melee_valkyrie_spear",
          "mdl/techart/mshop/weapons/class/heirloom/valkyrie/v22_spear/heirloom_valkyrie_v22_spear_v.rmdl",
          0, 0, 0, 0 },
        { "Loba", "Garra de Alanza", "mp_weapon_loba_heirloom_primary", "melee_loba_heirloom",
          "mdl/techart/mshop/weapons/class/heirloom/loba/v22_fan/heirloom_loba_v22_fan_v.rmdl",
          0, 0, 0, 0 },

        // Universal melees. Resolve their current model from WeaponDataRegistry.
        { "Universels", "Power Sword", "mp_weapon_newcastle_heirloom_primary", "melee_newcastle_heirloom",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v24_riseego/heirloom_agnostic_v24_riseego_v.rmdl",
          0, 0, 0, 0 },
        { "Universels", "Rift Metal", "mp_weapon_newcastle_heirloom_primary", "melee_newcastle_heirloom",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v24_riseego/heirloom_agnostic_v24_riseego_v.rmdl",
          0, 0, 0, 0 },
        { "Universels", "Fury Metal", "mp_weapon_newcastle_heirloom_primary", "melee_newcastle_heirloom",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v24_riseego/heirloom_agnostic_v24_riseego_v.rmdl",
          0, 0, 0, 0 },
        { "Universels", "Radiant Metal", "mp_weapon_newcastle_heirloom_primary", "melee_newcastle_heirloom",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v24_riseego/heirloom_agnostic_v24_riseego_v.rmdl",
          0, 0, 0, 0 },
        { "Universels", "Katar 1", "mp_weapon_artifact_dagger_primary", "melee_artifact_dagger",
          "mdl/techart/mshop/weapons/class/artifact/v23_set/artifact_v23_set_mob_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Katar 2", "mp_weapon_artifact_dagger_primary", "melee_artifact_dagger",
          "mdl/techart/mshop/weapons/class/artifact/v23_set/artifact_v23_set_mob_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Katar 3", "mp_weapon_artifact_dagger_primary", "melee_artifact_dagger",
          "mdl/techart/mshop/weapons/class/artifact/v23_set/artifact_v23_set_mob_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Buster Sword R5", "mp_weapon_artifact_sword_primary", "melee_artifact_sword",
          "mdl/techart/mshop/weapons/class/artifact/ragold/ragold_artifact_base_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Serpent's Sting", "mp_weapon_hook_sword_primary", "melee_hook_sword",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v24_maniaworld/heirloom_hooksword_v24_maniaworld_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Raptor's Claw", "mp_weapon_agnostic_karambit_primary", "melee_agnostic_karambit",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v24_cosmicmerc/heirloom_karambit_v24_cosmicmerc_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Hype Beast's Claw", "mp_weapon_agnostic_karambit_primary", "melee_agnostic_karambit",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v24_cosmicmerc/heirloom_karambit_v24_cosmicmerc_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Phoenix's Claw", "mp_weapon_agnostic_karambit_primary", "melee_agnostic_karambit",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v24_cosmicmerc/heirloom_karambit_v24_cosmicmerc_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Fluorescent Claw", "mp_weapon_agnostic_karambit_primary", "melee_agnostic_karambit",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v24_cosmicmerc/heirloom_karambit_v24_cosmicmerc_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Frozen Claw", "mp_weapon_agnostic_karambit_primary", "melee_agnostic_karambit",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v24_cosmicmerc/heirloom_karambit_v24_cosmicmerc_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Neon Claw", "mp_weapon_agnostic_karambit_primary", "melee_agnostic_karambit",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v24_cosmicmerc/heirloom_karambit_v24_cosmicmerc_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Sky Piercer", "mp_weapon_agnostic_prorookie_primary", "melee_agnostic_prorookie",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_prorookie/heirloom_agnostic_v25_prorookie_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Midnight Piercer", "mp_weapon_agnostic_prorookie_primary", "melee_agnostic_prorookie",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_prorookie/heirloom_agnostic_v25_prorookie_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Amethyst Piercer", "mp_weapon_agnostic_prorookie_primary", "melee_agnostic_prorookie",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_prorookie/heirloom_agnostic_v25_prorookie_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Emerald Piercer", "mp_weapon_agnostic_prorookie_primary", "melee_agnostic_prorookie",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_prorookie/heirloom_agnostic_v25_prorookie_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Grand Slam", "mp_weapon_agnostic_rivnation_primary", "melee_agnostic_rivnation",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_rivnation/heirloom_agnostic_v25_rivnation_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Corked Crimson", "mp_weapon_agnostic_rivnation_primary", "melee_agnostic_rivnation",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_rivnation/heirloom_agnostic_v25_rivnation_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Designated Yeeter", "mp_weapon_agnostic_rivnation_primary", "melee_agnostic_rivnation",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_rivnation/heirloom_agnostic_v25_rivnation_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Blue Dinger", "mp_weapon_agnostic_rivnation_primary", "melee_agnostic_rivnation",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_rivnation/heirloom_agnostic_v25_rivnation_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Death Duet", "mp_weapon_agnostic_veloform_primary", "melee_agnostic_veloform",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_veloform/heirloom_agnostic_v25_veloform_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Sakuras Edge", "mp_weapon_agnostic_veloform_primary", "melee_agnostic_veloform",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_veloform/heirloom_agnostic_v25_veloform_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Venom Strike", "mp_weapon_agnostic_veloform_primary", "melee_agnostic_veloform",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_veloform/heirloom_agnostic_v25_veloform_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Bloodmoon Blade", "mp_weapon_agnostic_veloform_primary", "melee_agnostic_veloform",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_veloform/heirloom_agnostic_v25_veloform_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Beam Saber", "mp_weapon_agnostic_breachbeamsaber_primary", "melee_agnostic_breachbeamsaber",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_breachbeamsaber/heirloom_agnostic_v25_breachbeamsaber_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Sky Saber", "mp_weapon_agnostic_breachbeamsaber_primary", "melee_agnostic_breachbeamsaber",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_breachbeamsaber/heirloom_agnostic_v25_breachbeamsaber_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Solar Saber", "mp_weapon_agnostic_breachbeamsaber_primary", "melee_agnostic_breachbeamsaber",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_breachbeamsaber/heirloom_agnostic_v25_breachbeamsaber_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Spectral Saber", "mp_weapon_agnostic_breachbeamsaber_primary", "melee_agnostic_breachbeamsaber",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_breachbeamsaber/heirloom_agnostic_v25_breachbeamsaber_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Epyon Heat Rod", "mp_weapon_agnostic_breachbeamwhip_primary", "melee_agnostic_breachbeamwhip",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_breachbeamwhip/heirloom_agnostic_v25_breachbeamwhip_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Metallic Banshee", "mp_weapon_agnostic_whip_primary", "melee_agnostic_whip",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_whip/heirloom_agnostic_v25_whip_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Twin Razors", "mp_weapon_agnostic_clockwild_primary", "melee_agnostic_clockwild",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_clockwild/heirloom_agnostic_v25_clockwild_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Prismatic Edge", "mp_weapon_agnostic_clockwild_primary", "melee_agnostic_clockwild",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_clockwild/heirloom_agnostic_v25_clockwild_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Venomous Fangs", "mp_weapon_agnostic_clockwild_primary", "melee_agnostic_clockwild",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_clockwild/heirloom_agnostic_v25_clockwild_v.rmdl", 0, 0, 0, 0 },
        { "Universels", "Glitch Rippers", "mp_weapon_agnostic_clockwild_primary", "melee_agnostic_clockwild",
          "mdl/techart/mshop/weapons/class/heirloom/agnostic/v25_clockwild/heirloom_agnostic_v25_clockwild_v.rmdl", 0, 0, 0, 0 },
    };

		static constexpr skin_cosmetic_t k_skin_cosmetics[] = {
        { "Hope's Dusk", 1699246881u, 1, 0 },
        { "Cold Steel", 1796294725u, 1, 0 },
        { "Emerald Edge", 1460582798u, 2, 0 },
        { "Raven's Bite", 1195204293u, 1, 0 },
        { "Winter's Bane", 1469336076u, 2, 0 },
        { "Death Hammer", 1480704598u, 1, 0 },
        { "War Club", 310155874u, 1, 0 },
        { "Shock Sticks", 1604275697u, 1, 0 },
        { "Too Much Witt", 547619595u, 1, 0 },
        { "Butterfly Knife", 1664195551u, 1, 0 },
        { "Octane's Prototype", 832275184u, 2, 0 },
        { "Boxing Gloves", 1696067991u, 1, 0 },
        { "Belva Bruisers", 1243554992u, 2, 0 },
        { "Energy Reader", 643211997u, 1, 0 },
        { "Biwon Blade", 1371662593u, 1, 0 },
        { "Durumi Blade", 1980968388u, 2, 0 },
        { "Dead Man's Curve", 917971300u, 1, 0 },
        { "Death Grip", 1517007105u, 2, 0 },
        { "Garra de Alanza", 3832492u, 1, 0 },
        { "Problem Solver", 1995161603u, 1, 0 },
        { "Gravity Maw", 1015480786u, 1, 0 },
        { "Suzaku", 260534220u, 1, 0 },
        { "Strongest Link", 1741148596u, 1, 0 },
        { "Showstoppers", 1036437235u, 1, 0 },
        { "Hope's Dawn", 991645580u, 1, 0 },
        { "Razor's Edge", 771582856u, 0, 0 },
        { "Katar 1", 1182724979u, 0, 0 },
        { "Katar 2", 2012311240u, 0, 0 },
        { "Katar 3", 399206089u, 0, 0 },
        { "Power Sword", 299455065u, 1, 0 },
        { "Rift Metal", 938599645u, 2, 0 },
        { "Fury Metal", 1485350717u, 3, 0 },
        { "Radiant Metal", 1917371551u, 4, 0 },
        { "Buster Sword R5", 947193899u, 0, 0 },
        { "Serpent's Sting", 104006608u, 0, 0 },
        { "Raptor's Claw", 701352910u, 1, 0 },
        { "Hype Beast's Claw", 1334276782u, 2, 0 },
        { "Phoenix's Claw", 670036670u, 3, 0 },
        { "Fluorescent Claw", 1401342333u, 4, 0 },
        { "Frozen Claw", 1976544958u, 5, 0 },
        { "Neon Claw", 729099036u, 6, 0 },
        { "Sky Piercer", 888768782u, 1, 0 },
        { "Midnight Piercer", 1149952879u, 2, 0 },
        { "Amethyst Piercer", 2013362460u, 3, 0 },
        { "Emerald Piercer", 1152183726u, 4, 0 },
        { "Grand Slam", 76590070u, 1, 0 },
        { "Corked Crimson", 543142459u, 2, 0 },
        { "Designated Yeeter", 1693975615u, 3, 0 },
        { "Blue Dinger", 1227975441u, 4, 0 },
        { "Death Duet", 1939741787u, 1, 0 },
        { "Sakuras Edge", 187114568u, 3, 0 },
        { "Venom Strike", 1204927775u, 4, 0 },
        { "Bloodmoon Blade", 441203325u, 2, 0 },
        { "Beam Saber", 709194556u, 1, 0 },
        { "Sky Saber", 611075273u, 2, 0 },
        { "Solar Saber", 415572746u, 3, 0 },
        { "Spectral Saber", 1555517436u, 4, 0 },
        { "Epyon Heat Rod", 324261460u, 0, 0 },
        { "Metallic Banshee", 932557460u, 1, 0 },
        { "Twin Razors", 132465672u, 1, 0 },
        { "Prismatic Edge", 1120222242u, 2, 0 },
        { "Venomous Fangs", 683387021u, 3, 0 },
        { "Glitch Rippers", 359204080u, 4, 0 },
    };

		static const bool k_guids_applied = [ & ]( )
		{
			for ( auto& entry : k_list )
			{
				for ( const auto& cosmetic : k_skin_cosmetics )
				{
					if ( std::string_view( entry.name ) == cosmetic.name )
					{
						entry.melee_guid = cosmetic.guid;
						entry.skin_index = cosmetic.skin_index;
						entry.camo_index = cosmetic.camo_index;
						break;
					}
				}
			}
			return true;
		}( );
		( void )k_guids_applied;
		return k_list;
	}
}

