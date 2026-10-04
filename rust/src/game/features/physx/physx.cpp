#include "physx.hpp"

#include <memory/memory.hpp>
#include <sdk/offsets.hpp>
#include <string_encryption.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <numeric>
#include <optional>
#include <thread>
#include <chrono>
#include <print>
#include <utils/debug.hpp>
#include <unordered_map>
#define ADD_LOG(strfmt) do { \
	auto _m = std::format("physx: L{} - {}", __LINE__, strfmt); \
	if (debug_info.empty() || debug_info.back() != _m) debug_info.push_back(std::move(_m)); \
} while (0)
namespace features::physx
{
	using uptr = std::uintptr_t;
	using u8   = std::uint8_t;
	using u16  = std::uint16_t;
	using u32  = std::uint32_t;
	using i16  = std::int16_t;
	using i32  = std::int32_t;
	using u64  = std::uint64_t;

	static std::atomic<uptr> g_off_physx_sdk{ 0 };
	constexpr uptr OFF_PX_SCENE_ARRAY      = 0x008;
	constexpr uptr OFF_PX_RIGID_ACTORS     = 0x23D8;
	constexpr uptr OFF_PX_BODY_STREAM_POSE = 0x0B0;

	enum class PxConcreteType : u16 {
		Undefined = 0,
		Heightfield, ConvexMesh, TriangleMeshBvh33, TriangleMeshBvh34, ClothFabric,
		RigidDynamic, RigidStatic, Shape, Material, Constraint, Cloth,
		ParticleSystem, ParticleFluid, Aggregate, Articulation, ArticulationLink,
		ArticulationJoint, PruningStructure,
	};

	enum class PxGeometryType : i32 {
		Sphere = 0, Plane, Capsule, Box, ConvexMesh, TriangleMesh, Heightfield,
		Count, Invalid = -1,
	};
	struct PxBounds3Raw { glm::vec3 mn, mx; };
	struct AABB {
		glm::vec3 mn{  FLT_MAX,  FLT_MAX,  FLT_MAX };
		glm::vec3 mx{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

		void expand(const glm::vec3& p) {
			mn = glm::min(mn, p);
			mx = glm::max(mx, p);
		}

		bool intersects(const glm::vec3& o, const glm::vec3& d, float& tmin, float& tmax) const {
			tmin = 0.0f; tmax = FLT_MAX;
			for (int i = 0; i < 3; ++i) {
				const float dd = d[i];
				const float invD = (std::abs(dd) < 1e-9f) ? 1e9f : 1.0f / dd;
				float t0 = (mn[i] - o[i]) * invD;
				float t1 = (mx[i] - o[i]) * invD;
				if (invD < 0.0f) std::swap(t0, t1);
				tmin = t0 > tmin ? t0 : tmin;
				tmax = t1 < tmax ? t1 : tmax;
				if (tmax <= tmin) return false;
			}
			return true;
		}

		int longest_axis() const {
			const glm::vec3 e = mx - mn;
			if (e.x > e.y && e.x > e.z) return 0;
			if (e.y > e.z)              return 1;
			return 2;
		}
	};

	struct Triangle { glm::vec3 v0, v1, v2;
		glm::vec3 center() const { return (v0 + v1 + v2) / 3.0f; }
	};

	static inline glm::vec3 quat_rotate(const glm::vec4& q, const glm::vec3& v) {
		const glm::vec3 qv(q.x, q.y, q.z);
		const glm::vec3 t = 2.0f * glm::cross(qv, v);
		return v + q.w * t + glm::cross(qv, t);
	}

	struct PxTransform {
		glm::vec4 rot{ 0, 0, 0, 1 };
		glm::vec3 pos{ 0, 0, 0 };

		glm::vec3 transform_point(const glm::vec3& p) const { return quat_rotate(rot, p) + pos; }
	};

	struct Mat3 {
		glm::vec3 c0{1,0,0}, c1{0,1,0}, c2{0,0,1};
		glm::vec3 transform(const glm::vec3& v) const { return c0*v.x + c1*v.y + c2*v.z; }
		Mat3 operator*(const Mat3& o) const { return { transform(o.c0), transform(o.c1), transform(o.c2) }; }
		static Mat3 from_quat(const glm::vec4& q) {
			Mat3 m;
			const float x=q.x, y=q.y, z=q.z, w=q.w;
			const float x2=x+x, y2=y+y, z2=z+z;
			const float xx=x2*x, yy=y2*y, zz=z2*z;
			const float xy=x2*y, xz=x2*z, xw=x2*w;
			const float yz=y2*z, yw=y2*w, zw=z2*w;
			m.c0 = { 1.0f-yy-zz,  xy+zw,      xz-yw     };
			m.c1 = { xy-zw,       1.0f-xx-zz, yz+xw     };
			m.c2 = { xz+yw,       yz-xw,      1.0f-xx-yy};
			return m;
		}
		Mat3 transpose() const {
			Mat3 m;
			m.c0 = { c0.x, c1.x, c2.x };
			m.c1 = { c0.y, c1.y, c2.y };
			m.c2 = { c0.z, c1.z, c2.z };
			return m;
		}
	};

	struct PxMeshScale {
		glm::vec3 scale{ 1,1,1 };   // 12 bytes, no pad
		glm::vec4 rot{ 0,0,0,1 };   // 16 bytes, 4-aligned
		Mat3 to_mat33() const {
			Mat3 r  = Mat3::from_quat(rot);
			Mat3 rt = r.transpose();
			rt.c0 *= scale.x;
			rt.c1 *= scale.y;
			rt.c2 *= scale.z;
			return rt * r;
		}
	};

	struct PxActor {
		std::byte       _pad[0x8]{};
		PxConcreteType  type{};
		u16             base_flags{};
		u64             user_data{};
	};

	template <typename T>
	struct PxArray {
		T**  data{};
		u32  capacity{};
		u32  size{};
	};

	struct PtrTable {
		union { void* m_single; void** m_list; };
		u16  count;
		bool owns_memory;
		bool buffer_used;
	};

