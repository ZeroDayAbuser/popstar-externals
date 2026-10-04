#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace unity_bundle {

struct AssetEntry {
    int64_t      path_id   = 0;
    int32_t      class_id  = 0;
    uint32_t     data_off  = 0;
    uint32_t     data_size = 0;
    std::string  name;
};

struct Texture2D {
    std::string  name;
    int32_t      width  = 0;
    int32_t      height = 0;
    int32_t      format = 0;
    int32_t      mip_count = 1;

    std::vector<uint8_t> inline_data;
    uint64_t     stream_offset = 0;
    uint32_t     stream_size   = 0;
    std::string  stream_path;
};

struct Bundle {
    std::string             unity_version;
    std::vector<uint8_t>    data;
    std::vector<AssetEntry> assets;

    uint64_t                resource_stream_base = 0;

    bool                    ok = false;
    std::string             error;
};

std::optional<Texture2D> parse_texture2d(const Bundle& b, const AssetEntry& entry);

struct DecodedTexture {
    int                  width  = 0;
    int                  height = 0;
    std::vector<uint8_t> rgba;       // size = width * height * 4
};

std::vector<uint8_t> fetch_pixel_bytes(const Bundle& b, const Texture2D& t);

std::optional<DecodedTexture> decode_texture(const Bundle& b, const Texture2D& t);

namespace store {
    void                  ingest(const Bundle& b);
    const DecodedTexture* get(std::string_view name);
    size_t                size();
}

Bundle parse(const uint8_t* src, size_t src_size);

Bundle parse_file(std::string_view path);

}
