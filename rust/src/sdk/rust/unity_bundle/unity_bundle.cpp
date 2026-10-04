#include "unity_bundle.hpp"
#include "lz4_decode.hpp"

#include <cstring>
#include <fstream>
#include <print>
#include <utils/debug.hpp>
#include <unordered_map>

namespace unity_bundle {

namespace {


struct Reader {
    const uint8_t* base;
    size_t         size;
    size_t         pos = 0;
    bool           big_endian = true;

    bool ok() const { return pos <= size; }
    bool need(size_t n) const { return pos + n <= size; }

    uint8_t u8() {
        if (!need(1)) { pos = size + 1; return 0; }
        return base[pos++];
    }
    uint16_t u16() {
        if (!need(2)) { pos = size + 1; return 0; }
        uint16_t v;
        if (big_endian) v = (uint16_t)base[pos] << 8 | base[pos + 1];
        else            v = (uint16_t)base[pos + 1] << 8 | base[pos];
        pos += 2;
        return v;
    }
    uint32_t u32() {
        if (!need(4)) { pos = size + 1; return 0; }
        uint32_t v;
        if (big_endian)
            v = (uint32_t)base[pos] << 24 | (uint32_t)base[pos + 1] << 16
              | (uint32_t)base[pos + 2] << 8 | base[pos + 3];
        else
            v = (uint32_t)base[pos + 3] << 24 | (uint32_t)base[pos + 2] << 16
              | (uint32_t)base[pos + 1] << 8 | base[pos];
        pos += 4;
        return v;
    }
    uint64_t u64() {
        const uint64_t hi = u32();
        const uint64_t lo = u32();
        return big_endian ? (hi << 32) | lo : hi | (lo << 32);
    }
    int64_t  i64() { return (int64_t)u64(); }
    int32_t  i32() { return (int32_t)u32(); }

