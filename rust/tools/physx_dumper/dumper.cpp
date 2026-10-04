// physx_dumper.dll — inject into Rust (Unity 2022 + PhysX 4.x). Dumps PhysX SDK struct layouts.
// Build: cl /LD /EHsc /std:c++20 /O2 dumper.cpp /link /MACHINE:X64 /OUT:physx_dumper.dll
// EAC blocks CreateRemoteThread+LoadLibrary — use a bypassing loader.

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>

using uptr = std::uintptr_t;
using u8   = std::uint8_t;
using u16  = std::uint16_t;
using u32  = std::uint32_t;
using u64  = std::uint64_t;

// ─── Offsets to probe (match physx.cpp) ───
constexpr uptr OFF_PHYSX_SDK         = 0x21717A8;
constexpr uptr OFF_PX_SCENE_ARRAY    = 0x008;
constexpr uptr OFF_PX_RIGID_ACTORS   = 0x23D8;

enum class GeomType : int { Sphere=0, Plane=1, Capsule=2, Box=3, ConvexMesh=4, TriangleMesh=5, Heightfield=6 };
static const char* geom_name(int t) {
    switch (t) {
    case 0: return "Sphere"; case 1: return "Plane"; case 2: return "Capsule";
    case 3: return "Box";    case 4: return "ConvexMesh"; case 5: return "TriangleMesh";
    case 6: return "Heightfield"; default: return "Unknown";
    }
}

// ─── Output (console + file) ───
static FILE* g_log_file = nullptr;

static void out(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
    if (g_log_file) {
        va_start(ap, fmt);
        vfprintf(g_log_file, fmt, ap);
        va_end(ap);
        fflush(g_log_file);
    }
}

// ─── Safe read (SEH-wrapped) ───
static bool seh_copy(void* dst, const void* src, size_t bytes) {
    __try {
        std::memcpy(dst, src, bytes);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

template<typename T>
static bool safe_read(uptr addr, T& out_val) {
    return seh_copy(&out_val, reinterpret_cast<void*>(addr), sizeof(T));
}

// ─── Hex / annotation dump ───
static void hexdump(const char* name, uptr addr, size_t bytes) {
    out("\n--- %s @ 0x%llx (%zu bytes) ---\n", name, (unsigned long long)addr, bytes);
    if (!addr) { out("  (null)\n"); return; }
    u8 buf[1024]{};
    const size_t take = bytes > sizeof(buf) ? sizeof(buf) : bytes;
    if (!seh_copy(buf, reinterpret_cast<void*>(addr), take)) {
        out("  (unreadable)\n");
        return;
    }
    for (size_t i = 0; i < take; i += 16) {
        out("  +0x%03zx: ", i);
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < take) out("%02x ", buf[i+j]);
            else              out("   ");
        }
        out(" | ");
        for (size_t j = 0; j < 16 && i + j < take; ++j) {
            u8 c = buf[i+j];
            out("%c", (c >= 32 && c < 127) ? c : '.');
        }
        out("\n");
    }
}

// per-qword annotation: PTR? u32 pair? f32 lo? helps spot ptr+size fields.
static void annotate(const char* name, uptr addr, size_t bytes) {
    out("\n--- %s annotated @ 0x%llx ---\n", name, (unsigned long long)addr);
    if (!addr) { out("  (null)\n"); return; }
    u8 buf[512]{};
    const size_t take = bytes > sizeof(buf) ? sizeof(buf) : bytes;
    if (!seh_copy(buf, reinterpret_cast<void*>(addr), take)) {
        out("  (unreadable)\n"); return;
    }
    for (size_t i = 0; i + 8 <= take; i += 8) {
        u64 v = *reinterpret_cast<u64*>(buf + i);
        u32 lo = (u32)v, hi = (u32)(v >> 32);
        out("  +0x%03zx u64=0x%016llx", i, (unsigned long long)v);
        if (v >= 0x10000ULL && v <= 0x00007FFFFFFFFFFFULL) out("  [PTR?]");
        if (lo < 0x100000 && hi < 0x100000) out("  [u32 pair: %u, %u]", lo, hi);
        float f; std::memcpy(&f, &lo, 4);
        if (f != 0.0f && f > -1e6f && f < 1e6f) out("  [f32 lo: %g]", f);
        out("\n");
    }
}

