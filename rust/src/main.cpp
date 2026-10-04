#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <print>
#include <string>
#include <thread>
#include <windows.h>

#include "app/winapp.hpp"
#include "utils/logging.hpp"
#include "auth/license.hpp"
#include <utils/debug.hpp>

#include "memory/memory.hpp"
#include "game/features/physx/physx.hpp"
#include "sdk/rust/unity_bundle/unity_bundle.hpp"
#include "utils/vmp.hpp"
#include <globals.hpp>
#include <isyscall/inline_syscall.hpp>
#include <string_encryption.hpp>
#include "sdk/obfuscation/build_obfuscation.hpp"

#include "../../resources/resource.h"

static void load_icon_bundle()
{
    HMODULE self = nullptr;
    if (!::GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&load_icon_bundle),
            &self) || !self)
        return;

    const HRSRC h = ::FindResourceW(self,
        MAKEINTRESOURCEW(IDR_PAWJOB_BUNDLE), RT_RCDATA);
    if (!h) return;
    const HGLOBAL g = ::LoadResource(self, h);
    if (!g) return;
    const void* ptr = ::LockResource(g);
    const DWORD sz  = ::SizeofResource(self, h);
    if (!ptr || !sz) return;

    auto b = unity_bundle::parse(reinterpret_cast<const uint8_t*>(ptr), sz);
    if (!b.ok) return;
    unity_bundle::store::ingest(b);
}

static std::string env_or(const char* name)
{
	char buf[512]{};
	const DWORD n = ::GetEnvironmentVariableA(name, buf, (DWORD)sizeof(buf));
	if (!n || n >= sizeof(buf)) return {};
	return buf;
}

static void wipe_env(const char* name)
{
	::SetEnvironmentVariableA(name, nullptr);
}

static bool require_license(const std::string& cli_key)
{
	std::string key = cli_key;
	if (key.empty() && g_injection_context.license_key[0])
		key = g_injection_context.license_key;
	if (key.empty())
		key = env_or("PS_KEY");
	std::string token = env_or("PS_SESSION");
	wipe_env("PS_KEY");
	wipe_env("PS_SESSION");
	if (key.empty() && token.empty())
		return false;
	return license::gate(key, token);
}

static void ensure_boot_log()
{
	logging::set_up();
}

static HWND ensure_debug_console_shown()
{
	logging::show();
	return ::GetConsoleWindow();
}

static void start_hotkeys()
{
	std::thread([]() {
		bool prev_del = false;
		bool prev_alt_p = false;
		bool console_visible = false;
		while (true) {
			const bool del_down = (::GetAsyncKeyState(VK_DELETE) & 0x8000) != 0;
			if (del_down && !prev_del) {
				memory::cleanup();
				::TerminateProcess(::GetCurrentProcess(), 0);
			}
			prev_del = del_down;

			const bool alt    = (::GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
			const bool p_down = (::GetAsyncKeyState('P') & 0x8000) != 0;
			const bool combo  = alt && p_down;
			if (combo && !prev_alt_p) {
				if (console_visible) {
					logging::hide();
					::debug::set_enabled(false);
				} else {
					ensure_debug_console_shown();
					::debug::set_enabled(true);
				}
				console_visible = !console_visible;
			}
			prev_alt_p = combo;
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}).detach();
}

#if defined(STABLE)
void init()
{
	ensure_boot_log();
	build_obf::init_runtime_seed();
	if (!require_license({}))
		::ExitProcess(1);
	logging::print("license ok");

	::timeBeginPeriod(1);
	::SetThreadPriority(::GetCurrentProcess(), THREAD_PRIORITY_HIGHEST);

	memory::bump_working_set();
	memory::pin_thread_stack();
	memory::create_instance(xs("RustClient.exe"));

	if (!memory::is_initialized())
		::ExitProcess(1);

	start_hotkeys();
	features::physx::init();
	load_icon_bundle();

	logging::print("creating overlay");
	if (!winapp::create_window_instance())
	{
		logging::print("overlay failed");
		return;
	}
	logging::print("overlay live — INSERT for menu");

	winapp::destroy_window_instance();
	isyscall::unload();
	::timeEndPeriod(1);
}

DWORD WINAPI TestThread(LPVOID lpParam)
{
	init();
	return 0;
}

BOOL APIENTRY DllMain(uptr module, i32 reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		if (!reserved)
		{
			return 3;
		}

		std::memcpy(&g_injection_context, reserved, sizeof(injection_context_t));
		if (!g_injection_context.can_access)
			__fastfail(rand() % RAND_MAX);

		HANDLE hThread = CreateThread(NULL, 0, TestThread, NULL, 0, NULL);
		if (hThread)
		{
			CloseHandle(hThread);
		}
	}

	return 1;
}
#else
std::int32_t main(int argc, char** argv)
{
	ensure_boot_log();
	build_obf::init_runtime_seed();

	std::string cli_license_key;
	for (int i = 1; i + 1 < argc; ++i) {
		const char* a = argv[i];
		const char* v = argv[i + 1];
		if (std::strcmp(a, "--key") == 0)
			cli_license_key = v;
	}
	if (!require_license(cli_license_key))
		return 1;
	logging::print("license ok");

	::timeBeginPeriod(1);
	::SetThreadPriority(::GetCurrentProcess(), THREAD_PRIORITY_HIGHEST);

	memory::bump_working_set();
	memory::pin_thread_stack();
	memory::create_instance(xs("RustClient.exe"));

	if (!memory::is_initialized())
		return 1;

	start_hotkeys();
	features::physx::init();
	load_icon_bundle();

	logging::print("creating overlay");
	if (!winapp::create_window_instance())
	{
		logging::print("overlay failed");
		return 0;
	}
	logging::print("overlay live — INSERT for menu");

	winapp::destroy_window_instance();
	isyscall::unload();
	::timeEndPeriod(1);
	return 0;
}
#endif
