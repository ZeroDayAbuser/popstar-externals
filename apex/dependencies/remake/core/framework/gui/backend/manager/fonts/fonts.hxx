#pragma once

#include <algorithm>
#include <cmath>
#include <cfloat>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#ifdef IMGUI_ENABLE_FREETYPE
#include <imgui_freetype.h>
#endif
#include <utils/output/console.hxx>

namespace core::gui
{
	class c_fonts
	{
	public:
		static constexpr const char* k_body_key = "body";
		static constexpr const char* k_title_key = "title";
		static constexpr const char* k_caption_key = "caption";
		static constexpr const char* k_icons_key = "icons";
		static constexpr const char* k_default_key = k_body_key;

		struct font_data_t
		{
			std::string name {};
			std::string key {};
			float size { 0.0f };
			ImFont* font { nullptr };
			ImFontConfig config {};
			bool pixel { false };
			bool gdi { false };
			bool from_file { false };
			std::wstring face {};
			std::wstring file {};
			std::string path_utf8 {};
			bool icons_range { false };
		};

		~c_fonts( )
		{
			this->shutdown( );
		}

		bool start( const std::string& fonts_dir )
		{
			if ( !this->initialize( ) )
			{
				g_console->error( "unable to initialize fonts." );
				return false;
			}

			const std::string dir = resolve_fonts_dir( fonts_dir );
			const std::string body_path = join_path( dir, "SSTMedium.TTF" );
			const std::string title_path = join_path( dir, "SSTBold.TTF" );
			const std::string icons_path = join_path( dir, "fa-solid-900.ttf" );

			if ( !this->load_file_font( k_body_key, body_path, 15.f ) )
			{
				g_console->warn( "body font missing, using default atlas font." );
				this->load_default_font( k_body_key );
			}

			this->load_file_font( k_title_key, title_path, 16.f );
			this->load_file_font( k_caption_key, body_path, 12.f );
			this->load_file_font( k_icons_key, icons_path, 13.f );

			if ( !this->build_fonts( ) )
			{
				g_console->error( "unable to build fonts." );
				return false;
			}

			this->set_default_font( k_body_key );
			return true;
		}

		bool load_file_font( const std::string& key, const std::string& path, float size )
		{
			if ( !m_initialized || key.empty( ) || path.empty( ) || size <= 0.f )
			{
				g_console->error( "file font load failed: invalid args (key: {}).", key );
				return false;
			}

			if ( !std::filesystem::exists( path ) )
			{
				g_console->warn( "font file missing: {}.", path );
				return false;
			}

			if ( is_font_loaded( key ) )
			{
				g_console->warn( "font already loaded: {}.", key );
				return false;
			}

			std::unique_ptr<font_data_t> data = std::make_unique<font_data_t>( );
			data->name = path;
			data->key = key;
			data->size = size;
			data->pixel = false;
			data->gdi = false;
			data->from_file = true;
			data->path_utf8 = path;
			data->icons_range = ( key == k_icons_key );
			data->config = make_pixel_config( key.c_str( ) );
			data->config.PixelSnapH = true;
			data->config.OversampleH = 1;
			data->config.OversampleV = 1;
			data->config.RasterizerMultiply = 1.18f;
			data->config.GlyphExtraAdvanceX = 0.f;
#ifdef IMGUI_ENABLE_FREETYPE
			data->config.FontLoaderFlags =
				ImGuiFreeTypeLoaderFlags_LightHinting | ImGuiFreeTypeLoaderFlags_ForceAutoHint;
#endif
			m_fonts[key] = std::move( data );
			m_fonts_built = false;
			return true;
		}

		bool initialize( )
		{
			if ( m_initialized )
				return false;

			m_initialized = true;
			m_fonts_built = false;
			return true;
		}

		void shutdown( )
		{
			this->unload_all_fonts( );
			for ( const std::wstring& path : m_private_fonts )
				RemoveFontResourceExW( path.c_str( ), FR_PRIVATE, nullptr );
			m_private_fonts.clear( );
			m_initialized = false;
		}

