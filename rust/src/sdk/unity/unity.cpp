#include "unity.hpp"
#include <memory/memory.hpp>
#include <string>
#include <vector>

namespace unity
{
	uptr get_component_by_id(uptr game_object, i32 index)
	{
		if (!game_object) return 0;

		uptr component_array = memory::read<uptr>(game_object + 0x20);
		if (!component_array) return 0;

		uptr native_comp = memory::read<uptr>(component_array + index * 0x10 + 0x8);
		if (!native_comp) return 0;

		uptr handle = memory::read<uptr>(native_comp + 0x18);
		if (!handle || (handle & 1)) return 0;

		uptr managed_obj = memory::read<uptr>(handle);
		return managed_obj;
	}

	uptr get_component_by_name(uptr seed, const char* class_name)
	{
		if (!seed || !class_name) return 0;

		uptr native = memory::read<uptr>(seed + 0x10);
		if (!native) return 0;

		uptr owner = memory::read<uptr>(native + 0x20);
		if (!owner) return 0;

		uptr component_array = memory::read<uptr>(owner + 0x20);
		i32  component_count = memory::read<i32> (owner + 0x30);
		if (!component_array || component_count <= 0 || component_count > 256)
			return 0;

		for (i32 i = 0; i < component_count; ++i)
		{
			uptr native_comp = memory::read<uptr>(component_array + i * 0x10 + 0x8);
			if (!native_comp) continue;

			uptr handle = memory::read<uptr>(native_comp + 0x18);
			if (!handle || (handle & 1)) continue;

			uptr managed_obj = memory::read<uptr>(handle);
			if (!managed_obj) continue;

			uptr klass = memory::read<uptr>(managed_obj);
			if (!klass) continue;

			uptr name_ptr = memory::read<uptr>(klass + 0x10);
			if (!name_ptr) continue;

			std::string name = memory::read_string(name_ptr);
			if (name == class_name)
				return managed_obj;
		}

		return 0;
	}

	static void collect_weapon_renderers(uptr game_object, std::vector<uptr>& gun_renderers, i32 depth)
	{
		if (!game_object || depth > 8) return;

		uptr component_array = memory::read<uptr>(game_object + 0x20);
		i32  component_count = memory::read<i32>(game_object + 0x30);
		if (!component_array || component_count <= 0 || component_count > 256)
			return;

		for (i32 i = 0; i < component_count; ++i)
		{
			uptr native_comp = memory::read<uptr>(component_array + i * 0x10 + 0x8);
			if (!native_comp) continue;

			uptr handle = memory::read<uptr>(native_comp + 0x18);
			if (!handle || (handle & 1)) continue;
			uptr managed_obj = memory::read<uptr>(handle);
			if (!managed_obj) continue;

			uptr klass = memory::read<uptr>(managed_obj);
			if (!klass) continue;
			uptr name_ptr = memory::read<uptr>(klass + 0x10);
			if (!name_ptr) continue;
			const std::string name = memory::read_string(name_ptr);

			if (name == "SkinnedMeshRenderer" || name == "MeshRenderer")
				gun_renderers.push_back(native_comp);

			if (name != "Transform") continue;

			uptr child_list = memory::read<uptr>(native_comp + 0x70);
			i32  child_size = memory::read<i32>(native_comp + 0x80);
			if (!child_list || child_size <= 0 || child_size > 256) continue;

			for (i32 c = 0; c < child_size; ++c)
			{
				uptr child_tf = memory::read<uptr>(child_list + c * 0x8);
				if (!child_tf) continue;
				uptr child_go = memory::read<uptr>(child_tf + 0x30);
				if (!child_go) continue;
				uptr child_name_ptr = memory::read<uptr>(child_go + 0x60);
				if (!child_name_ptr) continue;
				const std::string child_name = memory::read_string(child_name_ptr);
				if (child_name.find("holosight") != std::string::npos) continue;
				collect_weapon_renderers(child_go, gun_renderers, depth + 1);
			}
		}
	}

	void get_components_in_children(uptr game_object,
		std::vector<uptr>& gun_renderers,
		std::vector<uptr>& hand_renderers,
		i32 depth, bool weapon)
	{
		if (!game_object || depth > 8) return;

		uptr component_array = memory::read<uptr>(game_object + 0x20);
		i32  component_count = memory::read<i32>(game_object + 0x30);
		if (!component_array || component_count <= 0 || component_count > 256)
			return;

		for (i32 i = 0; i < component_count; ++i)
		{
			uptr native_comp = memory::read<uptr>(component_array + i * 0x10 + 0x8);
			if (!native_comp) continue;

			uptr handle = memory::read<uptr>(native_comp + 0x18);
			if (!handle || (handle & 1)) continue;
			uptr managed_obj = memory::read<uptr>(handle);
			if (!managed_obj) continue;

			uptr klass = memory::read<uptr>(managed_obj);
			if (!klass) continue;
			uptr name_ptr = memory::read<uptr>(klass + 0x10);
			if (!name_ptr) continue;
			const std::string name = memory::read_string(name_ptr);

			if (name == "SkinnedMeshRenderer" || name == "MeshRenderer")
			{
				if (weapon) gun_renderers.push_back(native_comp);
				else        hand_renderers.push_back(native_comp);
			}

			if (name != "Transform") continue;

			uptr child_list = memory::read<uptr>(native_comp + 0x70);
			i32  child_size = memory::read<i32>(native_comp + 0x80);
			if (!child_list || child_size <= 0 || child_size > 256) continue;

			for (i32 c = 0; c < child_size; ++c)
			{
				uptr child_tf = memory::read<uptr>(child_list + c * 0x8);
				if (!child_tf) continue;
				uptr child_go = memory::read<uptr>(child_tf + 0x30);
				if (!child_go) continue;
				uptr child_name_ptr = memory::read<uptr>(child_go + 0x60);
				if (!child_name_ptr) continue;
				const std::string child_name = memory::read_string(child_name_ptr);
				if (child_name.find("holosight") != std::string::npos) continue;

				if (child_name.rfind("v_", 0) == 0)
					collect_weapon_renderers(child_go, gun_renderers, depth + 1);
				else
					get_components_in_children(child_go, gun_renderers, hand_renderers, depth + 1, false);
			}
		}
	}
}
