#pragma once

#include <cstring>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <d3d11.h>
#include <d3dcompiler.h>

#include <utils/output/console.hxx>
#include <core/framework/gui/backend/render/device.hxx>

namespace core::gui
{
    class c_shaders
    {
    public:
        ID3D11VertexShader* vertex_shader = nullptr;
        ID3D11PixelShader* pixel_shader = nullptr;
        ID3D11InputLayout* input_layout = nullptr;

        c_shaders( ) = default;
        ~c_shaders( ) { shutdown( ); }

        c_shaders( const c_shaders& ) = delete;
        c_shaders& operator=( const c_shaders& ) = delete;
        c_shaders( c_shaders&& ) = delete;
        c_shaders& operator=( c_shaders&& ) = delete;

        static bool compile_source(
            const char* source,
            std::size_t source_bytes,
            const char* entry,
            const char* target,
            ID3DBlob** blob )
        {
            if ( !source || !entry || !target || !blob )
                return false;

            ID3DBlob* errors = nullptr;
            const HRESULT hr = D3DCompile(
                source,
                source_bytes,
                "shader.hlsl",
                nullptr,
                nullptr,
                entry,
                target,
                D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3,
                0,
                blob,
                &errors );

            if ( FAILED( hr ) )
            {
                if ( errors && g_console )
                    g_console->error( "{}", static_cast< const char* >( errors->GetBufferPointer( ) ) );
                if ( errors )
                    errors->Release( );
                return false;
            }

            if ( errors )
                errors->Release( );
            return true;
        }

        bool from_memory(
            ID3D11Device* device,
            const char* source,
            std::size_t source_bytes,
            const char* vs_entry,
            const char* ps_entry,
            const D3D11_INPUT_ELEMENT_DESC* layout_desc,
            UINT layout_desc_count )
        {
            shutdown( );
            if ( !device || !source || !vs_entry )
                return false;

            m_device = device;

            ID3DBlob* vs_blob = nullptr;
            if ( !compile_source( source, source_bytes, vs_entry, "vs_5_0", &vs_blob ) )
                return false;

            HRESULT hr = m_device->CreateVertexShader(
                vs_blob->GetBufferPointer( ), vs_blob->GetBufferSize( ), nullptr, &vertex_shader );
            if ( SUCCEEDED( hr ) && layout_desc && layout_desc_count > 0 )
            {
                hr = m_device->CreateInputLayout(
                    layout_desc,
                    layout_desc_count,
                    vs_blob->GetBufferPointer( ),
                    vs_blob->GetBufferSize( ),
                    &input_layout );
            }
            vs_blob->Release( );
            if ( FAILED( hr ) )
                return false;

            if ( !ps_entry )
                return true;

            ID3DBlob* ps_blob = nullptr;
            if ( !compile_source( source, source_bytes, ps_entry, "ps_5_0", &ps_blob ) )
                return false;

            hr = m_device->CreatePixelShader(
                ps_blob->GetBufferPointer( ), ps_blob->GetBufferSize( ), nullptr, &pixel_shader );
            ps_blob->Release( );
            return SUCCEEDED( hr );
        }

        bool add_pixel(
            const char* name,
            const char* source,
            std::size_t source_bytes,
            const char* entry )
        {
            if ( !m_device || !name || !source || !entry )
                return false;

            ID3DBlob* blob = nullptr;
            if ( !compile_source( source, source_bytes, entry, "ps_5_0", &blob ) )
                return false;

            extra_ps_t extra {};
            extra.name = name;
            const HRESULT hr = m_device->CreatePixelShader(
                blob->GetBufferPointer( ), blob->GetBufferSize( ), nullptr, &extra.shader );
            blob->Release( );
            if ( FAILED( hr ) )
                return false;

            m_extras.push_back( extra );
            return true;
        }