	struct PruningStructure {
		std::byte       _pad[0x8]{};
		PxConcreteType  type{};
		u16             base_flags{};
		u32             nb_nodes[2]{};
		void*           aabb_tree_nodes[2]{};
		u32             nb_objects[2]{};
		u32*            aabb_tree_indices[2]{};
		u32             nb_actors{};
		PxActor**       actors{};
		bool            valid{};
	};

	struct NpShapeManager {
		PtrTable          shapes{};
		PtrTable          scene_query_data{};
		PruningStructure* pruning{};
	};

	struct PxRigidActor : PxActor {
		const char*     name{};
		void*           connector_array{};
		NpShapeManager  shape_manager{};
		u32             index{};
	};

	struct PxGeometryBase { PxGeometryType type{}; };

	struct GeometryUnion {
		union {
			void* alignment{};
			u8    box[16];
			u8    sphere[8];
			u8    capsule[12];
			u8    plane[4];
			u8    convex[64];
			u8    mesh[80];
			u8    heightfield[56];
			u8    invalid[4];
		} g{};
		PxGeometryType get_type() const {
			const auto& base = reinterpret_cast<const PxGeometryBase&>(g);
			if (base.type < PxGeometryType::Sphere || base.type > PxGeometryType::Heightfield) return PxGeometryType::Invalid;
			return base.type;
		}
	};

	struct PxShapeCore {
		alignas(16) PxTransform transform{};
		float       contact_offset{};
		u8          shape_flags{};
		u8          owns_material_idx_memory{};
		u16         material_index{};
		GeometryUnion geometry{};
	};

	struct ShapeCore {
		u32   query_filter_data[4]{};
		u32   simulation_filter_data[4]{};
		alignas(16) PxShapeCore core{};
		float rest_offset{};
	};

	struct Shape {
		uptr      scene{};
		u32       control_state{};
		uptr      stream_ptr{};
		ShapeCore shape_core{};
	};

	struct NpShape : PxActor {
		uptr            ref_countable{};
		i32             ref_count{};
		PxRigidActor*   actor{};
		Shape           shape{};
		const char*     name{};
		i32             exclusive_and_actor_count{};

		PxGeometryType get_geometry_type() const {
			return shape.shape_core.core.geometry.get_type();
		}
	};

	struct PxRigidCore {
		alignas(16) PxTransform body_to_world{};
		u8                      flags{};
		u8                      idt_body_to_actor{};
		u16                     solver_iteration_counts{};
	};

	struct PxsBodyCore : PxRigidCore {
		alignas(16) PxTransform body_to_actor{};
		float                   ccd_advance_coefficient{};
		glm::vec3               linear_velocity{};
		float                   max_pen_bias{};
		glm::vec3               angular_velocity{};
		float                   contact_report_threshold{};
		float                   max_angular_velocity_sq{};
		float                   max_linear_velocity_sq{};
		float                   linear_damping{};
		float                   angular_damping{};
		glm::vec3               inverse_inertia{};
		float                   inverse_mass{};
		float                   max_contact_impulse{};
		float                   sleep_threshold{};
		float                   freeze_threshold{};
		float                   wake_counter{};
		float                   solver_wake_counter{};
		u32                     num_counted_interactions{};
		u32                     num_body_interactions{};
		u16                     is_fast_moving{};
		u16                     lock_flags{};
	};

	struct BodyCore { std::byte _pad[0x10]{}; alignas(16) PxsBodyCore core{}; void* sim_state_data{}; };

	struct Body {
		u64           scene{};
		u64           control_state{};
		u64           stream_ptr{};
		BodyCore      body_core{};
		alignas(16) PxTransform buffered_body2_world{};
		glm::vec3     buffered_lin_velocity{};
		glm::vec3     buffered_ang_velocity{};
		float         buffered_wake_counter{};
		u32           buffered_is_sleeping{};
		u32           body_buffer_flags{};
	};

	struct NpRigidStatic : PxRigidActor {
		Body body{};

		PxTransform get_global_pose() const {
			const u64 state = body.control_state & 0x28;
			if (state != 0) {
				const uptr loc = memory::read<uptr>(body.stream_ptr + OFF_PX_BODY_STREAM_POSE);
				if (loc) return memory::read<PxTransform>(loc);
			}
			return body.body_core.core.body_to_world;
		}
	};

	struct NpPhysics {
		std::byte             _pad[OFF_PX_SCENE_ARRAY]{};
		PxArray<void*>        scene_array{};
	};

	struct PxBoxGeom : PxGeometryBase     { glm::vec3 half_extents{}; };
	struct PxSphereGeom : PxGeometryBase  { float radius{}; };
	struct PxCapsuleGeom : PxGeometryBase { float radius{}; float half_height{}; };
	struct PxPlaneGeom : PxGeometryBase   { float _unused{}; };
	struct PxMeshGeom : PxGeometryBase {
		PxMeshScale scale{};         // 0x04 .. 0x20
		void* triangle_mesh{}; // 0x20
		u8          mesh_flags{};    // 0x28
		std::byte   _pad[3]{};       // 0x29..0x2B
	};
	static_assert(sizeof(PxMeshGeom) == 0x30);
	static_assert(offsetof(PxMeshGeom, triangle_mesh) == 0x20);

	struct PxConvexGeom : PxGeometryBase {
		PxMeshScale scale{};       // 0x04 .. 0x20
		void* convex_mesh{}; // 0x20
		u8          mesh_flags{};  // 0x28
		std::byte   _pad[3]{};     // 0x29..0x2B
	};
	static_assert(sizeof(PxConvexGeom) == 0x30);
	static_assert(offsetof(PxConvexGeom, convex_mesh) == 0x20);
	struct PxHeightFieldGeom : PxGeometryBase {
		void*  height_field{};
		float  height_scale{};
		float  row_scale{};
		float  column_scale{};
		u8     hf_flags{};
		std::byte _pad[3]{};
	};

	struct HeightField {
		std::byte    _prefix[0x20]{};      // pad+type+flags+vfptr+refcount
		glm::vec3    aabb_center{};        // 0x20
		glm::vec3    aabb_extents{};       // 0x2C
		u32          nb_rows{};            // 0x38
		u32          nb_columns{};         // 0x3C
		float        row_limit{};          // 0x40
		float        column_limit{};       // 0x44
		float        _floatpad{};          // 0x48
		std::byte    _align_pad[4]{};      // 0x4C — pad samples ptr to 8-align
		void*        samples{};            // 0x50
	};

