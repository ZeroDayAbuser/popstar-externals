#pragma once

#define _CRT_SECURE_NO_WARNINGS

using vi = void;

#include <Windows.h>
#include <winternl.h>

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cstdarg>

#include <iostream>
#include <iomanip>
#include <string>
#include <string_view>
#include <sstream>

#include <vector>
#include <array>
#include <deque>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <variant>
#include <utility>

#include <cmath>
#include <numbers>
#include <limits>

#include <ctime>
#include <chrono>

#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>

#include <fstream>

#include <cassert>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/scalar_constants.hpp>

#include <dependencies/imgui/imgui.h>
#include <dependencies/imgui/imgui_internal.h>
#include <dependencies/imgui/imgui_impl_dx11.h>
#include <dependencies/imgui/imgui_impl_win32.h>
#include <dependencies/imgui/imgui_freetype.h>


#include <src/utility/logger.cuh>
#include <src/driver/driver.cuh>
#include <src/sdk/offsets/offsets.cuh>
#include <src/sdk/data.cuh>
#include <src/sdk/math.cuh>
#include <src/sdk/classes/classes.cuh>
#include <src/sdk/cache/cache.cuh>

template<typename T>
struct li_fn_wrapper {
	T* f;
	li_fn_wrapper( T* _f ) : f( _f ) { }
	template<typename... Args>
	auto operator()( Args... args ) { return f( args... ); }
	li_fn_wrapper& safe_cached( ) { return *this; }
	li_fn_wrapper& cached( ) { return *this; }
	li_fn_wrapper& safe( ) { return *this; }
};
#define LI_FN(f) li_fn_wrapper(&f)

#define HASH_STR( x ) x

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")


#include <dependencies/overlay/overlay.h>