		bool load_gdi_font( const std::string& key, const wchar_t* face, int pixel_height, const std::wstring& file_path = {}, bool pixel = false )
		{
			if ( !m_initialized || key.empty( ) || !face || pixel_height < 1 )
			{
				g_console->error( "gdi font load failed: invalid args (key: {}).", key );
				return false;
			}

			if ( is_font_loaded( key ) )
			{
				g_console->warn( "font already loaded: {}.", key );
				return false;
			}

			std::unique_ptr<font_data_t> data = std::make_unique<font_data_t>( );
			data->name = narrow( face );
			data->key = key;
			data->size = static_cast<float>( pixel_height );
			data->pixel = pixel;
			data->gdi = true;
			data->face = face;
			data->file = file_path;
			data->config = make_pixel_config( key.c_str( ) );
			g_console->debug( "gdi font queued: {} | {}px | key: {}.", data->name, pixel_height, key );
			m_fonts[key] = std::move( data );
			m_fonts_built = false;
			return true;
		}

		bool load_default_font( const std::string& key = k_default_key )
		{
			if ( !m_initialized )
				return false;

			if ( is_font_loaded( key ) )
			{
				g_console->warn( "default font already exists: {}.", key );
				return false;
			}

			std::unique_ptr<font_data_t> data = std::make_unique<font_data_t>( );
			data->key = key;
			data->size = ( key == k_body_key ) ? 15.f : 13.f;
			data->pixel = true;
			data->gdi = false;
			data->config = make_pixel_config( key.c_str( ) );
			m_fonts[key] = std::move( data );
			m_default_font_key = key;
			m_fonts_built = false;
			g_console->print( "default font set: {}.", key );
			return true;
		}

		ImFont* get_font( const std::string& key ) const
		{
			auto it = m_fonts.find( key );
			if ( it == m_fonts.end( ) && key == "default" )
				it = m_fonts.find( k_body_key );
			if ( it == m_fonts.end( ) )
				return nullptr;
			return it->second->font;
		}

		font_data_t* get_font_data( const std::string& key )
		{
			const auto it = m_fonts.find( key );
			if ( it == m_fonts.end( ) )
				return nullptr;
			return it->second.get( );
		}

		bool is_font_loaded( const std::string& key ) const
		{
			return m_fonts.find( key ) != m_fonts.end( );
		}

		void unload_font( const std::string& key )
		{
			if ( !is_font_loaded( key ) )
			{
				g_console->warn( "font unload failed (not found): {}.", key );
				return;
			}

			m_fonts.erase( key );
			m_fonts_built = false;
			g_console->debug( "font unloaded: {}.", key );
		}

		void unload_all_fonts( )
		{
			const std::size_t count = m_fonts.size( );
			m_fonts.clear( );
			m_gdi_bakes.clear( );
			m_fonts_built = false;
			g_console->debug( "all fonts unloaded ({} total).", count );
		}

		bool push_font( const std::string& key )
		{
			ImFont* font = get_font( key );
			if ( !font )
				font = get_font( m_default_font_key );
			if ( !font )
				return false;
			ImGui::PushFont( font, font->LegacySize );
			return true;
		}

		void pop_font( )
		{
			ImGui::PopFont( );
		}

		bool set_default_font( const std::string& key )
		{
			ImFont* font = get_font( key );
			if ( !font )
			{
				g_console->error( "set default font failed (not loaded): {}.", key );
				return false;
			}

			m_default_font_key = key;
			ImGui::GetIO( ).FontDefault = font;
			return true;
		}

