#pragma once
#include <Windows.h>

#if defined(STABLE)
#define HIJACK_OVERLAY
#else
#endif

namespace Overlay {
	bool FindOverlay();
	extern HWND overlay;
	extern WNDPROC original_wndproc;
}
