#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <cstdint>

// Kernel side bakes these at compile time per-session. Userland is shipped
// once and learns them at runtime — either from the bundle's PWJBCMS1
// trailer (preferred) or from --cmd-* args. Defaults match an unmutated
// build of the Astrum driver from auth-server/driver_build.
namespace astrum_cmd {
	inline uint32_t null  = 0u;
	inline uint32_t read  = 1u;
	inline uint32_t write = 2u;
	inline uint32_t query = 3u;
	inline uint32_t alloc = 4u;
	inline uint32_t token = 0u;

	inline void set_from(uint32_t n, uint32_t r, uint32_t w, uint32_t q) {
		null = n; read = r; write = w; query = q;
	}
	inline void set_token(uint32_t t) { token = t; }
}

// MUST match kAstrumRequestMagic in kernelmode transport_types.hpp.
constexpr uint64_t kAstrumRequestMagic = 0xA5724DC0DEA11Eull;

enum class e_command : uint32_t {
	null       = 0,
	mem_read   = 1,
	mem_write  = 2,
	proc_query = 3,
	mem_alloc  = 4,
};

class c_request {
public:
	c_request(e_command command, void* data, uint64_t length) {
		m_magic = kAstrumRequestMagic;
		m_data = data;
		m_length = length;
		m_command = command;
		m_token = astrum_cmd::token;
	}

	~c_request() = default;

	template <typename T>
	T* get() const {
		if (!m_data || !m_length)
			return nullptr;
		if (sizeof(T) != m_length)
			return nullptr;
		return (T*)m_data;
	}

	e_command get_command() const { return m_command; }
	uint32_t get_token() const { return m_token; }
	uint64_t get_magic() const { return m_magic; }

private:
	uint64_t m_magic = 0; // MUST be first — kernel hijack reads this
	void* m_data = nullptr;
	uint64_t m_length = 0;
	e_command m_command = e_command::null;
	uint32_t m_token = 0;
};

typedef struct _mem_copy_request {
	uint32_t process_id = 0;
	uint64_t source = 0;
	void* dest = nullptr;
	uint64_t size = 0;
} mem_copy_request, *pmem_copy_request;

typedef struct _proc_info_request {
	uint32_t process_id = 0;
	void* peb = nullptr;
	void* base_address = nullptr;
	uint64_t cr3 = 0;
} proc_info_request, *pproc_info_request;

typedef struct _mem_alloc_request {
	uint32_t process_id = 0;
	uint64_t size = 0;
	void* address = nullptr;
} mem_alloc_request, *pmem_alloc_request;