		bool build_fonts( )
		{
			if ( !m_initialized )
				return false;

			ImGuiIO& io = ImGui::GetIO( );
			io.FontGlobalScale = 1.f;
			io.DisplayFramebufferScale = { 1.f, 1.f };
			io.Fonts->TexGlyphPadding = 2;
			io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines;
#ifdef IMGUI_ENABLE_FREETYPE
			io.Fonts->FontLoaderFlags =
				ImGuiFreeTypeLoaderFlags_LightHinting | ImGuiFreeTypeLoaderFlags_ForceAutoHint;
#endif

			ImGuiStyle& style = ImGui::GetStyle( );
			style.AntiAliasedLinesUseTex = false;
			style.AntiAliasedLines = true;
			style.AntiAliasedFill = true;

			m_fonts_built = false;
			m_gdi_bakes.clear( );

			std::vector<std::string> order {};
			if ( is_font_loaded( m_default_font_key ) )
				order.push_back( m_default_font_key );
			for ( const auto& [key, _] : m_fonts )
			{
				if ( key != m_default_font_key )
					order.push_back( key );
			}

			static const ImWchar k_icon_ranges[] = { 0xf000, 0xf8ff, 0 };

			for ( const std::string& key : order )
			{
				std::unique_ptr<font_data_t>& data = m_fonts[key];
				if ( data->gdi )
				{
					g_console->warn( "gdi fonts skipped on imgui 1.92 path (key: {}).", key );
					continue;
				}
				else if ( data->from_file && !data->path_utf8.empty( ) )
				{
					ImFontConfig cfg = data->config;
					cfg.FontDataOwnedByAtlas = false;
					cfg.SizePixels = data->size;
					cfg.RasterizerDensity = 1.f;
					cfg.PixelSnapH = true;
					cfg.FontLoaderData = nullptr;
					std::snprintf( cfg.Name, sizeof( cfg.Name ), "%s", key.c_str( ) );
					const ImWchar* ranges = data->icons_range ? k_icon_ranges : nullptr;
					data->font = io.Fonts->AddFontFromFileTTF( data->path_utf8.c_str( ), data->size, &cfg, ranges );
					if ( !data->font )
					{
						g_console->error( "file atlas add failed: {}.", key );
						if ( key == k_body_key )
						{
							cfg.PixelSnapH = true;
							cfg.OversampleH = 1;
							cfg.FontLoaderData = nullptr;
							data->font = io.Fonts->AddFontDefault( &cfg );
						}
						if ( !data->font )
							return false;
					}
				}
				else
				{
					ImFontConfig cfg = data->config;
					cfg.FontDataOwnedByAtlas = false;
					cfg.SizePixels = data->size;
					cfg.RasterizerDensity = 1.f;
					cfg.OversampleH = 1;
					cfg.OversampleV = 1;
					cfg.PixelSnapH = true;
					cfg.FontLoaderData = nullptr;
					std::snprintf( cfg.Name, sizeof( cfg.Name ), "%s", key.c_str( ) );
					data->font = io.Fonts->AddFontDefault( &cfg );
					if ( !data->font )
					{
						g_console->error( "atlas add failed: {}.", key );
						return false;
					}
				}
			}

			if ( io.Fonts->Fonts.empty( ) )
			{
				ImFontConfig cfg = make_pixel_config( k_default_key );
				cfg.FontLoaderData = nullptr;
				io.Fonts->AddFontDefault( &cfg );
			}

			if ( ImFont* def = get_font( m_default_font_key ) )
				io.FontDefault = def;
			else if ( !io.Fonts->Fonts.empty( ) )
				io.FontDefault = io.Fonts->Fonts[0];

			m_fonts_built = !io.Fonts->Fonts.empty( );
			return m_fonts_built;
		}

	private:
		inline static const ImWchar k_gdi_dummy_range[3] = { 0x20, 0x20, 0 };
		static constexpr ImWchar k_gdi_first = 0x20;
		static constexpr ImWchar k_gdi_last = 0xFF;

		static std::string join_path( const std::string& dir, const char* file )
		{
			if ( dir.empty( ) )
				return file ? file : "";
			const char last = dir.back( );
			if ( last == '\\' || last == '/' )
				return dir + ( file ? file : "" );
			return dir + "\\" + ( file ? file : "" );
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
			std::filesystem::path p( buffer );
			return narrow_path( p.parent_path( ).wstring( ) );
		}

		static std::string resolve_fonts_dir( const std::string& fonts_dir )
		{
			namespace fs = std::filesystem;
			if ( !fonts_dir.empty( ) && fs::exists( fonts_dir ) )
				return fonts_dir;

			const std::string mod = module_directory( );
			if ( !mod.empty( ) )
			{
				const std::string candidate = join_path( mod, fonts_dir.c_str( ) );
				if ( fs::exists( candidate ) )
					return candidate;

				const std::string nested = join_path( join_path( mod, "project" ), fonts_dir.c_str( ) );
				if ( fs::exists( nested ) )
					return nested;
			}

			return fonts_dir;
		}

