#pragma once
#include <dependencies/includes.h>
#include <src/cheat/loot/types.cuh>

class c_cache
{
public:
    classes::c_entity                 m_local { };
    std::vector<classes::c_entity>    m_players { };
    std::vector<loot::entity_t>   m_loot { };
    std::mutex                        m_mutex { };

    auto update( ) -> bool;
    auto tick( ) -> void;
};

inline std::unique_ptr<c_cache> cache = std::make_unique<c_cache>( );
