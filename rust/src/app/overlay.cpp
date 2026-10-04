#include "overlay.hpp"
#include <chrono>
#include <utils/debug.hpp>
#include <string_encryption.hpp>
#include <thread>
#include "../utils/vmp.hpp"

namespace Overlay {
	HWND overlay = nullptr;
	WNDPROC original_wndproc = nullptr;

	bool FindOverlay() {
		DBG("Searching for overlay");

		while (!overlay) {
			overlay = FindWindowA(xs("Chrome_WidgetWin_1"), xs("Discord Overlay"));
			if (overlay) break;

			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		return true;
	}
}