    std::string cstr() {
        if (!ok()) return {};
        size_t s = pos;
        while (pos < size && base[pos] != 0) ++pos;
        std::string out((const char*)(base + s), pos - s);
        if (pos < size) ++pos;
        return out;
    }
    void skip(size_t n) { pos += n; }
    void align(size_t a) {
        const size_t r = pos & (a - 1);
        if (r) pos += a - r;
    }
};

constexpr uint32_t kCompressionMask = 0x3F;
enum : uint32_t {
    kCompressionNone  = 0,
    kCompressionLzma  = 1,
    kCompressionLz4   = 2,
    kCompressionLz4HC = 3,
};
constexpr uint32_t kFlagDirInfoCombined  = 0x40;
constexpr uint32_t kFlagBlocksInfoAtEnd  = 0x80;

bool decompress_block(uint32_t flags,
                      const uint8_t* in, size_t in_size,
                      std::vector<uint8_t>& out,
                      uint32_t out_size,
                      std::string& err) {
    const uint32_t comp = flags & kCompressionMask;
    out.resize(out_size);

    switch (comp) {
    case kCompressionNone:
        if (in_size != out_size) { err = "uncompressed block size mismatch"; return false; }
        std::memcpy(out.data(), in, out_size);
        return true;

    case kCompressionLz4:
    case kCompressionLz4HC: {
        const int n = lz4::decompress(in, (int)in_size, out.data(), (int)out_size);
        if (n != (int)out_size) {
            err = "lz4 decompress failed (got " + std::to_string(n) +
                  " expected " + std::to_string(out_size) + ")";
            return false;
        }
        return true;
    }

    case kCompressionLzma:
        err = "LZMA compression not supported (this bundle uses an old format)";
        return false;

    default:
        err = "unknown compression type " + std::to_string(comp);
        return false;
    }
}

} // anonymous namespace

Bundle parse_file(std::string_view path) {
    Bundle b{};
    std::ifstream f((std::string)path, std::ios::binary | std::ios::ate);
    if (!f) { b.error = "open failed"; return b; }
    const std::streamsize n = f.tellg();
    if (n <= 0) { b.error = "empty file"; return b; }
    f.seekg(0, std::ios::beg);
    std::vector<uint8_t> buf(static_cast<size_t>(n));
    if (!f.read((char*)buf.data(), n)) { b.error = "read failed"; return b; }
    return parse(buf.data(), buf.size());
}

Bundle parse(const uint8_t* src, size_t src_size) {
    Bundle b{};
    Reader r{ src, src_size, 0, true };

    const std::string sig = r.cstr();
    if (sig != "UnityFS") {
        b.error = "not a UnityFS bundle (got '" + sig + "')";
        return b;
    }

    const uint32_t format_version    = r.u32();
    const std::string player_version = r.cstr();
    b.unity_version                  = r.cstr();

    if (format_version != 6 && format_version != 7) {
        b.error = "unsupported UnityFS format version " + std::to_string(format_version);
        return b;
    }

    const uint64_t total_file_size      = r.u64();
    const uint32_t compressed_block_sz  = r.u32();
    const uint32_t uncompressed_block_sz= r.u32();
    const uint32_t flags                = r.u32();
    (void)total_file_size; (void)player_version;

    if (!r.ok()) { b.error = "truncated header"; return b; }

    if (format_version >= 7) r.align(16);

    size_t block_info_offset;
    if (flags & kFlagBlocksInfoAtEnd) {
        if (compressed_block_sz > src_size) { b.error = "blocks-at-end overflows"; return b; }
        block_info_offset = src_size - compressed_block_sz;
    } else {
        block_info_offset = r.pos;
    }

    if (block_info_offset + compressed_block_sz > src_size) {
        b.error = "block info range out of bounds";
        return b;
    }

    std::vector<uint8_t> block_info;
    std::string err;
    if (!decompress_block(flags,
                          src + block_info_offset, compressed_block_sz,
                          block_info, uncompressed_block_sz, err)) {
        b.error = "block info: " + err;
        return b;
    }

    Reader br{ block_info.data(), block_info.size(), 0, true };
    br.skip(16);                                  // file hash
    const uint32_t block_count = br.u32();
    if (!br.ok() || block_count > 1024) {
        b.error = "implausible block count " + std::to_string(block_count);
        return b;
    }

    struct BlockDesc { uint32_t uncompressed; uint32_t compressed; uint16_t flags; };
    std::vector<BlockDesc> blocks(block_count);
    for (auto& bd : blocks) {
        bd.uncompressed = br.u32();
        bd.compressed   = br.u32();
        bd.flags        = br.u16();
    }
    if (!br.ok()) { b.error = "truncated block table"; return b; }

    size_t data_stream_off = (flags & kFlagBlocksInfoAtEnd)
        ? r.pos
        : r.pos + compressed_block_sz;

    uint64_t total_uncompressed = 0;
    for (const auto& bd : blocks) total_uncompressed += bd.uncompressed;
    if (total_uncompressed > 64ull * 1024 * 1024) {
        b.error = "bundle data > 64 MiB (refusing to allocate)";
        return b;
    }

    b.data.reserve((size_t)total_uncompressed);
    std::vector<uint8_t> scratch;
    for (const auto& bd : blocks) {
        if (data_stream_off + bd.compressed > src_size) {
            b.error = "data block extends past EOF";
            return b;
        }
        const uint32_t blk_flags = bd.flags;
        if (!decompress_block(blk_flags,
                              src + data_stream_off, bd.compressed,
                              scratch, bd.uncompressed, err)) {
            b.error = "data block: " + err;
            return b;
        }
        b.data.insert(b.data.end(), scratch.begin(), scratch.end());
        data_stream_off += bd.compressed;
    }

    const uint32_t node_count = br.u32();
    DBG("[bundle] block_count={} node_count={} data_total={}",
                 block_count, node_count, b.data.size());
    if (!br.ok() || node_count > 256) {
        b.error = "implausible node count " + std::to_string(node_count);
        return b;
    }

    struct Node { int64_t offset; int64_t size; uint32_t flags; std::string path; };
    std::vector<Node> nodes(node_count);
    for (auto& nd : nodes) {
        nd.offset = br.i64();
        nd.size   = br.i64();
        nd.flags  = br.u32();
        nd.path   = br.cstr();
    }
    if (!br.ok()) { b.error = "truncated node table"; return b; }

    int node_idx = 0;
    for (const auto& nd : nodes) {
        DBG("[bundle] node[{}] off={} size={} path='{}'",
                     node_idx, nd.offset, nd.size, nd.path);
        ++node_idx;

        if (nd.offset < 0 || (uint64_t)nd.offset + nd.size > b.data.size()) {
            DBG("[bundle]   skip: node range out of bounds");
            continue;
        }

        const auto ends_with = [](std::string_view s, std::string_view suffix) {
            return s.size() >= suffix.size() &&
                   s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
        };
        if (ends_with(nd.path, ".resS") || ends_with(nd.path, ".resource")) {
            DBG("[bundle]   skip: resource stream (not a SerializedFile)");
            if (b.resource_stream_base == 0) b.resource_stream_base = (uint64_t)nd.offset;
            continue;
        }
        const uint8_t* sf = b.data.data() + nd.offset;
        Reader sr{ sf, (size_t)nd.size, 0, true };

        sr.skip(4);                                 // metadata_size (legacy)
        sr.skip(4);                                 // file_size (legacy)
        const uint32_t version = sr.u32();          // SerializedFile version
        const uint32_t data_offset_legacy = sr.u32();
        const uint8_t endianess = sr.u8();
        sr.skip(3);                                 // reserved
        if (!sr.ok()) {
            DBG("[bundle]   skip: truncated SerializedFile header");
            continue;
        }

        uint64_t data_offset = data_offset_legacy;
        if (version >= 22) {
            sr.skip(4);                             // metadata_size_64
            sr.skip(8);                             // file_size_64
            data_offset = sr.u64();
        }
        DBG("[bundle]   sf_version={} endian={} data_off={}",
                     version, endianess, data_offset);

        sr.big_endian = (endianess != 0);

        const std::string node_unity_version = sr.cstr();
        const int32_t target_platform = sr.i32();
        const uint8_t enable_type_tree = sr.u8();
        (void)node_unity_version; (void)target_platform;

        const int32_t type_count = sr.i32();
        DBG("[bundle]   type_count={} type_tree={}",
                     type_count, enable_type_tree ? "yes" : "no");
        if (type_count < 0 || type_count > 1024) {
            DBG("[bundle]   skip: implausible type_count");
            continue;
        }

        struct TypeEntry { int32_t class_id; };
        std::vector<TypeEntry> types(type_count);
        for (int ti = 0; ti < type_count; ++ti) {
            auto& t = types[ti];
            const size_t type_start_pos = sr.pos;
            t.class_id = sr.i32();
            sr.u8();
            sr.u16();
            if (t.class_id == 114 || t.class_id < 0) sr.skip(16);
            sr.skip(16);
            if (enable_type_tree) {
                const int32_t node_count_tt = sr.i32();
                const int32_t string_size  = sr.i32();
                DBG("[bundle]     type[{}] cls={} pos={} tt_nodes={} str_sz={}",
                             ti, t.class_id, type_start_pos, node_count_tt, string_size);
                if (node_count_tt < 0 || string_size < 0 ||
                    node_count_tt > 8192 || string_size > 1024 * 1024) {
                    DBG("[bundle]   skip: implausible type-tree sizes");
                    goto next_node;
                }
                const size_t per_node = (version >= 19) ? 32 : 24;
                sr.skip((size_t)node_count_tt * per_node);
                sr.skip((size_t)string_size);
                if (version >= 21) {
                    const int32_t dep_count = sr.i32();
                    DBG("[bundle]       deps={}", dep_count);
                    if (dep_count < 0 || dep_count > 4096) {
                        DBG("[bundle]   skip: implausible dep_count");
                        goto next_node;
                    }
                    sr.skip((size_t)dep_count * 4);
                }
            }
            if (!sr.ok()) {
                DBG("[bundle]   skip: reader OOB after type[{}]", ti);
                goto next_node;
            }
        }
        if (!sr.ok()) {
            DBG("[bundle]   skip: reader OOB after type table");
            goto next_node;
        }

        {
            const int32_t obj_count = sr.i32();
            DBG("[bundle]   obj_count={} (reader_pos={}/{})",
                         obj_count, sr.pos, sr.size);
            if (obj_count < 0 || obj_count > 65535) {
                DBG("[bundle]   skip: implausible obj_count");
                goto next_node;
            }

            for (int32_t i = 0; i < obj_count; ++i) {
                sr.align(4);
                const int64_t path_id    = sr.i64();
                const uint64_t byte_start = (version >= 22)
                    ? (uint64_t)sr.i64()
                    : (uint64_t)sr.u32();
                const int32_t byte_size  = sr.i32();
                const int32_t type_id    = sr.i32();
                if (!sr.ok()) goto next_node;

                if (type_id < 0 || type_id >= type_count) continue;
                const int32_t class_id = types[type_id].class_id;

                AssetEntry e{};
                e.path_id   = path_id;
                e.class_id  = class_id;
                e.data_off  = (uint32_t)((uint64_t)nd.offset + (uint64_t)data_offset + (uint64_t)byte_start);
                e.data_size = (uint32_t)byte_size;
                if (e.data_off + 4 <= b.data.size() && e.data_size >= 4) {
                    Reader nr{ b.data.data() + e.data_off, e.data_size, 0, sr.big_endian };
                    const uint32_t name_len = nr.u32();
                    if (name_len > 0 && name_len < 256 && nr.need(name_len)) {
                        e.name.assign((const char*)nr.base + nr.pos, name_len);
                    }
                }

                b.assets.push_back(std::move(e));
            }
        }

    next_node:;
    }

    b.ok = true;
    return b;
}

std::optional<Texture2D> parse_texture2d(const Bundle& b, const AssetEntry& e) {
    if (e.class_id != 28) return std::nullopt;
    if (e.data_off + e.data_size > b.data.size()) return std::nullopt;
    if (e.data_size < 64) return std::nullopt;

    Reader r{ b.data.data() + e.data_off, e.data_size, 0, false };  // body is LE

    auto read_string = [&]() -> std::string {
        const uint32_t n = r.u32();
        if (!r.ok() || n > 4096) { r.pos = r.size + 1; return {}; }
        std::string s((const char*)r.base + r.pos, n);
        r.pos += n;
        r.align(4);
        return s;
    };

    Texture2D t{};
    t.name = read_string();
    if (!r.ok()) return std::nullopt;

    r.skip(4);
    r.skip(1); r.align(4);
    t.width      = r.i32();
    t.height     = r.i32();
    r.skip(4);
    t.format     = r.i32();
    t.mip_count  = r.i32();
    r.skip(2);
    r.align(4);
    r.skip(4);
    r.skip(4);
    r.skip(4);

    r.skip(24);

    r.skip(4);
    r.skip(4);

    if (!r.ok()) return std::nullopt;

    const uint32_t image_data_size = r.u32();
    if (image_data_size > 0) {
        if (!r.need(image_data_size)) return std::nullopt;
        t.inline_data.assign(r.base + r.pos, r.base + r.pos + image_data_size);
        r.pos += image_data_size;
        r.align(4);
    }

    if (r.need(8 + 4)) {
        const uint32_t s_offset = r.u32();
        const uint32_t s_size   = r.u32();
        t.stream_path = read_string();
        if (image_data_size == 0) {
            t.stream_offset = s_offset;
            t.stream_size   = s_size;
        }
    }

    return t;
}

std::vector<uint8_t> fetch_pixel_bytes(const Bundle& b, const Texture2D& t) {
    if (!t.inline_data.empty()) return t.inline_data;
    if (t.stream_size == 0) return {};
    if (t.stream_offset + t.stream_size > b.data.size()) return {};
    if (b.resource_stream_base + t.stream_offset + t.stream_size > b.data.size()) return {};
    const uint8_t* src = b.data.data() + b.resource_stream_base + t.stream_offset;
    return std::vector<uint8_t>(src, src + t.stream_size);
}


namespace {

bool decode_rgba32(const Texture2D& t, const uint8_t* src, size_t src_sz,
                   DecodedTexture& out) {
    const size_t need = (size_t)t.width * t.height * 4;
    if (src_sz < need) return false;
    out.width  = t.width;
    out.height = t.height;
    out.rgba.resize(need);

    for (int y = 0; y < t.height; ++y) {
        const uint8_t* row_src = src + (size_t)(t.height - 1 - y) * t.width * 4;
        uint8_t*       row_dst = out.rgba.data() + (size_t)y * t.width * 4;
        std::memcpy(row_dst, row_src, (size_t)t.width * 4);
    }
    return true;
}

}

std::optional<DecodedTexture> decode_texture(const Bundle& b, const Texture2D& t) {
    if (t.width <= 0 || t.height <= 0) return std::nullopt;
    if (t.width > 4096 || t.height > 4096) return std::nullopt;

    const auto bytes = fetch_pixel_bytes(b, t);
    if (bytes.empty()) return std::nullopt;

    DecodedTexture out{};
    switch (t.format) {
    case 4:
        if (!decode_rgba32(t, bytes.data(), bytes.size(), out)) return std::nullopt;
        return out;
    default:
        return std::nullopt;
    }
}

namespace store {

namespace {
    struct State {
        std::unordered_map<std::string, DecodedTexture> by_name;
    };
    State& state() { static State s; return s; }
}

void ingest(const Bundle& b) {
    auto& s = state();
    int decoded = 0, skipped_unsupported = 0, skipped_bad = 0;
    for (const auto& a : b.assets) {
        if (a.class_id != 28) continue;
        auto t = parse_texture2d(b, a);
        if (!t || t->name.empty()) { ++skipped_bad; continue; }
        auto dec = decode_texture(b, *t);
        if (!dec) { ++skipped_unsupported; continue; }
        auto it = s.by_name.find(t->name);
        if (it != s.by_name.end()) {
            const size_t existing_px = (size_t)it->second.width * it->second.height;
            const size_t new_px      = (size_t)dec->width      * dec->height;
            if (new_px >= existing_px) continue;
            it->second = std::move(*dec);
        } else {
            s.by_name.emplace(t->name, std::move(*dec));
        }
        ++decoded;
    }
    DBG("[bundle][store] decoded={} unsupported={} bad={}",
                 decoded, skipped_unsupported, skipped_bad);
}

const DecodedTexture* get(std::string_view name) {
    auto& s = state();
    auto it = s.by_name.find(std::string(name));
    if (it == s.by_name.end()) return nullptr;
    return &it->second;
}

size_t size() { return state().by_name.size(); }

} // namespace store

} // namespace unity_bundle
