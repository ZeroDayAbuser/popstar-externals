#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>

class c_driver
{
public:
	unsigned long      m_pid = 0;
	unsigned long long m_base_address = 0;
	unsigned long long m_cr3 = 0;

	auto setup( ) -> bool;
	auto attach( const wchar_t* process_name ) -> bool;

	auto read_memory( std::uint64_t address , void* buffer , std::size_t size ) -> bool;
	auto write_memory( std::uint64_t address , const void* buffer , std::size_t size ) -> bool;
	auto read_physical_memory( std::uint64_t address , void* buffer , unsigned long size ) -> bool;

	template<typename T>
	auto read( unsigned long long address ) -> T
	{
		T buffer{ };
		this->read_memory( address , &buffer , sizeof( T ) );
		return buffer;
	}

	template<typename T>
	auto write( unsigned long long address , T value ) -> void
	{
		this->write_memory( address , &value , sizeof( T ) );
	}

	auto is_valid( std::uint64_t address ) -> bool
	{
		return address >= 0x10000 && address <= 0x7FFFFFFFFFFF;
	}
};

inline auto hypervisor = std::make_shared<c_driver>( );
