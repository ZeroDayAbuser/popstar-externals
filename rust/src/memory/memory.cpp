#include "../utils/utils.hpp"
#include "memory.hpp"
#include "camouflage.hpp"
#include "driver/driver_memory.hpp"

#include <cstring>

#include <TlHelp32.h>

#include <chrono>
#include <sstream>
#include <thread>
#include <vector>

#include "../globals.hpp"
#include <isyscall/inline_syscall.hpp>
#include <print>
#include <utils/debug.hpp>
#include "../utils/logging.hpp"
#include <string_encryption.hpp>
#include "../utils/vmp.hpp"

#include "../game/game.hpp"

#include "../settings/settings.hpp"

namespace memory
{
	std::atomic<bool> g_writes_enabled{ true };

	static DriverMemory g_driver{};

	namespace
	{
		struct handle_data {
			unsigned long process_id;
			HWND window_handle;
		};

		BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam) {
			handle_data& data = *(handle_data*)lParam;
			unsigned long process_id = 0;
			GetWindowThreadProcessId(handle, &process_id);
			if (data.process_id != process_id || !IsWindowVisible(handle))
				return TRUE;
			data.window_handle = handle;
			return FALSE;
		}

		HWND find_main_window(unsigned long process_id) {
			handle_data data;
			data.process_id = process_id;
			data.window_handle = 0;
			EnumWindows(enum_windows_callback, (LPARAM)&data);
			return data.window_handle;
		}
	}

	void create_instance(std::string_view target_process)
	{
		VMP_START("create_instance");
		if (!isyscall::load())
		{
			VMP_END;
			return;
		}

		std::wstring target_w(target_process.begin(), target_process.end());

		logging::print("waiting for rust...");
		while (impl::process_id == 0)
		{
			impl::process_id = static_cast<i32>(g_driver.get_process_id(target_w));
			if (impl::process_id == 0)
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		logging::print("found rust (pid %d)", impl::process_id);

		logging::print("waiting for hypervisor...");
		while (!g_driver.attach(target_w))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
		logging::print("hypervisor attached");

		std::this_thread::sleep_for(std::chrono::seconds(5));

		auto refresh_proc = [&]() {
			const proc_info_request* proc = g_driver.get_proc_data();
			impl::process_peb = reinterpret_cast<uptr>(proc->peb);
			impl::cr3 = proc->cr3;
			impl::base_address = reinterpret_cast<uptr>(proc->base_address);
			if (!impl::base_address)
				impl::base_address = g_driver.get_module_base(target_w);
		};

		refresh_proc();
		while (impl::base_address == 0)
		{
			if (!g_driver.attach(target_w))
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}
			refresh_proc();
			if (impl::base_address == 0)
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		DBG("base address: {:#x}", impl::base_address);
		DBG("process PEB: {:#x}", impl::process_peb);
		DBG("cr3: {:#x}", impl::cr3);

		{
			uint8_t hdr[4]{};
			if (g_driver.read(impl::base_address, hdr, sizeof(hdr)))
			{
				DBG("driver R/W test: header {:02x} {:02x} {:02x} {:02x} ({}{})",
					hdr[0], hdr[1], hdr[2], hdr[3],
					(char)hdr[0], (char)hdr[1]);
			}
		}

		while (impl::game_hwnd == nullptr)
		{
			impl::game_hwnd = find_main_window(impl::process_id);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (impl::process_peb)
		{
			uptr process_parameters = memory::read<uptr>(impl::process_peb + 0x20);
			if (process_parameters)
			{
				uptr string_buffer = memory::read<uptr>(process_parameters + 0x60 + 0x8);
				unsigned short string_len = memory::read<unsigned short>(process_parameters + 0x60);

				if (string_buffer && string_len)
				{
					std::wstring full_path = memory::read_wstring<2>(string_buffer, string_len / 2);
					size_t pos = full_path.find_last_of(L"\\/");
					if (pos != std::wstring::npos)
					{
						std::wstring dir = full_path.substr(0, pos);
						impl::game_directory = std::string(dir.begin(), dir.end());
						DBG("game directory (PEB): {}", impl::game_directory);
					}
				}
			}
		}

		if (impl::game_directory.empty())
		{
			HANDLE h = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE,
				static_cast<DWORD>(impl::process_id));
			if (h)
			{
				wchar_t buf[MAX_PATH * 2]{};
				DWORD len = static_cast<DWORD>(std::size(buf));
				if (::QueryFullProcessImageNameW(h, 0, buf, &len) && len > 0)
				{
					std::wstring full_path(buf, len);
					size_t pos = full_path.find_last_of(L"\\/");
					if (pos != std::wstring::npos)
					{
						std::wstring dir = full_path.substr(0, pos);
						impl::game_directory = std::string(dir.begin(), dir.end());
						DBG("game directory (QFPIN): {}", impl::game_directory);
					}
				}
				::CloseHandle(h);
			}

			if (impl::game_directory.empty())
				DBG("game directory: <empty>, item icons will fail to load");
		}

		logging::print("game ready (pid %d)", impl::process_id);
		on_frame();
		VMP_END;
	}


	struct pattern_byte
	{
		uint8_t value;
		bool wildcard;
	};

	static std::vector<pattern_byte> parse_ida_pattern(const std::string& sig)
	{
		std::vector<pattern_byte> result;
		std::istringstream ss(sig);
		std::string token;
		while (ss >> token)
		{
			if (token == "?")
				result.push_back({ 0, true });
			else
				result.push_back({ static_cast<uint8_t>(strtoul(token.c_str(), nullptr, 16)), false });
		}
		return result;
	}

	std::uintptr_t pattern_scan(std::uintptr_t module_base, const std::string& signature, uintptr_t start_from)
	{
		auto pattern = parse_ida_pattern(signature);
		if (pattern.empty()) return 0;

		uint8_t header[0x1000]{};
		if (!read_memory_raw(module_base, header, sizeof(header)))
			return 0;

		auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(header);
		if (dos->e_magic != IMAGE_DOS_SIGNATURE) return 0;

		auto* nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(header + dos->e_lfanew);
		if (nt->Signature != IMAGE_NT_SIGNATURE) return 0;

		auto* sections = IMAGE_FIRST_SECTION(nt);

		for (WORD s = 0; s < nt->FileHeader.NumberOfSections; s++)
		{
			bool is_readable = (sections[s].Characteristics & IMAGE_SCN_MEM_READ) != 0;
			if (!is_readable)
				continue;

			uptr sec_rva = sections[s].VirtualAddress;
			uptr sec_size = sections[s].Misc.VirtualSize;

			constexpr size_t PAGE_SIZE = 0x1000;
			const size_t carry_size = pattern.size() - 1;
			std::vector<uint8_t> buffer(PAGE_SIZE + carry_size);
			size_t carried = 0;

			uptr start_offset = (start_from > (module_base + sec_rva)) ? (start_from - (module_base + sec_rva)) : 0;
			if (start_offset >= sec_size) continue;

			for (uptr offset = start_offset; offset < sec_size; offset += PAGE_SIZE)
			{
				size_t read_size = std::min<size_t>(PAGE_SIZE, sec_size - offset);
				uptr addr = module_base + sec_rva + offset;

				if (!read_memory_raw(addr, buffer.data() + carried, read_size))
				{
					carried = 0;
					continue;
				}

				size_t total = carried + read_size;

				for (size_t i = 0; i + pattern.size() <= total; i++)
				{
					bool found = true;
					for (size_t j = 0; j < pattern.size(); j++)
					{
						if (!pattern[j].wildcard && buffer[i + j] != pattern[j].value)
						{
							found = false;
							break;
						}
					}
					if (found)
						return addr - carried + i;
				}

				if (total >= carry_size)
				{
					memmove(buffer.data(), buffer.data() + total - carry_size, carry_size);
					carried = carry_size;
				}
				else
				{
					carried = total;
				}
			}
		}

		return 0;
	}

	std::uintptr_t resolve_lea(std::uintptr_t lea_address)
	{
		int32_t disp = read<int32_t>(lea_address + 3);
		return lea_address + 7 + disp;
	}

	void on_frame()
	{
	}

	static std::atomic<bool> g_io_disabled{ false };

	bool is_io_disabled()
	{
		return g_io_disabled.load(std::memory_order_acquire);
	}

	bool is_initialized()
	{
		return g_driver.is_attached() && impl::process_id != 0 &&
			impl::base_address != 0;
	}

	void cleanup()
	{
		if (g_io_disabled.exchange(true, std::memory_order_acq_rel))
			return;
		g_writes_enabled.store(false, std::memory_order_release);
		std::println("memory::cleanup, detaching Astrum driver");
		g_driver.detach();
	}

	alignas(4096) thread_local static unsigned char g_io_bounce[4096];
	thread_local static bool g_io_bounce_pinned = false;

	void bump_working_set();

	static void force_touch_bounce()
	{
		*reinterpret_cast<volatile unsigned char*>(g_io_bounce) = 0;
		*reinterpret_cast<volatile unsigned char*>(g_io_bounce + sizeof(g_io_bounce) - 1) = 0;
	}

	static void ensure_bounce_pinned()
	{
		if (!g_io_bounce_pinned) {
			if (!::VirtualLock(g_io_bounce, sizeof(g_io_bounce))) {
				bump_working_set();
				if (!::VirtualLock(g_io_bounce, sizeof(g_io_bounce))) {
					force_touch_bounce();
					return;
				}
			}
			g_io_bounce_pinned = true;
		}
		force_touch_bounce();
	}

	void bump_working_set()
	{
		SIZE_T cur_min = 0, cur_max = 0;
		HANDLE hp = ::GetCurrentProcess();
		::GetProcessWorkingSetSize(hp, &cur_min, &cur_max);
		constexpr SIZE_T kWantMin = 32ull * 1024 * 1024;   // 32 MB
		constexpr SIZE_T kWantMax = 512ull * 1024 * 1024;  // 512 MB
		if (cur_min < kWantMin || cur_max < kWantMax) {
			::SetProcessWorkingSetSize(hp,
				std::max<SIZE_T>(cur_min, kWantMin),
				std::max<SIZE_T>(cur_max, kWantMax));
		}
	}

	void pin_thread_stack()
	{
		ensure_bounce_pinned();
	}

	bool read_memory_raw(const std::uintptr_t& address, void* buffer, std::size_t size)
	{
		if (g_io_disabled.load(std::memory_order_acquire))
			return false;
		return g_driver.read(address, buffer, size);
	}

	bool write_memory_raw(const std::uintptr_t& address, void* buffer, std::size_t size)
	{
		if (g_io_disabled.load(std::memory_order_acquire))
			return false;

		const bool now_effective =
			settings.settings.ui.enable_memory_writes &&
			g_writes_enabled.load(std::memory_order_acquire);

		static std::atomic<bool>    s_last_effective{ false };
		static std::atomic<int64_t> s_enabled_at_ns{ 0 };

		if (!now_effective) {
			s_last_effective.store(false, std::memory_order_release);
			return false;
		}

		const auto now_ns = std::chrono::steady_clock::now().time_since_epoch().count();
		bool was_effective = s_last_effective.exchange(true, std::memory_order_acq_rel);
		if (!was_effective) {
			s_enabled_at_ns.store(now_ns, std::memory_order_release);
			return false;
		}
		const auto enabled_at = s_enabled_at_ns.load(std::memory_order_acquire);
		if (now_ns - enabled_at < 500'000'000LL)
			return false;

		if (!address || !buffer || !size || size > 0x1000)
			return false;
		if (address < 0x10000ULL || address > 0x00007FFFFFFFFFFFULL)
			return false;

		return g_driver.write(buffer, address, size);
	}

	std::uintptr_t get_base_address()
	{
		return impl::base_address;
	}

	std::string get_game_directory()
	{
		return impl::game_directory;
	}

	std::pair<std::uintptr_t, std::size_t> get_module_easy(std::wstring_view name)
	{
		if (!impl::process_id)
			return {};

		std::wstring module_name(name);
		const auto base = g_driver.get_module_base(module_name);
		if (base)
		{
			const auto e_lfanew = read<std::uint32_t>(base + 0x3C);
			const auto size = e_lfanew && e_lfanew < 0x1000
				? read<std::uint32_t>(base + e_lfanew + 0x50)
				: 0;
			return { base, size };
		}

		const HANDLE snap = ::CreateToolhelp32Snapshot(
			TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
			static_cast<DWORD>(impl::process_id));

		if (snap == INVALID_HANDLE_VALUE)
			return {};

		MODULEENTRY32W entry{};
		entry.dwSize = sizeof(entry);

		std::pair<std::uintptr_t, std::size_t> result{};
		for (BOOL ok = ::Module32FirstW(snap, &entry); ok; ok = ::Module32NextW(snap, &entry))
		{
			if (::_wcsicmp(entry.szModule, name.data()) == 0)
			{
				result = { reinterpret_cast<std::uintptr_t>(entry.modBaseAddr),
					static_cast<std::size_t>(entry.modBaseSize) };
				break;
			}
		}

		::CloseHandle(snap);
		return result;
	}


	std::pair<std::uintptr_t, std::size_t> get_module(std::wstring_view name)
	{
		if (!impl::process_id || !impl::process_peb)
			return get_module_easy(name);

		uptr ldr = read<uptr>(impl::process_peb + 0x18);
		if (!ldr) return get_module_easy(name);

		uptr list_head = ldr + 0x10;
		uptr current_entry = read<uptr>(list_head);

		while (current_entry != list_head)
		{
			uptr base = read<uptr>(current_entry + 0x30);
			uptr size = read<uptr>(current_entry + 0x40);

			uptr name_ptr = read<uptr>(current_entry + 0x58 + 8);
			unsigned short name_len = read<unsigned short>(current_entry + 0x58);

			if (name_ptr && name_len)
			{
				std::wstring name_buf = read_wstring<2>(name_ptr, name_len / 2);
				if (_wcsicmp(name_buf.c_str(), name.data()) == 0)
				{
					return { base, size };
				}
			}

			current_entry = read<uptr>(current_entry);
			if (!current_entry) break;
		}

		return get_module_easy(name);
	}

	std::pair<std::uintptr_t, std::size_t> get_module_hidden(const std::string& module_name)
	{
		auto [up_base, up_size] = get_module(xs(L"UnityPlayer.dll"));
		if (up_base && up_size)
		{
			DBG("scanning {} ({:#x}) for {} pointer...", xs("UnityPlayer.dll"), up_base, module_name);
			std::vector<uint8_t> up_buffer(up_size);
			if (read_memory_raw(up_base, up_buffer.data(), up_size))
			{
				for (size_t i = 0; i < up_size - sizeof(uptr); i += 8)
				{
					uptr potential_base = *reinterpret_cast<uptr*>(up_buffer.data() + i);

					if (potential_base < 0x100000000 || potential_base > 0x7FFFFFFFFFFF || (potential_base & 0xFFFF) != 0)
						continue;

					uint16_t magic = 0;
					if (!read_memory_raw(potential_base, &magic, sizeof(magic)) || magic != IMAGE_DOS_SIGNATURE)
						continue;

					uint32_t e_lfanew = read<uint32_t>(potential_base + 0x3C);
					if (e_lfanew > 0x1000 || e_lfanew < 0x40)
						continue;

					uint32_t signature = read<uint32_t>(potential_base + e_lfanew);
					if (signature != IMAGE_NT_SIGNATURE)
						continue;

					uint32_t export_dir_rva = read<uint32_t>(potential_base + e_lfanew + 0x88);
					if (!export_dir_rva || export_dir_rva > 0x10000000)
						continue;

					uint32_t name_rva = read<uint32_t>(potential_base + export_dir_rva + 0x0C);
					if (!name_rva || name_rva > 0x10000000)
						continue;

					char name[256] = { 0 };
					if (read_memory_raw(potential_base + name_rva, name, 255))
					{
						if (_stricmp(name, module_name.c_str()) == 0)
						{
							uint32_t size = read<uint32_t>(potential_base + e_lfanew + 0x50);
							DBG("Found {} via {} at {:#x} (size: {:#x})", module_name, xs("UnityPlayer.dll"), potential_base, size);
							return { potential_base, size };
						}
					}
				}
			}
		}

		DBG("{} {}...", xs("UnityPlayer heuristic failed, scanning physical ranges for unlinked module"), module_name);

		for (uptr addr = 0x100000000; addr < 0x500000000; addr += 0x10000)
		{
			uint16_t magic = 0;
			if (!read_memory_raw(addr, &magic, sizeof(magic)) || magic != IMAGE_DOS_SIGNATURE)
				continue;

			uint32_t e_lfanew = read<uint32_t>(addr + 0x3C);
			if (e_lfanew > 0x1000 || e_lfanew < 0x40)
				continue;

			uint32_t signature = read<uint32_t>(addr + e_lfanew);
			if (signature != IMAGE_NT_SIGNATURE)
				continue;

			uint32_t export_dir_rva = read<uint32_t>(addr + e_lfanew + 0x88);
			if (!export_dir_rva || export_dir_rva > 0x10000000)
				continue;

			uint32_t name_rva = read<uint32_t>(addr + export_dir_rva + 0x0C);
			if (!name_rva || name_rva > 0x10000000)
				continue;

			char name[256] = { 0 };
			if (read_memory_raw(addr + name_rva, name, 255))
			{
				if (_stricmp(name, module_name.c_str()) == 0)
				{
					uint32_t size = read<uint32_t>(addr + e_lfanew + 0x50);
					DBG("Found {} at {:#x} (size: {:#x})", module_name, addr, size);
					return { addr, size };
				}
			}
		}

		DBG("Scanning secondary ranges...");
		for (uptr addr = 0x7FF000000000; addr < 0x7FFFFFFFFFFF; addr += 0x10000)
		{
			uint16_t magic = 0;
			if (!read_memory_raw(addr, &magic, sizeof(magic)) || magic != IMAGE_DOS_SIGNATURE)
				continue;

			uint32_t e_lfanew = read<uint32_t>(addr + 0x3C);
			if (e_lfanew > 0x1000 || e_lfanew < 0x40)
				continue;

			uint32_t signature = read<uint32_t>(addr + e_lfanew);
			if (signature != IMAGE_NT_SIGNATURE)
				continue;

			uint32_t export_dir_rva = read<uint32_t>(addr + e_lfanew + 0x88);
			if (!export_dir_rva || export_dir_rva > 0x10000000)
				continue;

			uint32_t name_rva = read<uint32_t>(addr + export_dir_rva + 0x0C);
			if (!name_rva || name_rva > 0x10000000)
				continue;

			char name[256] = { 0 };
			if (read_memory_raw(addr + name_rva, name, 255))
			{
				if (_stricmp(name, module_name.c_str()) == 0)
				{
					uint32_t size = read<uint32_t>(addr + e_lfanew + 0x50);
					DBG("Found {} at {:#x} (size: {:#x})", module_name, addr, size);
					return { addr, size };
				}
			}
		}

		return {};
	}
}