		struct gdi_glyph_t
		{
			ImWchar codepoint {};
			int rect_id { -1 };
			int w { 0 };
			int h { 0 };
			float advance { 0.f };
			ImVec2 offset {};
			std::vector<unsigned char> alpha {};
		};

		struct gdi_bake_t
		{
			std::string key {};
			ImFont* font { nullptr };
			float font_size { 0.f };
			float ascent { 0.f };
			float descent { 0.f };
			std::vector<gdi_glyph_t> glyphs {};
		};

		std::unordered_map<std::string, std::unique_ptr<font_data_t>> m_fonts {};
		std::vector<gdi_bake_t> m_gdi_bakes {};
		std::vector<std::wstring> m_private_fonts {};
		std::string m_default_font_key { k_default_key };
		bool m_initialized { false };
		bool m_fonts_built { false };

		static ImFontConfig make_pixel_config( const char* name )
		{
			ImFontConfig cfg {};
			cfg.FontDataOwnedByAtlas = false;
			cfg.OversampleH = 1;
			cfg.OversampleV = 1;
			cfg.PixelSnapH = true;
			cfg.RasterizerMultiply = 1.f;
			cfg.RasterizerDensity = 1.f;
			cfg.GlyphExtraAdvanceX = 0.f;
			cfg.GlyphOffset = { 0.f, 0.f };
			if ( name )
				std::snprintf( cfg.Name, sizeof( cfg.Name ), "%s", name );
			return cfg;
		}

		static std::string narrow( const std::wstring& w )
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

		static std::uint16_t be16( const unsigned char* p )
		{
			return static_cast<std::uint16_t>( ( p[0] << 8 ) | p[1] );
		}

		static std::uint32_t be32( const unsigned char* p )
		{
			return ( static_cast<std::uint32_t>( p[0] ) << 24 )
				| ( static_cast<std::uint32_t>( p[1] ) << 16 )
				| ( static_cast<std::uint32_t>( p[2] ) << 8 )
				| static_cast<std::uint32_t>( p[3] );
		}

		static bool is_fon_file( const std::wstring& path )
		{
			if ( path.size( ) < 4 )
				return false;
			const std::wstring ext = path.substr( path.size( ) - 4 );
			return _wcsicmp( ext.c_str( ), L".fon" ) == 0;
		}

		static bool is_ttf_like( const std::wstring& path )
		{
			if ( path.size( ) < 4 )
				return false;
			const std::wstring ext = path.substr( path.size( ) - 4 );
			return _wcsicmp( ext.c_str( ), L".ttf" ) == 0
				|| _wcsicmp( ext.c_str( ), L".otf" ) == 0
				|| _wcsicmp( ext.c_str( ), L".ttc" ) == 0;
		}

