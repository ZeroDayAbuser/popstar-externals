#pragma once
#include <chrono>
#include <glm/glm.hpp>
#include <sdk/globals.hpp>
#include <vector>

namespace rust
{
	enum class BoneList : i32
	{
		pelvis = 0,
		l_hip = 1,
		l_hip_twist_02 = 2,
		l_knee = 3,
		l_foot = 4,
		l_toe = 5,
		l_knee_twist_02 = 6,
		GenitalCensor = 8,
		GenitalCensor_LOD0 = 9,
		Inner_LOD0 = 10,
		GenitalCensor_LOD1 = 12,
		GenitalCensor_LOD2 = 13,
		r_hip = 14,
		r_hip_twist_02 = 15,
		r_knee = 16,
		r_foot = 17,
		r_toe = 18,
		r_knee_twist_02 = 19,
		spine1 = 20,
		spine2 = 21,
		spine3 = 22,
		spine4 = 23,
		l_clavicle = 24,
		l_upperarm = 25,
		l_forearm = 26,
		l_forearm_twist_01 = 27,
		l_forearm_twist_02 = 28,
		l_hand = 29,
		l_index_meta = 30,
		l_index1 = 31,
		l_index2 = 32,
		l_index3 = 33,
		l_middle_meta = 34,
		l_middle1 = 35,
		l_middle2 = 36,
		l_middle3 = 37,
		l_pinky_meta = 38,
		l_little1 = 39,
		l_little2 = 40,
		l_little3 = 41,
		l_prop = 42,
		l_ring_meta = 43,
		l_ring1 = 44,
		l_ring2 = 45,
		l_ring3 = 46,
		l_thumb1 = 47,
		l_thumb2 = 48,
		l_thumb3 = 49,
		l_upperarm_twist_01 = 50,
		l_upperarm_twist_02 = 51,
		neck = 52,
		head = 53,
		eyeTransform = 54,
		jaw = 55,
		l_eye = 56,
		l_Eyelid = 57,
		r_eye = 58,
		r_Eyelid = 59,
		r_clavicle = 60,
		r_upperarm = 61,
		r_forearm = 62,
		r_forearm_twist_01 = 63,
		r_forearm_twist_02 = 64,
		r_hand = 65,
		r_index_meta = 66,
		r_index1 = 67,
		r_index2 = 68,
		r_index3 = 69,
		r_middle_meta = 70,
		r_middle1 = 71,
		r_middle2 = 72,
		r_middle3 = 73,
		r_pinky_meta = 74,
		r_little1 = 75,
		r_little2 = 76,
		r_little3 = 77,
		r_prop = 78,
		r_ring_meta = 79,
		r_ring1 = 80,
		r_ring2 = 81,
		r_ring3 = 82,
		r_thumb1 = 83,
		r_thumb2 = 84,
		r_thumb3 = 85,
		r_upperarm_twist_01 = 86,
		r_upperarm_twist_02 = 87,
		BoobCensor = 88,
		BreastCensor_LOD0 = 89,
		BreastCensor_LOD1 = 92,
		BreastCensor_LOD2 = 93
	};

	struct BoneData
	{
		glm::vec3 position{};
		bool visible{ false };
		std::chrono::steady_clock::time_point last_seen{};
	};

	struct BoneTransformRaw
	{
		uptr transform_data{};
		i32 index{};
	};

	struct BoneMatrix3x4
	{
		float vec0[4]{};
		float vec1[4]{};
		float vec2[4]{};
	};
}
