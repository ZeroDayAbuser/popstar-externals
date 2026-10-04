#include "../memory/memory.hpp"
#include "game.hpp"

#include <chrono>

#include "cache/cache.hpp"
#include <sdk/offsets.hpp>
#include <utils/debug.hpp>

#include "features/aimbot/aimbot.hpp"
#include <sdk/decryptions.hpp>
#include <sdk/rust/entity/entity.hpp>
#include <sdk/rust/entity/weapon.hpp>
#include <sdk/unity/unity.hpp>


#include <app/app.hpp>
#include <controls/slider/slider.hpp>
#include <globals.hpp>
#include <string_encryption.hpp>

namespace game
{
	void create_instance()
	{
		if (impl::created_instance)
			return;

		{
			auto [base_address_easy, s_easy] = memory::get_module_easy(xs(L"GameAssembly.dll"));
			impl::game_assembly = base_address_easy;

			if (impl::game_assembly == 0)
			{
				auto [base_address, s] = memory::get_module(xs(L"GameAssembly.dll"));
				impl::game_assembly = base_address;
			}

			if (impl::game_assembly == 0)
			{
				auto [base_address, s] = memory::get_module_hidden(xs("GameAssembly.dll"));
				impl::game_assembly = base_address;
			}
		}

		const std::vector<std::uintptr_t> required_offsets = { impl::game_assembly };
		impl::created_instance = std::all_of(required_offsets.begin(), required_offsets.end(),
			[](std::uintptr_t ptr) { return ptr != 0; });

		impl::convar_graphics = impl::game_assembly + offsets::convar_graphics::typeinfo;
		impl::convar_player   = impl::game_assembly + 0xe4583f0;

		if (!impl::buttons_static) {
			const std::string sig =
				xs("48 8B 05 ? ? ? ? 48 8B 80 B8 00 00 00 48 8B 80 A0 08 00 00");
			uintptr_t match = memory::pattern_scan(impl::game_assembly, sig, 0);
			if (match) {
				int32_t disp = memory::read<int32_t>(match + 3);
				impl::buttons_static = match + 7 + disp;
				DBG("[buttons-sig] OK match=0x{:x} class_ptr_addr=0x{:x}",
				             match, impl::buttons_static);
				uptr cls = memory::read<uptr>(impl::buttons_static);
				if (cls) {
					uptr sf = memory::read<uptr>(cls + offsets::StaticFieldsBase);
					if (sf) {
						uptr sprint_btn = memory::read<uptr>(sf + offsets::Buttons_Static::Sprint);
						if (sprint_btn) {
							uint8_t is_down = memory::read<uint8_t>(sprint_btn + offsets::Buttons_ConButton::IsDown);
							DBG("[buttons-sig] cls=0x{:x} sf=0x{:x} sprint=0x{:x} IsDown={}",
							             cls, sf, sprint_btn, (int)is_down);
						} else {
							DBG("[buttons-sig] sprint button null at sf+Buttons_Static::Sprint");
						}
					} else {
						DBG("[buttons-sig] static_fields null at cls+StaticFieldsBase");
					}
				} else {
					DBG("[buttons-sig] class null after deref of buttons_static");
				}
			} else {
				DBG("[buttons-sig] pattern not found — sprint chain disabled");
			}
		}

		cache::create_instance();
	}

	void update_instance()
	{
		create_instance();
		if (!impl::created_instance) return;

		impl::tod_sky = memory::chain_read<uptr>(game::impl::game_assembly, {
			offsets::TOD_Sky::TOD_Sky_C,
			offsets::TOD_Sky::step_1,
			offsets::TOD_Sky::step_2,
			offsets::TOD_Sky::step_3,
			offsets::TOD_Sky::step_4,
		});

		auto block_writes_now = []() {
			memory::g_writes_enabled.store(false, std::memory_order_release);
			impl::camera_object = 0;
		};

		if (g_should_refresh_cache) { block_writes_now(); return; }

		uptr typeinfo = memory::read(impl::game_assembly + offsets::main_camera::typeinfo);
		if (!typeinfo) { block_writes_now(); return; }

		uptr static_fields = memory::read(typeinfo + offsets::main_camera::static_fields);
		if (!static_fields) { block_writes_now(); return; }

		uptr managed_cam = memory::read(static_fields + offsets::main_camera::instance);
		if (!managed_cam) { block_writes_now(); return; }

		uptr native_cam = memory::read(managed_cam + offsets::UnityObject::cached_ptr);
		if (!native_cam) { block_writes_now(); return; }

		impl::camera_object = native_cam;
		impl::view_matrix = memory::read<glm::mat4>(native_cam + offsets::main_camera::view_matrix);

		static auto last_oog_tick = std::chrono::steady_clock::now();
		const bool in_game_now =
			impl::view_matrix[0][0] != 0.0f && !g_should_refresh_cache;
		if (!in_game_now) {
			last_oog_tick = std::chrono::steady_clock::now();
			memory::g_writes_enabled.store(false, std::memory_order_release);
		} else {
			const auto warm = std::chrono::steady_clock::now() - last_oog_tick;
			memory::g_writes_enabled.store(
				warm >= std::chrono::milliseconds(500),
				std::memory_order_release);
		}

		features::aimbot::on_tick();
	}

	bool is_in_game()
	{
		return impl::view_matrix[0][0] != 0.0f && !g_should_refresh_cache;
	}
}