	struct HeightFieldSample {
		i16 height{};
		u8  material_idx_0{};
		u8  material_idx_1{};
	};
	static_assert(sizeof(HeightFieldSample) == 4, "HeightFieldSample must be 4 bytes");

	struct TriangleMeshData {
		std::byte       _pad[0x8]{};
		PxConcreteType  type{};
		u16             base_flags{};
		uptr            ref_countable_vfptr{};
		i64             ref_count{};
		u32             nb_vertices{};
		u32             nb_triangles{};
		glm::vec3*      vertices{};
		void*           triangles{};
		struct { glm::vec3 center, extents; } aabb{};
		u8*             extra_trig_data{};
		float           geom_epsilon{};
		u8              flags{};
		u16*            material_indices{};
	};

	struct BVHNode {
		AABB                       bounds;
		std::vector<std::size_t>   tri_indices;
		std::unique_ptr<BVHNode>   left, right;
	};

	struct Actor {
		std::vector<Triangle>      triangles;
		PxTransform                transform;
		PxGeometryType             type{};
		AABB                       bounds;
		std::unique_ptr<BVHNode>   bvh;
	};

	static std::vector<Triangle> tri_box(const PxTransform& t, const glm::vec3& half) {
		std::array<glm::vec3, 8> c;
		int idx = 0;
		for (int x : { -1, 1 }) for (int y : { -1, 1 }) for (int z : { -1, 1 })
			c[idx++] = t.transform_point({ x*half.x, y*half.y, z*half.z });

		constexpr std::array<std::array<int, 3>, 12> faces = { {
			{0,1,2},{1,3,2}, {4,6,5},{5,6,7}, {0,2,4},{2,6,4},
			{1,5,3},{3,5,7}, {0,4,1},{1,4,5}, {2,3,6},{3,7,6}
		} };
		std::vector<Triangle> out; out.reserve(12);
		for (const auto& f : faces) {
			const Triangle tri{ c[f[0]], c[f[1]], c[f[2]] };
			const glm::vec3 n = glm::cross(tri.v1 - tri.v0, tri.v2 - tri.v0);
			if (glm::dot(n, n) < 1e-6f) continue;
			out.push_back(tri);
		}
		return out;
	}

	static std::vector<Triangle> tri_sphere(const PxTransform& t, float radius) {
		const float p = (1.0f + std::sqrt(5.0f)) * 0.5f;
		const float n = 1.0f / std::sqrt(1.0f + p*p);
		const float a = n;
		const float b = p * n;
		const std::array<glm::vec3, 12> v = {
			glm::vec3{-a, b, 0}, { a, b, 0}, {-a,-b, 0}, { a,-b, 0},
			{ 0,-a, b}, { 0, a, b}, { 0,-a,-b}, { 0, a,-b},
			{ b, 0,-a}, { b, 0, a}, {-b, 0,-a}, {-b, 0, a},
		};
		constexpr std::array<std::array<int,3>, 20> f = { {
			{0,11,5},{0,5,1},{0,1,7},{0,7,10},{0,10,11},
			{1,5,9},{5,11,4},{11,10,2},{10,7,6},{7,1,8},
			{3,9,4},{3,4,2},{3,2,6},{3,6,8},{3,8,9},
			{4,9,5},{2,4,11},{6,2,10},{8,6,7},{9,8,1},
		} };
		std::vector<Triangle> out; out.reserve(20);
		for (const auto& tri : f) {
			out.push_back({
				t.transform_point(v[tri[0]] * radius),
				t.transform_point(v[tri[1]] * radius),
				t.transform_point(v[tri[2]] * radius),
			});
		}
		return out;
	}

	static std::vector<Triangle> tri_capsule(const PxTransform& t, float radius, float half_h) {
		constexpr int SEG = 8;
		std::vector<Triangle> out; out.reserve(SEG * 4 + 16);
		for (int i = 0; i < SEG; ++i) {
			const float a0 = 2.0f * 3.14159265f * (i      / float(SEG));
			const float a1 = 2.0f * 3.14159265f * ((i+1)  / float(SEG));
			const glm::vec3 p00 = { std::cos(a0)*radius, -half_h, std::sin(a0)*radius };
			const glm::vec3 p10 = { std::cos(a1)*radius, -half_h, std::sin(a1)*radius };
			const glm::vec3 p01 = { std::cos(a0)*radius,  half_h, std::sin(a0)*radius };
			const glm::vec3 p11 = { std::cos(a1)*radius,  half_h, std::sin(a1)*radius };
			out.push_back({ t.transform_point(p00), t.transform_point(p10), t.transform_point(p11) });
			out.push_back({ t.transform_point(p00), t.transform_point(p11), t.transform_point(p01) });
		}
		const glm::vec3 top_pole{ 0,  half_h + radius, 0 };
		const glm::vec3 bot_pole{ 0, -half_h - radius, 0 };
		for (int i = 0; i < SEG; ++i) {
			const float a0 = 2.0f * 3.14159265f * (i      / float(SEG));
			const float a1 = 2.0f * 3.14159265f * ((i+1)  / float(SEG));
			const glm::vec3 p01 = { std::cos(a0)*radius,  half_h, std::sin(a0)*radius };
			const glm::vec3 p11 = { std::cos(a1)*radius,  half_h, std::sin(a1)*radius };
			const glm::vec3 p00 = { std::cos(a0)*radius, -half_h, std::sin(a0)*radius };
			const glm::vec3 p10 = { std::cos(a1)*radius, -half_h, std::sin(a1)*radius };
			out.push_back({ t.transform_point(p01), t.transform_point(top_pole), t.transform_point(p11) });
			out.push_back({ t.transform_point(p00), t.transform_point(p10),      t.transform_point(bot_pole) });
		}
		return out;
	}

	static std::vector<Triangle> tri_plane(const PxTransform& t) {
		constexpr float K = 200.0f;
		const glm::vec3 a = t.transform_point({ -K, 0, -K });
		const glm::vec3 b = t.transform_point({  K, 0, -K });
		const glm::vec3 c = t.transform_point({  K, 0,  K });
		const glm::vec3 d = t.transform_point({ -K, 0,  K });
		return { { a, b, c }, { a, c, d } };
	}

