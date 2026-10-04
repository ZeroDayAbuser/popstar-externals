#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <atomic>
#include <cstdint>

namespace features::physx
{
	struct HitInfo
	{
		bool      hit{ false };
		glm::vec3 point{ 0, 0, 0 };
		glm::vec3 normal{ 0, 0, 0 };
		float     distance{ 0.0f };
	};

	void init();
	void shutdown();

	HitInfo raycast(const glm::vec3& origin, const glm::vec3& dir, float max_distance);
	HitInfo linecast(const glm::vec3& from, const glm::vec3& to);
	bool    is_visible(const glm::vec3& from, const glm::vec3& to);

	struct BallisticHit
	{
		bool      hit{ false };
		glm::vec3 point{ 0, 0, 0 };
		float     time{ 0.0f };
		std::vector<glm::vec3> path;
	};

	BallisticHit trace_ballistic(const glm::vec3& origin, const glm::vec3& velocity, float gravity_mod, float drag, float max_time = 4.0f);

	bool is_ready();

	struct Stats { int actors{0}; size_t triangles{0}; };
	Stats stats();
}
