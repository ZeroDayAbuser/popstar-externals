#pragma once
#include <cstdint>
#include <string>




namespace bypass {
    void initialize ( );
    bool create ( );



    namespace target {
        extern std::uint32_t m_process_pid;
        extern std::uint64_t m_eprocess;
        extern std::uint64_t m_process_peb;
        extern std::uint64_t m_base_address;
        extern std::uint64_t m_directory_table_base;

        bool is_valid ( const uint64_t address ) {
            if ( address <= 0x400000 || address == 0xCCCCCCCCCCCCCCCC || reinterpret_cast< void* >( address ) == nullptr || address > 0x7FFFFFFFFFFFFFFF ) {
                return false;
            }

            return true;
        }


        bool setup ( std::wstring process_name );

        std::uint64_t translate_linear ( std::uint64_t virt_addr , std::uint32_t* page_size = nullptr );
        std::uint64_t allocate_virtual ( std::size_t size );
        bool free_virtual ( std::uint64_t base_address );

        bool read_memory ( std::uint64_t va , void* buf , std::size_t size );
        bool write_memory ( std::uint64_t va , void* buf , std::size_t size );
        void benchmark_rps ( );
    }

    template <typename ret_t = std::uint64_t , typename addr_t>
    ret_t read ( addr_t va ) {
        std::uint64_t va64;
        if constexpr ( std::is_pointer_v<addr_t> )
            va64 = reinterpret_cast< std::uint64_t >( va );
        else if constexpr ( std::is_integral_v<addr_t> )
            va64 = static_cast< std::uint64_t >( va );
        else
            static_assert( std::is_pointer_v<addr_t> || std::is_integral_v<addr_t> ,
                "addr_t must be pointer or integral" );
        ret_t ret {};
        target::read_memory ( va64 , &ret , sizeof ( ret ) );
        return ret;
    }

    template <typename val_t , typename addr_t>
    bool write ( addr_t va , val_t val ) {
        std::uint64_t va64;
        if constexpr ( std::is_pointer_v<addr_t> )
            va64 = reinterpret_cast< std::uint64_t >( va );
        else if constexpr ( std::is_integral_v<addr_t> )
            va64 = static_cast< std::uint64_t >( va );
        else
            static_assert( std::is_pointer_v<addr_t> || std::is_integral_v<addr_t> ,
                "addr_t must be pointer or integral" );

        return target::write_memory ( va64 , &val , sizeof ( val_t ) );
    }
}