	static std::vector<Triangle> tri_heightfield(const PxTransform& t, const PxHeightFieldGeom& g) {
		std::vector<Triangle> out;
		if (!g.height_field) return out;

		const auto hf = memory::read<HeightField>(reinterpret_cast<uptr>(g.height_field));
		const u32 rows = hf.nb_rows;
		const u32 cols = hf.nb_columns;
		if (rows < 2 || cols < 2 || rows > 16385 || cols > 16385 || !hf.samples) {
			static bool s_dumped = false;
			if (!s_dumped) {
				s_dumped = true;
				const uptr hf_addr = reinterpret_cast<uptr>(g.height_field);
				u8 raw[0x40] = {};
				memory::read_memory_raw(hf_addr, raw, sizeof(raw));
				DBG("[physx] tri_heightfield rejected (rows={} cols={} samples={:#x}). hf_addr={:#x}",
					rows, cols, reinterpret_cast<uptr>(hf.samples), hf_addr);
				std::print("[physx] hf raw bytes:");
				for (size_t i = 0; i < sizeof(raw); ++i) {
					if (i % 16 == 0) std::print("\n  +{:#04x}:", (unsigned)i);
					std::print(" {:02x}", raw[i]);
				}
				DBG("");
				for (uptr off : { (uptr)0x10, (uptr)0x18, (uptr)0x20, (uptr)0x28, (uptr)0x30 }) {
					const u32 r_try = *(u32*)(raw + off);
					const u32 c_try = *(u32*)(raw + off + 4);
					DBG("[physx] hf probe +{:#04x}: rows={} cols={}{}",
						(unsigned)off, r_try, c_try,
						(r_try >= 64 && r_try <= 16385 && c_try >= 64 && c_try <= 16385)
							? " [LOOKS PLAUSIBLE]" : "");
				}
			}
			return out;
		}

		u32 step = 1;
		while (static_cast<u64>(rows / step) * (cols / step) > 500000 && step < 64) step *= 2;

		std::unordered_map<u32, std::vector<HeightFieldSample>> row_cache;
		for (u32 r = 0; r < rows; r += step) {
			const uptr row_addr = reinterpret_cast<uptr>(hf.samples)
				+ static_cast<uptr>(r) * cols * sizeof(HeightFieldSample);
			std::vector<HeightFieldSample> buf(cols);
			if (!memory::read_memory_raw(row_addr, buf.data(), cols * sizeof(HeightFieldSample))) return out;
			row_cache[r] = std::move(buf);
		}
		if (row_cache.empty()) return out;

		auto get_world = [&](u32 r, u32 c) -> glm::vec3 {
			float h = 0.0f;
			auto it = row_cache.find(r);
			if (it != row_cache.end() && c < it->second.size())
				h = static_cast<float>(it->second[c].height);
			const glm::vec3 local{
				static_cast<float>(r) * g.row_scale,
				h * g.height_scale,
				static_cast<float>(c) * g.column_scale
			};
			return t.transform_point(local);
		};

		const u32 lod_rows = (rows + step - 1) / step;
		const u32 lod_cols = (cols + step - 1) / step;
		out.reserve(static_cast<size_t>(lod_rows - 1) * (lod_cols - 1) * 2);

		for (u32 r = 0; r + step < rows; r += step) {
			for (u32 c = 0; c + step < cols; c += step) {
				const glm::vec3 v00 = get_world(r,         c);
				const glm::vec3 v10 = get_world(r,         c + step);
				const glm::vec3 v01 = get_world(r + step,  c);
				const glm::vec3 v11 = get_world(r + step,  c + step);

				const glm::vec3 n1 = glm::cross(v10 - v00, v11 - v00);
				if (glm::dot(n1, n1) >= 1e-6f) out.push_back({ v00, v10, v11 });

				const glm::vec3 n2 = glm::cross(v11 - v00, v01 - v00);
				if (glm::dot(n2, n2) >= 1e-6f) out.push_back({ v00, v11, v01 });
			}
		}
		return out;
	}

