#pragma once
#include <atomic>
#include <mutex>
#include <unordered_map>

namespace rust
{
	class Entity;
}

namespace cache
{
	bool create_instance();

	void gather();

	void update();

	inline std::recursive_mutex cache_mut = {};
	inline std::unordered_map<std::uintptr_t, rust::Entity*>* entity_list = {};

	inline std::atomic<std::uint32_t> g_cache_epoch{ 0 };

	template <typename T, typename Func>
	void for_each_entity(Func callback)
	{
		std::unique_lock lock(cache_mut);
		if (!entity_list) return;
		for (const auto& pair : *entity_list)
		{
			T* entity = static_cast<T*>(pair.second);
			if (!entity) continue;
			callback(entity);
		}
	}

	template <typename T, typename Func>
	bool try_for_each_entity(Func callback)
	{
		std::unique_lock lock(cache_mut, std::try_to_lock);
		if (!lock.owns_lock()) return false;
		if (!entity_list) return true;
		for (const auto& pair : *entity_list)
		{
			T* entity = static_cast<T*>(pair.second);
			if (!entity) continue;
			callback(entity);
		}
		return true;
	}

	rust::Entity* get_local_player();
}
