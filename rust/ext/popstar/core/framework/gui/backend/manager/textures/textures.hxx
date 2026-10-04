#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <d3d11.h>

#include <menu/framework/render/stb_image.hpp>
#include <utils/output/console.hxx>
#include <app/embedded_assets.hpp>

namespace core::gui
{
	class c_textures
	{
	public:
		struct texture_data_t
		{
			ID3D11ShaderResourceView* srv = nullptr;
			int width = 0;
			int height = 0;

			~texture_data_t( )
			{
				if ( srv )
				{
					srv->Release( );
					srv = nullptr;
				}
			}
		};

		~c_textures( )
		{
			shutdown( );
		}

		bool initialize( ID3D11Device* device )
		{
			if ( m_initialized )
				return true;

			if ( !device )
			{
				g_console->error( "texture system init failed: null device." );
				return false;
			}

			m_device = device;
			m_initialized = true;

			auto load_rc = [this]( int id, const char* key )
			{
				const void* ptr = nullptr;
				DWORD n = 0;
				if ( !embedded_assets::load( id, &ptr, &n ) ||
					 !load_texture_from_memory( ptr, n, key ) )
					g_console->error( "failed to load embedded texture {}", key );
			};

			load_rc( IDR_IMG_LOGO, k_logo_key );
			load_rc( IDR_IMG_AIMBOT, "icon_aimbot" );
			load_rc( IDR_IMG_VISUALS, "icon_visuals" );
			load_rc( IDR_IMG_MISC, "icon_misc" );
			load_rc( IDR_IMG_SETTINGS, "icon_settings" );
			load_rc( IDR_IMG_PLAYER, "icon_player" );
			load_rc( IDR_IMG_WORLD, "icon_world" );
			load_rc( IDR_IMG_ENTITIES, "icon_entities" );
			load_rc( IDR_IMG_COMBAT, "icon_combat" );
			load_rc( IDR_IMG_MOVEMENT, "icon_movement" );
			load_rc( IDR_IMG_CAMERA, "icon_camera" );

			return true;
		}

		void shutdown( )
		{
			const size_t count = m_textures.size( );
			unload_all_textures( );
			m_device = nullptr;
			m_initialized = false;
			g_console->print( "texture system shutdown ({} textures freed).", count );
		}

		bool load_texture( const std::string& path, const std::string& key )
		{
			if ( !m_initialized || path.empty( ) || key.empty( ) )
			{
				g_console->error( "texture load failed: invalid args (path: {}, key: {})", path, key );
				return false;
			}

			if ( is_texture_loaded( key ) )
				return true;

			ID3D11ShaderResourceView* srv = nullptr;
			int width = 0;
			int height = 0;

			if ( !load_texture_from_file( path.c_str( ), &srv, &width, &height ) )
				return false;

			auto data = std::make_unique<texture_data_t>( );
			data->srv = srv;
			data->width = width;
			data->height = height;
			m_textures[key] = std::move( data );

			g_console->debug( "texture loaded: {} -> {} ({}x{})", path, key, width, height );
			return true;
		}

		bool load_asset( const std::string& relative, const std::string& key )
		{
			const std::string path = resolve_asset_path( relative );
			if ( path.empty( ) || !std::filesystem::exists( path ) )
			{
				g_console->error( "texture asset missing: {}", relative );
				return false;
			}
			return load_texture( path, key );
		}

		bool load_texture_from_memory( const void* data, size_t data_size, const std::string& key )
		{
			if ( !m_initialized || !data || data_size == 0 || key.empty( ) )
			{
				g_console->error( "memory texture load failed (key: {}).", key );
				return false;
			}

			if ( is_texture_loaded( key ) )
				return true;

			ID3D11ShaderResourceView* srv = nullptr;
			int width = 0;
			int height = 0;

			if ( !create_texture_from_memory( data, data_size, &srv, &width, &height ) )
			{
				g_console->error( "failed to create texture from memory: {}", key );
				return false;
			}

			auto data_ptr = std::make_unique<texture_data_t>( );
			data_ptr->srv = srv;
			data_ptr->width = width;
			data_ptr->height = height;
			m_textures[key] = std::move( data_ptr );

			g_console->debug( "memory texture loaded: {} ({}x{}).", key, width, height );
			return true;
		}

		bool load_texture_from_bytes( const void* bytes, size_t size, const std::string& key )
		{
			return load_texture_from_memory( bytes, size, key );
		}

		texture_data_t* get_texture( const std::string& key )
		{
			auto it = m_textures.find( key );
			if ( it == m_textures.end( ) )
				return nullptr;
			return it->second.get( );
		}

		ID3D11ShaderResourceView* get_texture_view( const std::string& key )
		{
			auto* tex = get_texture( key );
			return tex ? tex->srv : nullptr;
		}

		bool get_texture_dimensions( const std::string& key, int& out_w, int& out_h )
		{
			auto* tex = get_texture( key );
			if ( !tex )
				return false;
			out_w = tex->width;
			out_h = tex->height;
			return true;
		}

		void unload_texture( const std::string& key )
		{
			if ( !is_texture_loaded( key ) )
			{
				g_console->error( "texture unload failed (not found): {}", key );
				return;
			}
			m_textures.erase( key );
			g_console->debug( "texture unloaded: {}", key );
		}