	static std::vector<Triangle> tri_convex(const PxTransform& t, const PxConvexGeom& g) {
		std::vector<Triangle> out;
		if (!g.convex_mesh) return out;

		struct PxBounds3 { glm::vec3 mn, mx; };
		struct CenterExtents { glm::vec3 center, extents; };

		/*auto try_bounds = [&](uptr off) -> std::optional<PxBounds3> {
			CenterExtents ce{};
			if (!memory::read_memory_raw(reinterpret_cast<uptr>(g.convex_mesh) + off, &ce, sizeof(ce)))
				return std::nullopt;
			if (!std::isfinite(ce.center.x) || !std::isfinite(ce.center.y) || !std::isfinite(ce.center.z))
				return std::nullopt;
			if (!std::isfinite(ce.extents.x) || !std::isfinite(ce.extents.y) || !std::isfinite(ce.extents.z))
				return std::nullopt;
			if (ce.extents.x <= 0.001f || ce.extents.y <= 0.001f || ce.extents.z <= 0.001f)
				return std::nullopt;
			if (ce.extents.x > 100.0f || ce.extents.y > 100.0f || ce.extents.z > 100.0f)
				return std::nullopt;
			PxBounds3 b{ ce.center - ce.extents, ce.center + ce.extents };
			return b;
		};*/
		auto try_bounds = [&](uptr off) -> std::optional<PxBounds3> {
			PxBounds3Raw b{};
			if (!memory::read_memory_raw(reinterpret_cast<uptr>(g.convex_mesh) + off, &b, sizeof(b)))
				return std::nullopt;
			if (!std::isfinite(b.mn.x) || !std::isfinite(b.mn.y) || !std::isfinite(b.mn.z)) return std::nullopt;
			if (!std::isfinite(b.mx.x) || !std::isfinite(b.mx.y) || !std::isfinite(b.mx.z)) return std::nullopt;
			const glm::vec3 ext = b.mx - b.mn;
			if (ext.x <= 0.001f || ext.y <= 0.001f || ext.z <= 0.001f) return std::nullopt;
			if (ext.x > 100.0f || ext.y > 100.0f || ext.z > 100.0f) return std::nullopt;
			// sanity: min should be <= max
			if (b.mn.x > b.mx.x || b.mn.y > b.mx.y || b.mn.z > b.mx.z) return std::nullopt;
			return PxBounds3{ b.mn, b.mx };
			};
		std::optional<PxBounds3> bounds;
		uptr picked_off = 0;
		for (uptr off : { (uptr)0x20, (uptr)0x18, (uptr)0x10, (uptr)0x00 }) {
			bounds = try_bounds(off);
			if (bounds) { picked_off = off; break; }
		}
		if (!bounds) {
			static bool s_logged = false;
			if (!s_logged) {
				s_logged = true;
				const uptr addr = reinterpret_cast<uptr>(g.convex_mesh);
				DBG("[physx] tri_convex fail: g.convex_mesh={:#x}", addr);
				DBG("[physx]   sizeof(PxMeshScale)={} offsetof(convex_mesh)={}",
					sizeof(PxMeshScale), offsetof(PxConvexGeom, convex_mesh));

				const u8* gp = reinterpret_cast<const u8*>(&g);
				DBG("[physx]   &g={:#x}  bytes at &g:", reinterpret_cast<uptr>(&g));
				for (size_t i = 0; i < 0x30; ++i) {
					if (i % 16 == 0) std::print("\n    +{:#04x}:", (unsigned)i);
					std::print(" {:02x}", gp[i]);
				}
				DBG("");

				u8 raw[0x40] = {};
				memory::read_memory_raw(addr, raw, sizeof(raw));
				std::print("[physx]   bytes at convex_mesh:");
				for (size_t i = 0; i < sizeof(raw); ++i) {
					if (i % 16 == 0) std::print("\n    +{:#04x}:", (unsigned)i);
					std::print(" {:02x}", raw[i]);
				}
				DBG("");
			}
			return out;
		}

		static bool s_picked_logged = false;
		if (!s_picked_logged) {
			s_picked_logged = true;
			DBG("[physx] tri_convex: AABB at offset +{:#04x} works "
			             "(first bounds: mn=({:.1f},{:.1f},{:.1f}) mx=({:.1f},{:.1f},{:.1f}))",
				(unsigned)picked_off,
				bounds->mn.x, bounds->mn.y, bounds->mn.z,
				bounds->mx.x, bounds->mx.y, bounds->mx.z);
		}

		const auto& mn = bounds->mn;
		const auto& mx = bounds->mx;
		const Mat3 scale_m = g.scale.to_mat33();

		const glm::vec3 local_corners[8] = {
			{mn.x, mn.y, mn.z}, {mx.x, mn.y, mn.z},
			{mn.x, mx.y, mn.z}, {mx.x, mx.y, mn.z},
			{mn.x, mn.y, mx.z}, {mx.x, mn.y, mx.z},
			{mn.x, mx.y, mx.z}, {mx.x, mx.y, mx.z},
		};
		std::array<glm::vec3, 8> world;
		for (int i = 0; i < 8; ++i) {
			world[i] = t.transform_point(scale_m.transform(local_corners[i]));
		}

		constexpr std::array<std::array<int, 3>, 12> faces = { {
			{0,1,2},{1,3,2}, {4,6,5},{5,6,7}, {0,2,4},{2,6,4},
			{1,5,3},{3,5,7}, {0,4,1},{1,4,5}, {2,3,6},{3,7,6}
		} };
		out.reserve(12);
		for (const auto& f : faces) {
			const Triangle tri{ world[f[0]], world[f[1]], world[f[2]] };
			const glm::vec3 n = glm::cross(tri.v1 - tri.v0, tri.v2 - tri.v0);
			if (glm::dot(n, n) < 1e-6f) continue;
			out.push_back(tri);
		}
		return out;
	}

	static std::vector<Triangle> tri_mesh(const PxTransform& t, const PxMeshGeom& g) {
		std::vector<Triangle> out;
		if (!g.triangle_mesh) return out;
		const auto m = memory::read<TriangleMeshData>(reinterpret_cast<uptr>(g.triangle_mesh));
		if (m.nb_vertices == 0 || m.nb_triangles == 0 || !m.vertices || !m.triangles) {
			static bool s_logged = false;
			if (!s_logged) {
				s_logged = true;
				DBG("[physx] tri_mesh first miss: nb_v={} nb_t={} verts={:#x} tris={:#x} flags={:#x}",
					m.nb_vertices, m.nb_triangles,
					reinterpret_cast<uptr>(m.vertices),
					reinterpret_cast<uptr>(m.triangles), m.flags);
			}
			return out;
		}
		if (m.nb_vertices > 65535 || m.nb_triangles > 65535) return out;
		if (m.nb_triangles > 20000) return out;

		std::vector<glm::vec3> verts(m.nb_vertices);
		if (!memory::read_memory_raw(reinterpret_cast<uptr>(m.vertices), verts.data(),
			m.nb_vertices * sizeof(glm::vec3))) return out;

		const bool has16 = (m.flags & 2u) != 0;
		std::vector<u32> idx(m.nb_triangles * 3);
		if (has16) {
			std::vector<u16> idx16(m.nb_triangles * 3);
			if (!memory::read_memory_raw(reinterpret_cast<uptr>(m.triangles), idx16.data(),
				idx16.size() * sizeof(u16))) return out;
			for (size_t i = 0; i < idx16.size(); ++i) idx[i] = idx16[i];
		} else {
			if (!memory::read_memory_raw(reinterpret_cast<uptr>(m.triangles), idx.data(),
				idx.size() * sizeof(u32))) return out;
		}

		const Mat3 scale_m = g.scale.to_mat33();
		out.reserve(m.nb_triangles);
		for (u32 i = 0; i + 2 < idx.size(); i += 3) {
			const u32 i0 = idx[i], i1 = idx[i+1], i2 = idx[i+2];
			if (i0 >= verts.size() || i1 >= verts.size() || i2 >= verts.size()) continue;
			const glm::vec3 v0 = t.transform_point(scale_m.transform(verts[i0]));
			const glm::vec3 v1 = t.transform_point(scale_m.transform(verts[i1]));
			const glm::vec3 v2 = t.transform_point(scale_m.transform(verts[i2]));
			const glm::vec3 n  = glm::cross(v1 - v0, v2 - v0);
			if (glm::dot(n, n) < 1e-6f) continue;
			out.push_back({ v0, v1, v2 });
		}
		return out;
	}

