#pragma once
#include <atomic>
#include <thread>

struct injection_context_t
{
	char username[256];
	char session_id[256];
	char product[256];
	bool can_access;
	std::uint8_t* avatar_data;
	std::size_t avatar_size;
	std::uint16_t hypercall_primary_key;
	std::uint8_t hypercall_secondary_key;
	char discord_id[32];
	char license_key[64];
};

inline injection_context_t g_injection_context{};

inline std::atomic<bool> g_should_terminate = false;
inline bool g_menu_3d_enabled = false;
inline std::atomic<bool> g_should_refresh_cache = false;