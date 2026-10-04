#pragma once

#include <cstdint>
#include <cstdio>
#include <windows.h>

namespace titles
{
	inline std::uint32_t mix()
	{
		LARGE_INTEGER qpc{};
		::QueryPerformanceCounter(&qpc);
		std::uint32_t x = (std::uint32_t)qpc.LowPart ^ (std::uint32_t)::GetTickCount() ^ (std::uint32_t)::GetCurrentProcessId();
		x ^= x << 13;
		x ^= x >> 17;
		x ^= x << 5;
		return x ? x : 0xA5A5A5A5u;
	}

	inline void ascii(char* out, int n)
	{
		static const char* k[] = {
			"RuntimeBroker", "dllhost", "sihost", "taskhostw", "conhost",
			"fontdrvhost", "SearchHost", "TextInputHost", "ctfmon", "WmiPrvSE",
			"svchost", "backgroundTaskHost", "ApplicationFrameHost"
		};
		if (!out || n < 8) return;
		const std::uint32_t s = mix();
		const char* base = k[s % (sizeof(k) / sizeof(k[0]))];
		_snprintf_s(out, n, _TRUNCATE, "%s", base);
	}

	inline void wide(wchar_t* out, int n)
	{
		char a[64]{};
		ascii(a, (int)sizeof(a));
		if (!out || n <= 0) return;
		::MultiByteToWideChar(CP_ACP, 0, a, -1, out, n);
	}
}