	static AABB compute_aabb(const std::vector<Triangle>& tris, const std::vector<size_t>& idx) {
		AABB b;
		for (auto i : idx) { b.expand(tris[i].v0); b.expand(tris[i].v1); b.expand(tris[i].v2); }
		return b;
	}
	static AABB compute_aabb(const std::vector<Triangle>& tris) {
		AABB b;
		for (const auto& t : tris) { b.expand(t.v0); b.expand(t.v1); b.expand(t.v2); }
		return b;
	}

	static BVHNode* build_bvh(const std::vector<Triangle>& tris, std::vector<size_t>& idx, int depth = 0) {
		auto* n = new BVHNode();
		n->bounds = compute_aabb(tris, idx);
		if (idx.size() <= 16 || depth > 12) { n->tri_indices = idx; return n; }
		const int axis = n->bounds.longest_axis();
		auto mid = idx.begin() + idx.size() / 2;
		std::nth_element(idx.begin(), mid, idx.end(),
			[&](size_t a, size_t b) { return tris[a].center()[axis] < tris[b].center()[axis]; });
		std::vector<size_t> L(idx.begin(), mid), R(mid, idx.end());
		n->left .reset(build_bvh(tris, L, depth + 1));
		n->right.reset(build_bvh(tris, R, depth + 1));
		return n;
	}

	static void build_actor_bvh(Actor& a) {
		if (a.triangles.empty()) return;
		std::vector<size_t> idx(a.triangles.size());
		std::iota(idx.begin(), idx.end(), 0);
		a.bounds = compute_aabb(a.triangles);
		a.bvh.reset(build_bvh(a.triangles, idx));
	}

	static std::atomic<std::shared_ptr<std::vector<Actor>>> g_actors;
	static std::atomic<bool> g_should_stop{ false };
	static uptr g_unity_player_base = 0;
	static uptr g_unity_player_size = 0;

	static bool looks_like_ptr(u64 v)
	{
		return v >= 0x10000ULL && v <= 0x00007FFFFFFFFFFFULL;
	}

	static bool candidate_is_sdk(u64 candidate, u32* out_scenes = nullptr, u32* out_actors = nullptr)
	{
		NpPhysics np{};
		if (!memory::read_memory_raw(candidate, &np, sizeof(np))) return false;
		if (!np.scene_array.data || np.scene_array.size == 0 || np.scene_array.size > 100) return false;
		if (!looks_like_ptr(reinterpret_cast<u64>(np.scene_array.data))) return false;

		u64 scene_ptr = 0;
		if (!memory::read_memory_raw(reinterpret_cast<uptr>(np.scene_array.data), &scene_ptr, sizeof(u64))) return false;
		if (!looks_like_ptr(scene_ptr)) return false;

		PxArray<PxRigidActor*> actors_arr{};
		if (!memory::read_memory_raw(scene_ptr + OFF_PX_RIGID_ACTORS, &actors_arr, sizeof(actors_arr))) return false;
		if (!actors_arr.data || actors_arr.size == 0 || actors_arr.size > 2'000'000) return false;
		if (!looks_like_ptr(reinterpret_cast<u64>(actors_arr.data))) return false;
		if (actors_arr.size < 8) return false;

		if (out_scenes) *out_scenes = np.scene_array.size;
		if (out_actors) *out_actors = actors_arr.size;
		return true;
	}

	static uptr scan_near_offset(uptr hint, uptr radius)
	{
		if (!g_unity_player_base || !g_unity_player_size) return 0;
		const uptr start = (hint > radius) ? (hint - radius) : 0;
		const uptr end = (std::min)(g_unity_player_size, hint + radius);
		constexpr size_t kChunkBytes = 64 * 1024;
		std::vector<u64> chunk(kChunkBytes / sizeof(u64));

		for (uptr off = start & ~7ull; off + 8 <= end; off += kChunkBytes) {
			const size_t n = (std::min)(kChunkBytes, static_cast<size_t>(end - off));
			if (!memory::read_memory_raw(g_unity_player_base + off, chunk.data(), n))
				continue;
			const size_t count = n / sizeof(u64);
			for (size_t i = 0; i < count; ++i) {
				const u64 candidate = chunk[i];
				if (!looks_like_ptr(candidate)) continue;
				u32 scenes = 0, actors = 0;
				if (!candidate_is_sdk(candidate, &scenes, &actors)) continue;
				const uptr found = off + i * sizeof(u64);
				DBG("[physx] nearby scan hit UnityPlayer+{:#x} -> {:#x} (scenes={} actors={})",
					found, candidate, scenes, actors);
				return found;
			}
		}
		return 0;
	}

