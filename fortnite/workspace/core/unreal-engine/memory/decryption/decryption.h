#pragma once

#include <cstdint>
#include "../../../../../impl/driverless/day1.h"
#include <memory>

struct g_decryptions_ {
	inline uint64_t decrypt ( )
	{


		uint64_t v = bypass::read<uint64_t> ( bypass::target::m_base_address + 0x19D0D570 );
		if ( !v )
			return 0;

		v ^= 0xC774FEull;
		v = _rotl64 ( v , 56 );
		v ^= 0xF81BA2C7ull;
		v *= 0x80CD98FC8EC532E9ull;

		return static_cast< uintptr_t >( v );

	}
};

std::unique_ptr<g_decryptions_> g_decrypt = std::make_unique<g_decryptions_> ( );