		static std::wstring ttf_family_name( const std::wstring& path )
		{
			std::ifstream file( path, std::ios::binary | std::ios::ate );
			if ( !file )
				return {};

			const auto bytes = file.tellg( );
			if ( bytes < 12 )
				return {};

			std::vector<unsigned char> data( static_cast<std::size_t>( bytes ) );
			file.seekg( 0, std::ios::beg );
			file.read( reinterpret_cast<char*>( data.data( ) ), bytes );
			if ( !file )
				return {};

			if ( data.size( ) < 12 )
				return {};

			const auto num_tables = be16( data.data( ) + 4 );
			const std::size_t records = 12;
			if ( records + static_cast<std::size_t>( num_tables ) * 16 > data.size( ) )
				return {};

			std::uint32_t name_off = 0;
			std::uint32_t name_len = 0;
			for ( std::uint16_t i = 0; i < num_tables; ++i )
			{
				const unsigned char* rec = data.data( ) + records + static_cast<std::size_t>( i ) * 16;
				if ( std::memcmp( rec, "name", 4 ) == 0 )
				{
					name_off = be32( rec + 8 );
					name_len = be32( rec + 12 );
					break;
				}
			}

			if ( !name_off || name_off + name_len > data.size( ) || name_len < 6 )
				return {};

			const unsigned char* name = data.data( ) + name_off;
			const auto count = be16( name + 2 );
			const auto string_off = be16( name + 4 );
			std::wstring family {};
			std::wstring full {};

			for ( std::uint16_t i = 0; i < count; ++i )
			{
				const unsigned char* rec = name + 6 + static_cast<std::size_t>( i ) * 12;
				if ( rec + 12 > data.data( ) + name_off + name_len )
					break;

				const auto platform = be16( rec );
				const auto encoding = be16( rec + 2 );
				const auto name_id = be16( rec + 6 );
				const auto length = be16( rec + 8 );
				const auto offset = be16( rec + 10 );
				if ( platform != 3 || ( encoding != 1 && encoding != 0 ) )
					continue;
				if ( name_id != 1 && name_id != 4 )
					continue;

				const std::size_t str_at = name_off + string_off + offset;
				if ( str_at + length > data.size( ) )
					continue;

				std::wstring value {};
				value.reserve( length / 2 );
				for ( std::uint16_t b = 0; b + 1 < length; b += 2 )
					value.push_back( static_cast<wchar_t>( ( data[str_at + b] << 8 ) | data[str_at + b + 1] ) );

				if ( name_id == 1 && family.empty( ) )
					family = std::move( value );
				else if ( name_id == 4 && full.empty( ) )
					full = std::move( value );
			}

			return !family.empty( ) ? family : full;
		}

		bool add_private_font( const std::wstring& path )
		{
			if ( path.empty( ) )
				return false;

			for ( const std::wstring& existing : m_private_fonts )
			{
				if ( _wcsicmp( existing.c_str( ), path.c_str( ) ) == 0 )
					return true;
			}

			if ( !AddFontResourceExW( path.c_str( ), FR_PRIVATE, nullptr ) )
			{
				g_console->error( "AddFontResourceEx failed: {}.", narrow( path ) );
				return false;
			}

			m_private_fonts.push_back( path );
			return true;
		}

		static HFONT create_gdi_font( const wchar_t* face, int pixel_height, BYTE quality )
		{
			LOGFONTW lf {};
			lf.lfHeight = -pixel_height;
			lf.lfWidth = 0;
			lf.lfWeight = FW_NORMAL;
			lf.lfCharSet = DEFAULT_CHARSET;
			lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
			lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
			lf.lfQuality = quality;
			lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
			wcsncpy_s( lf.lfFaceName, face, _TRUNCATE );
			return CreateFontIndirectW( &lf );
		}

		static bool face_matches( HDC hdc, const wchar_t* requested )
		{
			wchar_t actual[LF_FACESIZE] {};
			GetTextFaceW( hdc, LF_FACESIZE, actual );
			return _wcsicmp( actual, requested ) == 0;
		}