		bool load_vertex_shader( const std::string& file_path, const D3D11_INPUT_ELEMENT_DESC* layout_desc, UINT layout_desc_count )
		{
			if ( !g_device || !g_device->m_device )
				return false;

			std::string source;
			if ( !read_file( file_path, source ) )
				return false;

			ID3DBlob* vs_blob = nullptr;
			if ( !compile_source( source.data( ), source.size( ), "main", "vs_5_0", &vs_blob ) )
				return false;

			HRESULT hr = g_device->m_device->CreateVertexShader(
				vs_blob->GetBufferPointer( ), vs_blob->GetBufferSize( ), nullptr, &vertex_shader );
			if ( SUCCEEDED( hr ) && layout_desc && layout_desc_count > 0 )
			{
				hr = g_device->m_device->CreateInputLayout(
					layout_desc, layout_desc_count, vs_blob->GetBufferPointer( ), vs_blob->GetBufferSize( ), &input_layout );
			}
			vs_blob->Release( );
			return SUCCEEDED( hr );
		}

		bool load_pixel_shader( const std::string& file_path )
		{
			if ( !g_device || !g_device->m_device )
				return false;

			std::string source;
			if ( !read_file( file_path, source ) )
				return false;

			ID3DBlob* ps_blob = nullptr;
			if ( !compile_source( source.data( ), source.size( ), "main", "ps_5_0", &ps_blob ) )
				return false;

			const HRESULT hr = g_device->m_device->CreatePixelShader(
				ps_blob->GetBufferPointer( ), ps_blob->GetBufferSize( ), nullptr, &pixel_shader );
			ps_blob->Release( );
			return SUCCEEDED( hr );
		}

        c_shaders& with_vertex_shader( const std::string& file_path, const D3D11_INPUT_ELEMENT_DESC* layout_desc, UINT layout_desc_count )
        {
            load_vertex_shader( file_path, layout_desc, layout_desc_count );
            return *this;
        }

        c_shaders& with_pixel_shader( const std::string& file_path )
        {
            load_pixel_shader( file_path );
            return *this;
        }

        bool initialize( const std::string& vs_path, const D3D11_INPUT_ELEMENT_DESC* layout_desc, UINT layout_desc_count, const std::string& ps_path )
        {
            return load_vertex_shader( vs_path, layout_desc, layout_desc_count ) &&
                load_pixel_shader( ps_path );
        }

        void shutdown( )
        {
            if ( vertex_shader ) { vertex_shader->Release( ); vertex_shader = nullptr; }
            if ( pixel_shader ) { pixel_shader->Release( ); pixel_shader = nullptr; }
            if ( input_layout ) { input_layout->Release( ); input_layout = nullptr; }
            for ( extra_ps_t& extra : m_extras )
            {
                if ( extra.shader )
                    extra.shader->Release( );
            }
            m_extras.clear( );
            m_device = nullptr;
        }

        void bind( ID3D11DeviceContext* context )
        {
            if ( !context )
                return;
            context->VSSetShader( vertex_shader, nullptr, 0 );
            context->PSSetShader( pixel_shader, nullptr, 0 );
            context->IASetInputLayout( input_layout );
        }

        void bind_pixel( ID3D11DeviceContext* context, const char* name = nullptr )
        {
            if ( !context )
                return;
            if ( !name )
            {
                context->PSSetShader( pixel_shader, nullptr, 0 );
                return;
            }
            for ( const extra_ps_t& extra : m_extras )
            {
                if ( extra.name && std::strcmp( extra.name, name ) == 0 )
                {
                    context->PSSetShader( extra.shader, nullptr, 0 );
                    return;
                }
            }
            context->PSSetShader( pixel_shader, nullptr, 0 );
        }

        [[nodiscard]] bool ready( ) const
        {
            return vertex_shader && pixel_shader && input_layout;
        }

    private:
        struct extra_ps_t
        {
            const char* name { nullptr };
            ID3D11PixelShader* shader { nullptr };
        };

        static bool read_file( const std::string& file_path, std::string& source )
        {
            std::ifstream file( file_path, std::ios::binary | std::ios::ate );
            if ( !file )
                return false;

            const std::streamsize size = file.tellg( );
            if ( size <= 0 )
                return false;

            file.seekg( 0, std::ios::beg );
            source.assign( static_cast< std::size_t >( size ), '\0' );
            return static_cast< bool >( file.read( source.data( ), size ) );
        }

        ID3D11Device* m_device { nullptr };
        std::vector< extra_ps_t > m_extras {};
    };

    inline std::shared_ptr<c_shaders> g_shaders = std::make_shared<c_shaders>( );
}
