#pragma once
#include <sdk/globals.hpp>

namespace gchandle {
	enum gchandle_type {
		HANDLE_WEAK,
		HANDLE_WEAK_TRACK,
		HANDLE_NORMAL,
		HANDLE_PINNED
	};

	typedef struct
	{
		u32* bitmap;
		void** entries;
		u32 size;
		u8 type;
		unsigned int slot_hint : 24;
		u16* domain_ids;
	} handle_data;

	static inline gchandle_type get_handle_type(u32 gchandle) {
		return static_cast<gchandle_type>((gchandle & 7) - 1);
	}

	static inline u32 get_handle_slot(u32 gchandle) {
		return gchandle >> 3;
	}

	uptr get_target(uintptr_t encrypted);
}