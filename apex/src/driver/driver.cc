#include <src/driver/driver.cuh>
#include <src/driver/athena/hypervisor/hvre.hpp>

#include <Windows.h>
#include <chrono>
#include <thread>

auto c_driver::setup( ) -> bool
{
	if ( !hvre::initialize( ) )
		return false;
	return hvre::available;
}

auto c_driver::attach( const wchar_t* process_name ) -> bool
{
	char narrow[ MAX_PATH ]{ };
	WideCharToMultiByte( CP_UTF8 , 0 , process_name , -1 , narrow , MAX_PATH , nullptr , nullptr );

	while ( !hvre::get_pid( narrow ) )
		std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );

	if ( !hvre::bind( narrow ) )
		return false;

	m_pid          = static_cast<unsigned long>( hvre::target_pid );
	m_cr3          = hvre::target_cr3;
	m_base_address = hvre::target_base;

	return m_pid && m_cr3 && m_base_address;
}

auto c_driver::read_memory( std::uint64_t address , void* buffer , std::size_t size ) -> bool
{
	if ( !buffer || !size || !is_valid( address ) )
		return false;
	return hvre::read( buffer , address , size );
}

auto c_driver::write_memory( std::uint64_t address , const void* buffer , std::size_t size ) -> bool
{
	if ( !buffer || !size || !is_valid( address ) )
		return false;
	return hvre::write( address , buffer , size );
}

auto c_driver::read_physical_memory( std::uint64_t address , void* buffer , unsigned long size ) -> bool
{
	if ( !address || !buffer || !size )
		return false;
	return read_memory( address , buffer , size );
}