		void unload_all_textures( )
		{
			const size_t count = m_textures.size( );
			m_textures.clear( );
			g_console->debug( "all textures unloaded ({} total)", count );
		}

		bool is_texture_loaded( const std::string& key ) const
		{
			return m_textures.find( key ) != m_textures.end( );
		}

		std::vector<std::string> get_loaded_texture_keys( ) const
		{
			std::vector<std::string> keys;
			for ( const auto& [key, _] : m_textures )
				keys.push_back( key );
			return keys;
		}

		size_t get_texture_usage( ) const
		{
			return m_textures.size( );
		}

		size_t get_memory_usage( ) const
		{
			size_t bytes = 0;
			for ( const auto& [_, data] : m_textures )
			{
				if ( data )
					bytes += static_cast<size_t>( data->width ) * static_cast<size_t>( data->height ) * 4u;
			}
			return bytes;
		}

		static constexpr const char* k_logo_key = "logo";

	private:
		bool upload_rgba( const unsigned char* pixels, int width, int height, ID3D11ShaderResourceView** out_srv )
		{
			if ( !m_device || !pixels || !out_srv || width <= 0 || height <= 0 )
				return false;

			D3D11_TEXTURE2D_DESC desc {};
			desc.Width = static_cast<UINT>( width );
			desc.Height = static_cast<UINT>( height );
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			D3D11_SUBRESOURCE_DATA sub {};
			sub.pSysMem = pixels;
			sub.SysMemPitch = static_cast<UINT>( width * 4 );

			ID3D11Texture2D* tex = nullptr;
			HRESULT hr = m_device->CreateTexture2D( &desc, &sub, &tex );
			if ( FAILED( hr ) || !tex )
				return false;

			D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc {};
			srv_desc.Format = desc.Format;
			srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Texture2D.MipLevels = 1;

			hr = m_device->CreateShaderResourceView( tex, &srv_desc, out_srv );
			tex->Release( );
			return SUCCEEDED( hr ) && *out_srv;
		}

		bool load_texture_from_file( const char* name, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height )
		{
			int w = 0, h = 0, channels = 0;
			unsigned char* pixels = stbi_load( name, &w, &h, &channels, 4 );
			if ( !pixels )
			{
				g_console->error( "stbi_load failed: {} ({})", name, stbi_failure_reason( ) );
				return false;
			}

			const bool ok = upload_rgba( pixels, w, h, out_srv );
			stbi_image_free( pixels );
			if ( !ok )
				return false;

			if ( out_width )
				*out_width = w;
			if ( out_height )
				*out_height = h;
			return true;
		}

		bool create_texture_from_memory( const void* data, size_t data_size, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height )
		{
			int w = 0, h = 0, channels = 0;
			unsigned char* pixels = stbi_load_from_memory(
				static_cast<const stbi_uc*>( data ),
				static_cast<int>( data_size ),
				&w,
				&h,
				&channels,
				4 );
			if ( !pixels )
			{
				g_console->error( "stbi_load_from_memory failed ({})", stbi_failure_reason( ) );
				return false;
			}

			const bool ok = upload_rgba( pixels, w, h, out_srv );
			stbi_image_free( pixels );
			if ( !ok )
				return false;

			if ( out_width )
				*out_width = w;
			if ( out_height )
				*out_height = h;
			return true;
		}

		static std::string narrow_path( const std::wstring& w )
		{
			if ( w.empty( ) )
				return {};
			const int bytes = WideCharToMultiByte( CP_UTF8, 0, w.c_str( ), -1, nullptr, 0, nullptr, nullptr );
			if ( bytes <= 1 )
				return {};
			std::string out( static_cast<std::size_t>( bytes - 1 ), '\0' );
			WideCharToMultiByte( CP_UTF8, 0, w.c_str( ), -1, out.data( ), bytes, nullptr, nullptr );
			return out;
		}

		static std::string module_directory( )
		{
			wchar_t buffer[MAX_PATH] {};
			const DWORD n = GetModuleFileNameW( nullptr, buffer, MAX_PATH );
			if ( n == 0 || n >= MAX_PATH )
				return {};
			return narrow_path( std::filesystem::path( buffer ).parent_path( ).wstring( ) );
		}

		static std::string join_path( const std::string& dir, const char* file )
		{
			if ( dir.empty( ) )
				return file ? file : "";
			const char last = dir.back( );
			if ( last == '\\' || last == '/' )
				return dir + ( file ? file : "" );
			return dir + "\\" + ( file ? file : "" );
		}

		static std::string resolve_asset_path( const std::string& relative )
		{
			namespace fs = std::filesystem;
			if ( !relative.empty( ) && fs::exists( relative ) )
				return relative;

			const std::string mod = module_directory( );
			if ( !mod.empty( ) )
			{
				const std::string candidate = join_path( mod, relative.c_str( ) );
				if ( fs::exists( candidate ) )
					return candidate;

				const std::string nested = join_path( join_path( mod, "project" ), relative.c_str( ) );
				if ( fs::exists( nested ) )
					return nested;
			}

			return relative;
		}

		std::unordered_map<std::string, std::unique_ptr<texture_data_t>> m_textures;
		ID3D11Device* m_device = nullptr;
		bool m_initialized = false;
	};

	inline std::shared_ptr<c_textures> g_textures = std::make_shared<c_textures>( );
}