		static bool raster_glyph_outline( HDC hdc, ImWchar cp, const TEXTMETRICW& tm, gdi_glyph_t& glyph, bool mono )
		{
			MAT2 mat {};
			mat.eM11.value = 1;
			mat.eM22.value = 1;

			const UINT format = mono ? GGO_BITMAP : GGO_GRAY8_BITMAP;
			GLYPHMETRICS gm {};
			const DWORD bytes = GetGlyphOutlineW( hdc, cp, format, &gm, 0, nullptr, &mat );
			if ( bytes == GDI_ERROR )
				return false;

			glyph.codepoint = cp;
			glyph.advance = std::round( static_cast<float>( gm.gmCellIncX ) );
			if ( glyph.advance <= 0.f )
			{
				INT width = 0;
				if ( GetCharWidth32W( hdc, cp, cp, &width ) )
					glyph.advance = std::round( static_cast<float>( width ) );
			}

			if ( bytes == 0 || gm.gmBlackBoxX == 0 || gm.gmBlackBoxY == 0 )
			{
				glyph.w = 1;
				glyph.h = 1;
				glyph.offset = { 0.f, 0.f };
				glyph.alpha.assign( 1, 0 );
				return true;
			}

			std::vector<unsigned char> packed( bytes );
			if ( GetGlyphOutlineW( hdc, cp, format, &gm, bytes, packed.data( ), &mat ) == GDI_ERROR )
				return false;

			const int w = static_cast<int>( gm.gmBlackBoxX );
			const int h = static_cast<int>( gm.gmBlackBoxY );
			glyph.w = w;
			glyph.h = h;
			glyph.offset.x = std::round( static_cast<float>( gm.gmptGlyphOrigin.x ) );
			glyph.offset.y = std::round( static_cast<float>( tm.tmAscent - gm.gmptGlyphOrigin.y ) );
			glyph.alpha.assign( static_cast<std::size_t>( w * h ), 0 );

			if ( mono )
			{
				const int stride = ( ( w + 31 ) / 32 ) * 4;
				for ( int y = 0; y < h; ++y )
				{
					const unsigned char* row = packed.data( ) + y * stride;
					for ( int x = 0; x < w; ++x )
					{
						const unsigned char byte = row[x >> 3];
						if ( byte & ( 1 << ( 7 - ( x & 7 ) ) ) )
							glyph.alpha[static_cast<std::size_t>( y * w + x )] = 255;
					}
				}
			}
			else
			{
				const int stride = ( w + 3 ) & ~3;
				unsigned char peak = 0;
				for ( int y = 0; y < h; ++y )
				{
					const unsigned char* row = packed.data( ) + y * stride;
					for ( int x = 0; x < w; ++x )
						peak = ( std::max )( peak, row[x] );
				}

				for ( int y = 0; y < h; ++y )
				{
					const unsigned char* row = packed.data( ) + y * stride;
					for ( int x = 0; x < w; ++x )
					{
						const int v = row[x];
						unsigned char a = 0;
						if ( peak > 64 )
							a = static_cast<unsigned char>( v );
						else if ( v > 0 )
							a = static_cast<unsigned char>( ( v * 255 + 32 ) / 64 );
						glyph.alpha[static_cast<std::size_t>( y * w + x )] = a;
					}
				}
			}
			return true;
		}

		bool bake_gdi( font_data_t& data, gdi_bake_t& bake )
		{
			if ( !data.file.empty( ) )
				add_private_font( data.file );

			std::wstring face = data.face;
			if ( !data.file.empty( ) && is_ttf_like( data.file ) )
			{
				const std::wstring parsed = ttf_family_name( data.file );
				if ( !parsed.empty( ) )
					face = parsed;
			}

			const int pixel_height = static_cast<int>( data.size );
			const BYTE quality = data.pixel ? NONANTIALIASED_QUALITY : ANTIALIASED_QUALITY;
			HFONT font = create_gdi_font( face.c_str( ), pixel_height, quality );
			if ( !font )
			{
				g_console->error( "CreateFont failed: {}.", data.name );
				return false;
			}

			HDC hdc = CreateCompatibleDC( nullptr );
			if ( !hdc )
			{
				DeleteObject( font );
				return false;
			}

			const HGDIOBJ old_font = SelectObject( hdc, font );
			SetMapMode( hdc, MM_TEXT );
			SetBkMode( hdc, TRANSPARENT );
			SetTextAlign( hdc, TA_LEFT | TA_BASELINE );
			SetTextCharacterExtra( hdc, 0 );

			if ( !face_matches( hdc, face.c_str( ) ) )
			{
				wchar_t actual[LF_FACESIZE] {};
				GetTextFaceW( hdc, LF_FACESIZE, actual );
				g_console->error( "gdi face '{}' not found (got '{}').", narrow( face ), narrow( actual ) );
				SelectObject( hdc, old_font );
				DeleteDC( hdc );
				DeleteObject( font );
				return false;
			}

			TEXTMETRICW tm {};
			GetTextMetricsW( hdc, &tm );

			bake.key = data.key;
			bake.font_size = static_cast<float>( tm.tmHeight );
			bake.ascent = static_cast<float>( tm.tmAscent );
			bake.descent = -static_cast<float>( tm.tmDescent );
			bake.glyphs.reserve( k_gdi_last - k_gdi_first + 1 );

			for ( ImWchar cp = k_gdi_first; cp <= k_gdi_last; ++cp )
			{
				gdi_glyph_t glyph {};
				if ( !raster_glyph_outline( hdc, cp, tm, glyph, data.pixel ) )
				{
					g_console->warn( "gdi glyph {} failed for '{}'.", static_cast<unsigned>( cp ), data.name );
					continue;
				}
				bake.glyphs.push_back( std::move( glyph ) );
			}

			SelectObject( hdc, old_font );
			DeleteDC( hdc );
			DeleteObject( font );

			data.face = face;
			data.name = narrow( face );
			g_console->print( "gdi baked '{}' ({}px cell, {} glyphs, {}).",
				data.name,
				tm.tmHeight,
				bake.glyphs.size( ),
				data.pixel ? "GGO_BITMAP" : "GGO_GRAY8" );
			return !bake.glyphs.empty( );
		}

