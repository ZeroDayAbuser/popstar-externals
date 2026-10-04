#include "texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.hpp"

#include <app/app.hpp>
#include <vector>

namespace render {

Texture::~Texture() { release(); }

void Texture::release() {
    if (m_srv) {
        m_srv->Release();
        m_srv = nullptr;
    }
    m_w = m_h = 0;
}

static bool decode_and_upload(const unsigned char* data, std::size_t size,
                              int& out_w, int& out_h,
                              ID3D11ShaderResourceView*& out_srv) {
    int w = 0, h = 0, comp = 0;
    unsigned char* pixels = stbi_load_from_memory(data, int(size), &w, &h, &comp, 4);
    if (!pixels) return false;

    out_w = w;
    out_h = h;

    ID3D11Device* device = app::device;
    if (!device) { stbi_image_free(pixels); return false; }

    D3D11_TEXTURE2D_DESC td{};
    td.Width            = UINT(w);
    td.Height           = UINT(h);
    td.MipLevels        = 1;
    td.ArraySize        = 1;
    td.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage            = D3D11_USAGE_DEFAULT;
    td.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem     = pixels;
    sd.SysMemPitch = UINT(w * 4);

    ID3D11Texture2D* tex = nullptr;
    HRESULT hr = device->CreateTexture2D(&td, &sd, &tex);
    stbi_image_free(pixels);
    if (FAILED(hr) || !tex) return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
    srvd.Format                    = td.Format;
    srvd.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MipLevels       = 1;
    hr = device->CreateShaderResourceView(tex, &srvd, &out_srv);
    tex->Release();
    return SUCCEEDED(hr) && out_srv != nullptr;
}

bool Texture::load_from_memory(const unsigned char* data, std::size_t size) {
    release();
    return decode_and_upload(data, size, m_w, m_h, m_srv);
}

bool Texture::load_from_rgba(const unsigned char* pixels, int width, int height) {
    release();
    if (!pixels || width <= 0 || height <= 0) return false;
    ID3D11Device* device = app::device;
    if (!device) return false;

    D3D11_TEXTURE2D_DESC td{};
    td.Width            = UINT(width);
    td.Height           = UINT(height);
    td.MipLevels        = 1;
    td.ArraySize        = 1;
    td.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage            = D3D11_USAGE_DEFAULT;
    td.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem     = pixels;
    sd.SysMemPitch = UINT(width * 4);

    ID3D11Texture2D* tex = nullptr;
    HRESULT hr = device->CreateTexture2D(&td, &sd, &tex);
    if (FAILED(hr) || !tex) return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
    srvd.Format              = td.Format;
    srvd.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MipLevels = 1;
    hr = device->CreateShaderResourceView(tex, &srvd, &m_srv);
    tex->Release();
    if (FAILED(hr) || !m_srv) return false;

    m_w = width;
    m_h = height;
    return true;
}

}