	static bool cache_actors_once()
	{
		using clk = std::chrono::steady_clock;
		const auto t0 = clk::now();
		auto new_actors = std::make_shared<std::vector<Actor>>();

		if (!g_unity_player_base) {
			DBG("[physx] no UnityPlayer base, skip");
			return false;
		}

		uptr off_sdk = g_off_physx_sdk.load(std::memory_order_acquire);
		if (!off_sdk)
			off_sdk = ::offsets::physx::sdk_offset;

		const auto sdk_ptr = memory::read<u64>(g_unity_player_base + off_sdk);
		if (!sdk_ptr) {
			static int s_wait = 0;
			++s_wait;
			if (s_wait == 1 || (s_wait % 6) == 0)
				DBG("[physx] UnityPlayer+{:#x} is null — waiting for world (try {})", off_sdk, s_wait);
			if (s_wait == 8) {
				DBG("[physx] scanning ±256KB around {:#x}", off_sdk);
				if (const uptr found = scan_near_offset(off_sdk, 256 * 1024)) {
					g_off_physx_sdk.store(found, std::memory_order_release);
					off_sdk = found;
				}
			}
			if (!memory::read<u64>(g_unity_player_base + off_sdk))
				return false;
		}
		g_off_physx_sdk.store(off_sdk, std::memory_order_release);
		const auto sdk_live = memory::read<u64>(g_unity_player_base + off_sdk);
		if (!sdk_live)
			return false;

		const auto sdk = memory::read<NpPhysics>(sdk_live);
		if (!sdk.scene_array.size || !sdk.scene_array.data) {
			DBG("[physx] scene array empty/null (size={}, data={:#x})",
				sdk.scene_array.size, reinterpret_cast<uptr>(sdk.scene_array.data));
			return false;
		}

		std::vector<u64> scene_ptrs(sdk.scene_array.size);
		if (!memory::read_memory_raw(reinterpret_cast<uptr>(sdk.scene_array.data),
			scene_ptrs.data(), scene_ptrs.size() * sizeof(u64))) {
			DBG("[physx] failed to read scene_ptrs");
			return false;
		}
		
		
		int n_scenes = 0, n_actors_seen = 0, n_actors_kept = 0;
		int by_type[8] = { 0 };

		for (u64 scene_ptr : scene_ptrs) {
			if (!scene_ptr) continue;
			++n_scenes;
			
			const auto actors_arr = memory::read<PxArray<PxRigidActor*>>(scene_ptr + OFF_PX_RIGID_ACTORS);
			if (!actors_arr.data || actors_arr.size == 0 || actors_arr.size > 2'000'000) continue;

			std::vector<u64> actor_ptrs(actors_arr.size);
			constexpr size_t kItemsPerChunk = 16 * 1024 / sizeof(u64);
			size_t kept_count = 0;
			for (size_t off = 0; off < actor_ptrs.size(); off += kItemsPerChunk) {
				const size_t count = std::min(kItemsPerChunk, actor_ptrs.size() - off);
				const uptr addr = reinterpret_cast<uptr>(actors_arr.data) + off * sizeof(u64);
				if (!memory::read_memory_raw(addr, actor_ptrs.data() + off, count * sizeof(u64)))
					break;
				kept_count = off + count;
			}
			if (kept_count == 0) continue;
			actor_ptrs.resize(kept_count);

			for (u64 actor_ptr : actor_ptrs) {
				if (!actor_ptr) continue;
				++n_actors_seen;
				const auto a = memory::read<PxActor>(actor_ptr);
				if (a.type != PxConcreteType::RigidDynamic && a.type != PxConcreteType::RigidStatic) continue;

				const auto rs = memory::read<NpRigidStatic>(actor_ptr);
				if (!rs.shape_manager.shapes.m_single) continue;
				const auto shape = memory::read<NpShape>(reinterpret_cast<uptr>(rs.shape_manager.shapes.m_single));
				if (!shape.shape.scene) continue;

				Actor info;
				info.type      = shape.get_geometry_type();
				info.transform = rs.get_global_pose();

				const i32 ti = static_cast<i32>(info.type);
				if (ti >= 0 && ti < 8) ++by_type[ti];

				switch (info.type) {
				case PxGeometryType::Box: {
					const auto& gg = reinterpret_cast<const PxBoxGeom&>(shape.shape.shape_core.core.geometry.g);
					info.triangles = tri_box(info.transform, gg.half_extents);
					break;
				}
				case PxGeometryType::TriangleMesh: {
					const auto& gg = reinterpret_cast<const PxMeshGeom&>(shape.shape.shape_core.core.geometry.g);
					info.triangles = tri_mesh(info.transform, gg);
					break;
				}
				case PxGeometryType::Sphere: {
					const auto& gg = reinterpret_cast<const PxSphereGeom&>(shape.shape.shape_core.core.geometry.g);
					info.triangles = tri_sphere(info.transform, gg.radius);
					break;
				}
				case PxGeometryType::Capsule: {
					const auto& gg = reinterpret_cast<const PxCapsuleGeom&>(shape.shape.shape_core.core.geometry.g);
					info.triangles = tri_capsule(info.transform, gg.radius, gg.half_height);
					break;
				}
				case PxGeometryType::Plane: {
					info.triangles = tri_plane(info.transform);
					break;
				}
				case PxGeometryType::Heightfield: {
					const auto& gg = reinterpret_cast<const PxHeightFieldGeom&>(shape.shape.shape_core.core.geometry.g);
					info.triangles = tri_heightfield(info.transform, gg);
					break;
				}
				case PxGeometryType::ConvexMesh: {
					const auto& gg = reinterpret_cast<const PxConvexGeom&>(shape.shape.shape_core.core.geometry.g);
					info.triangles = tri_convex(info.transform, gg);
					break;
				}
				default: break;
				}

				if (info.triangles.empty()) continue;
				build_actor_bvh(info);
				new_actors->push_back(std::move(info));
				++n_actors_kept;
			}
		}

		size_t total_tris = 0;
		for (const auto& a : *new_actors) total_tris += a.triangles.size();

		const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(clk::now() - t0).count();
		DBG(
			"[physx] cache: {} ms | scenes={} | actors seen={} kept={} | tris={} | by-type "
			"sphere={} plane={} capsule={} box={} cvx={} mesh={} hf={}",
			ms, n_scenes, n_actors_seen, n_actors_kept, total_tris,
			by_type[0], by_type[1], by_type[2], by_type[3], by_type[4], by_type[5], by_type[6]);

		if (n_scenes > 0 && n_actors_seen == 0) {
			DBG("[physx] SDK at UnityPlayer+{:#x} has {} scenes but 0 actors — waiting for world",
				off_sdk, n_scenes);
			return false;
		}

		g_actors.store(new_actors, std::memory_order_release);
		DBG("[physx] cache READY — vischeck active ({} actors, {} tris)", n_actors_kept, total_tris);
		return n_actors_kept > 0;
	}