		bool add_gdi_font( font_data_t& data )
		{
			ImGuiIO& io = ImGui::GetIO( );
			ImFontConfig cfg = make_pixel_config( data.key.c_str( ) );
			cfg.MergeMode = false;
			cfg.DstFont = nullptr;
			cfg.PixelSnapH = true;
			cfg.OversampleH = 1;
			cfg.OversampleV = 1;
			cfg.RasterizerDensity = 1.f;
			cfg.FontDataOwnedByAtlas = true;
			std::snprintf( cfg.Name, sizeof( cfg.Name ), "%s", data.key.c_str( ) );

			gdi_bake_t bake {};
			if ( !bake_gdi( data, bake ) )
				return false;

			cfg.SizePixels = bake.font_size;
			cfg.GlyphRanges = k_gdi_dummy_range;

			const bool force_default_host = data.file.empty( ) || is_fon_file( data.file ) || !is_ttf_like( data.file );
			if ( !force_default_host )
			{
				const std::string host = narrow( data.file );
				data.font = io.Fonts->AddFontFromFileTTF( host.c_str( ), bake.font_size, &cfg, k_gdi_dummy_range );
			}

			if ( !data.font )
			{
				cfg.FontDataOwnedByAtlas = false;
				data.font = io.Fonts->AddFontDefault( &cfg );
				if ( !data.font )
				{
					g_console->error( "atlas host failed for gdi font: {}.", data.key );
					return false;
				}
			}

			bake.font = data.font;
			for ( gdi_glyph_t& glyph : bake.glyphs )
			{
				glyph.rect_id = io.Fonts->AddCustomRectFontGlyph(
					data.font,
					glyph.codepoint,
					glyph.w,
					glyph.h,
					glyph.advance,
					glyph.offset );
			}

			m_gdi_bakes.push_back( std::move( bake ) );
			return true;
		}

		void blit_gdi_glyphs( unsigned char* pixels, int tex_w, int tex_h )
		{
			ImFontAtlas* atlas = ImGui::GetIO( ).Fonts;
			for ( gdi_bake_t& bake : m_gdi_bakes )
			{
				for ( gdi_glyph_t& glyph : bake.glyphs )
				{
					if ( glyph.rect_id < 0 )
						continue;

					ImFontAtlasRect rect {};
					if ( !atlas->GetCustomRect( glyph.rect_id, &rect ) )
						continue;

					const int copy_w = ( std::min )( glyph.w, static_cast<int>( rect.w ) );
					const int copy_h = ( std::min )( glyph.h, static_cast<int>( rect.h ) );
					for ( int y = 0; y < copy_h; ++y )
					{
						if ( rect.y + y >= tex_h )
							break;
						unsigned char* dst = pixels + ( rect.y + y ) * tex_w + rect.x;
						const unsigned char* src = glyph.alpha.data( ) + y * glyph.w;
						const int row = ( std::min )( copy_w, tex_w - static_cast<int>( rect.x ) );
						if ( row > 0 )
							std::memcpy( dst, src, static_cast<std::size_t>( row ) );
					}
				}
			}
		}
	};

	inline std::shared_ptr<c_fonts> g_fonts = std::make_shared<c_fonts>( );
}
