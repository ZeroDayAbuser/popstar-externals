#pragma once
#include <d3d11.h>
#include <cstddef>
#include <string>

namespace render {

class Texture {
public:
    Texture() = default;
    ~Texture();

    bool load_from_memory(const unsigned char* data, std::size_t size);
    // Upload raw RGBA8 pixels (4 bytes/pixel, row-major, top-left origin).
    // Skips stb_image — used when pixels are already decoded (e.g. from
    // a Unity asset bundle).
    bool load_from_rgba(const unsigned char* pixels, int width, int height);
    void release();

    bool                       is_valid() const { return m_srv != nullptr; }
    ID3D11ShaderResourceView*  get_srv()  const { return m_srv; }
    int                        width()    const { return m_w; }
    int                        height()   const { return m_h; }

private:
    ID3D11ShaderResourceView* m_srv = nullptr;
    int m_w = 0, m_h = 0;
};

}

// Lowercase alias used throughout sdk/rust/entity.hpp etc.
using texture_t = render::Texture;