	void init()
	{
		g_should_stop.store(false, std::memory_order_release);

		auto [base, size] = memory::get_module(xs(L"UnityPlayer.dll"));
		g_unity_player_base = base;
		g_unity_player_size = size;
		if (!g_unity_player_base) {
			DBG("[physx] init: UnityPlayer.dll not found yet — scan thread will retry");
		} else {
			DBG("[physx] init: UnityPlayer @ {:#x} size {:#x}", g_unity_player_base, g_unity_player_size);
			if (::offsets::physx::sdk_offset)
				g_off_physx_sdk.store(::offsets::physx::sdk_offset, std::memory_order_release);
		}

		std::thread([] {
			using namespace std::chrono_literals;
			int fails = 0;
			while (!g_should_stop.load(std::memory_order_acquire)) {
				if (!g_unity_player_base) {
					auto [base, size] = memory::get_module(xs(L"UnityPlayer.dll"));
					g_unity_player_base = base;
					g_unity_player_size = size;
					if (!g_unity_player_base) {
						std::this_thread::sleep_for(2s);
						continue;
					}
					DBG("[physx] UnityPlayer resolved @ {:#x}", g_unity_player_base);
				}

				if (cache_actors_once()) {
					fails = 0;
					std::this_thread::sleep_for(8s);
				} else {
					++fails;
					const auto wait = (fails < 5) ? 2s : 6s;
					std::this_thread::sleep_for(wait);
				}
			}
		}).detach();
	}

	void shutdown() { g_should_stop.store(true, std::memory_order_release); }

	static bool ray_tri(const glm::vec3& o, const glm::vec3& d, const Triangle& t,
	                    float& out_t, glm::vec3& out_n)
	{
		const float EPS = 1e-6f;
		const glm::vec3 e1 = t.v1 - t.v0;
		const glm::vec3 e2 = t.v2 - t.v0;
		const glm::vec3 p  = glm::cross(d, e2);
		const float det    = glm::dot(e1, p);
		if (std::abs(det) < EPS) return false;
		const float invDet = 1.0f / det;

		const glm::vec3 tv = o - t.v0;
		const float u = glm::dot(tv, p) * invDet;
		if (u < 0.0f || u > 1.0f) return false;

		const glm::vec3 q = glm::cross(tv, e1);
		const float v = glm::dot(d, q) * invDet;
		if (v < 0.0f || u + v > 1.0f) return false;

		const float tt = glm::dot(e2, q) * invDet;
		if (tt <= 0.0f) return false;

		out_t = tt;
		out_n = glm::normalize(glm::cross(e1, e2));
		return true;
	}

	static bool ray_bvh(const BVHNode* n, const glm::vec3& o, const glm::vec3& d,
	                    float& closest_t, HitInfo& best, const std::vector<Triangle>& tris)
	{
		float tmin, tmax;
		if (!n->bounds.intersects(o, d, tmin, tmax)) return false;
		if (tmin > closest_t) return false;

		bool hit = false;
		if (!n->left && !n->right) {
			for (size_t i : n->tri_indices) {
				float t; glm::vec3 nrm;
				if (ray_tri(o, d, tris[i], t, nrm) && t < closest_t) {
					closest_t = t;
					best.point    = o + d * t;
					best.normal   = nrm;
					best.distance = t;
					best.hit      = true;
					hit = true;
				}
			}
			return hit;
		}
		if (n->left)  hit |= ray_bvh(n->left .get(), o, d, closest_t, best, tris);
		if (n->right) hit |= ray_bvh(n->right.get(), o, d, closest_t, best, tris);
		return hit;
	}

	HitInfo raycast(const glm::vec3& origin, const glm::vec3& dir, float max_distance)
	{
		HitInfo h;
		if (max_distance <= 0.0f) return h;
		auto actors = g_actors.load(std::memory_order_acquire);
		if (!actors) return h;

		float closest_t = max_distance;
		for (const auto& a : *actors) {
			if (!a.bvh) continue;
			float tmin, tmax;
			if (!a.bounds.intersects(origin, dir, tmin, tmax)) continue;
			if (tmin > closest_t) continue;
			ray_bvh(a.bvh.get(), origin, dir, closest_t, h, a.triangles);
		}
		return h;
	}

	HitInfo linecast(const glm::vec3& from, const glm::vec3& to)
	{
		const glm::vec3 delta = to - from;
		const float len = glm::length(delta);
		if (len < 1e-4f) return {};
		return raycast(from, delta / len, len);
	}

	bool is_visible(const glm::vec3& from, const glm::vec3& to)
	{
		auto actors = g_actors.load(std::memory_order_acquire);
		if (!actors || actors->empty())
			return true;
		const float dist = glm::length(to - from);
		if (dist < 0.05f)
			return true;
		const HitInfo hit = linecast(from, to);
		if (!hit.hit)
			return true;
		if (hit.distance < 0.45f)
			return true;
		const float slop = (std::max)(1.25f, dist * 0.12f);
		return hit.distance >= dist - slop;
	}

	BallisticHit trace_ballistic(const glm::vec3& origin, const glm::vec3& velocity, float gravity_mod, float drag, float max_time)
	{
		BallisticHit out;
		constexpr float dt = 0.03125f;
		constexpr float G = 9.81f;
		glm::vec3 pos = origin;
		glm::vec3 vel = velocity;
		out.path.reserve(64);
		out.path.push_back(pos);
		float t = 0.0f;
		while (t < max_time)
		{
			glm::vec3 next = pos + vel * dt;
			const glm::vec3 delta = next - pos;
			const float step = glm::length(delta);
			if (step > 1e-4f)
			{
				const HitInfo hit = linecast(pos, next);
				if (hit.hit && hit.distance > 0.08f && hit.distance < step - 0.01f)
				{
					out.hit = true;
					out.point = hit.point;
					out.time = t + dt * (hit.distance / step);
					out.path.push_back(hit.point);
					return out;
				}
			}
			pos = next;
			vel.y -= G * gravity_mod * dt;
			vel -= vel * drag * dt;
			t += dt;
			out.path.push_back(pos);
			if (pos.y < origin.y - 250.0f)
				break;
		}
		out.point = pos;
		out.time = t;
		return out;
	}

	bool is_ready()
	{
		auto actors = g_actors.load(std::memory_order_acquire);
		return actors && !actors->empty();
	}

	Stats stats()
	{
		Stats s;
		auto actors = g_actors.load(std::memory_order_acquire);
		if (!actors) return s;
		s.actors = (int)actors->size();
		for (const auto& a : *actors) s.triangles += a.triangles.size();
		return s;
	}
}