// ─── PhysX walk ───
struct PxArrayRaw { void** data; u32 capacity; u32 size; };

static uptr get_unity_player_base() {
    HMODULE h = GetModuleHandleW(L"UnityPlayer.dll");
    return reinterpret_cast<uptr>(h);
}

// first actor of each geometry type → dump it.
static void dump_actors() {
    const uptr up = get_unity_player_base();
    out("UnityPlayer.dll base: 0x%llx\n", (unsigned long long)up);
    if (!up) { out("FATAL: UnityPlayer.dll not loaded\n"); return; }

    uptr sdk = 0;
    if (!safe_read(up + OFF_PHYSX_SDK, sdk) || !sdk) {
        out("FATAL: SDK ptr null/unreadable at base+0x%llx\n", (unsigned long long)OFF_PHYSX_SDK);
        return;
    }
    out("PhysX SDK ptr: 0x%llx\n", (unsigned long long)sdk);

    PxArrayRaw scene_arr{};
    if (!safe_read(sdk + OFF_PX_SCENE_ARRAY, scene_arr) || !scene_arr.data) {
        out("FATAL: scene_array unreadable\n"); return;
    }
    out("scene array: data=%p capacity=%u size=%u\n", scene_arr.data, scene_arr.capacity, scene_arr.size);

    bool found_box=false, found_sphere=false, found_capsule=false;
    bool found_convex=false, found_mesh=false, found_heightfield=false, found_plane=false;

    for (u32 si = 0; si < scene_arr.size; ++si) {
        u64 scene_ptr = 0;
        if (!safe_read(reinterpret_cast<uptr>(scene_arr.data) + si * sizeof(u64), scene_ptr)) continue;
        if (!scene_ptr) continue;

        PxArrayRaw actor_arr{};
        if (!safe_read(scene_ptr + OFF_PX_RIGID_ACTORS, actor_arr)) continue;
        if (!actor_arr.data || actor_arr.size == 0 || actor_arr.size > 2'000'000) continue;

        out("\n========== Scene 0x%llx: %u actors ==========\n",
            (unsigned long long)scene_ptr, actor_arr.size);

        for (u32 ai = 0; ai < actor_arr.size; ++ai) {
            u64 actor_ptr = 0;
            if (!safe_read(reinterpret_cast<uptr>(actor_arr.data) + ai * sizeof(u64), actor_ptr)) continue;
            if (!actor_ptr) continue;

            // RigidDynamic=6, RigidStatic=7
            u16 type_field = 0;
            if (!safe_read(actor_ptr + 0x08, type_field)) continue;
            if (type_field != 6 && type_field != 7) continue;

            // c_physx layout: NpShapeManager at +0x28, PtrTable.m_single is first field.
            u64 shape_single = 0;
            const uptr shape_mgr_off = 0x28;
            if (!safe_read(actor_ptr + shape_mgr_off, shape_single)) continue;
            if (!shape_single) continue;

            // GeometryUnion type field lives at NpShape + 0x98.
            constexpr uptr kGeomTypeOffset = 0x98;
            u32 type_u32 = 0;
            if (!safe_read(shape_single + kGeomTypeOffset, type_u32)) continue;
            if (type_u32 > 6) continue;
            const int  geom_type = (int)type_u32;
            const uptr geom_blob_addr = shape_single + kGeomTypeOffset;

            const bool first_box        = (geom_type == 3 && !found_box);
            const bool first_sphere     = (geom_type == 0 && !found_sphere);
            const bool first_plane      = (geom_type == 1 && !found_plane);
            const bool first_capsule    = (geom_type == 2 && !found_capsule);
            const bool first_convex     = (geom_type == 4 && !found_convex);
            const bool first_mesh       = (geom_type == 5 && !found_mesh);
            const bool first_heightfield= (geom_type == 6 && !found_heightfield);

            if (!(first_box || first_sphere || first_plane || first_capsule ||
                  first_convex || first_mesh || first_heightfield))
                continue;

            out("\n========== %s actor 0x%llx ==========\n",
                geom_name(geom_type), (unsigned long long)actor_ptr);
            out("  actor_type=%u (Dynamic=6, Static=7)\n", type_field);
            out("  shape_ptr=0x%llx, geom_type_at_offset=0x%llx\n",
                (unsigned long long)shape_single,
                (unsigned long long)(geom_blob_addr - shape_single));

            hexdump("PxActor (first 256 bytes)", actor_ptr, 256);
            hexdump("NpShape (first 512 bytes)", shape_single, 512);
            hexdump("Geometry blob (128 bytes from +0x98)", geom_blob_addr, 128);
            annotate("Geometry blob annotated", geom_blob_addr, 128);

            // mesh-bearing geometries hold a ptr to the mesh struct; probe offsets and dump it.
            if (geom_type == 4 || geom_type == 5 || geom_type == 6) {
                out("\n  Probing for mesh/heightfield pointer field in geometry blob:\n");
                for (uptr poff = 0; poff < 0x60; poff += 8) {
                    u64 candidate = 0;
                    if (!safe_read(geom_blob_addr + poff, candidate)) continue;
                    if (candidate >= 0x10000ULL && candidate <= 0x00007FFFFFFFFFFFULL) {
                        // deref and check vtable-ish first qword.
                        u64 maybe_vtable = 0;
                        if (safe_read(candidate, maybe_vtable) &&
                            maybe_vtable >= 0x10000ULL && maybe_vtable <= 0x00007FFFFFFFFFFFULL) {
                            out("    +0x%02zx -> 0x%llx [PROBABLY MESH/HF PTR]\n",
                                (size_t)poff, (unsigned long long)candidate);
                            char name_buf[64];
                            std::snprintf(name_buf, sizeof(name_buf),
                                geom_type == 4 ? "PxConvexMesh @ +0x%02zx" :
                                geom_type == 5 ? "PxTriangleMesh @ +0x%02zx" :
                                                  "PxHeightField @ +0x%02zx",
                                (size_t)poff);
                            hexdump(name_buf, candidate, 256);
                            annotate(name_buf, candidate, 128);
                        }
                    }
                }
            }

            if (first_box)         found_box = true;
            if (first_sphere)      found_sphere = true;
            if (first_plane)       found_plane = true;
            if (first_capsule)     found_capsule = true;
            if (first_convex)      found_convex = true;
            if (first_mesh)        found_mesh = true;
            if (first_heightfield) found_heightfield = true;

            if (found_convex && found_heightfield && found_mesh &&
                found_box && found_sphere && found_capsule) {
                out("\n[done] all geometry types sampled\n");
                return;
            }
        }
    }

    out("\n[done] finished walk; not all types were sampled\n");
    out("  found: box=%d sphere=%d capsule=%d plane=%d convex=%d mesh=%d heightfield=%d\n",
        found_box, found_sphere, found_capsule, found_plane,
        found_convex, found_mesh, found_heightfield);
}

DWORD WINAPI dumper_thread(LPVOID) {
    AllocConsole();
    FILE* tmp;
    freopen_s(&tmp, "CONOUT$", "w", stdout);
    SetConsoleTitleA("PhysX Struct Dumper");
    fopen_s(&g_log_file, "C:\\physx_dump.txt", "w");

    out("PhysX Struct Dumper — waiting 5 s for game to settle...\n");
    Sleep(5000);

    out("\n========== DUMP START ==========\n");
    dump_actors();
    out("\n========== DUMP END ==========\n");

    if (g_log_file) fflush(g_log_file);
    out("\nWrote C:\\physx_dump.txt — share this with the AI to add the missing shape handlers.\n");
    out("Press any key to dismiss (or close the console).\n");
    std::getchar();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(nullptr, 0, dumper_thread, nullptr, 0, nullptr);
    }
    return TRUE;
}
