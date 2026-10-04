#pragma once

#include <src/driver/ia32/ia32.cuh>

namespace requests
{
    namespace paging
    {
        constexpr auto page_4kb_size = 0x1000ull;
        constexpr auto page_2mb_size = 0x200000ull;
        constexpr auto page_1gb_size = 0x40000000ull;

        constexpr auto page_shift = 12ull;
        constexpr auto page_2mb_shift = 21ull;
        constexpr auto page_1gb_shift = 30ull;

        constexpr auto page_4kb_mask = 0xFFFull;
        constexpr auto page_2mb_mask = 0x1FFFFFull;
        constexpr auto page_1gb_mask = 0x3FFFFFFFull;

        struct pt_entries_t
        {
            pml4e m_pml4e;
            pdpte m_pdpte;
            pde   m_pde;
            pte   m_pte;
        };
    }

    struct control_initialize_t
    {
        std::uint64_t m_process_id;
        std::uint64_t m_base_address;
        void*         m_response_semaphore;
        void*         m_request_semaphore;
    };

    enum class e_control_type
    {
        none = 0 ,
        verify ,
        get_eprocess ,
        get_process_id ,
        get_process_peb ,
        get_base_address ,
        get_directory_table_base ,
        swap_directory_table_base ,
        hyperspace_entries ,
        hide_process_pages ,
        map_process_page ,
        read_physical ,
        write_physical ,
        read_virtual ,
        write_virtual ,
        lookup_thread ,
        suspend_thread ,
        resume_thread ,
        get_thread_context ,
        set_thread_context ,
        clone_in_hyperspace ,
        hyperspace_get_context ,
        hyperspace_allocate_virtual ,
        hyperspace_free_virtual ,
        hyperspace_remap_pages ,
        hyperspace_expose_pages ,
        hyperspace_clone_pages ,
        hyperspace_rexpose_pages ,
        unload_driver
    };

    struct control_data_t
    {
        e_control_type       m_request_type;
        paging::pt_entries_t m_pt_entries;
        pml4e                m_pml4e;
        pdpte                m_pdpte;
        pde                  m_pde;
        pte                  m_pte;
        std::uint32_t        m_thread_id;
        std::uint32_t        m_process_id;
        std::uint32_t        m_protection;
        std::uint32_t        m_count;
        std::uint32_t        m_mode;
        CONTEXT*             m_context;
        eprocess_t*          m_process;
        eprocess_t*          m_process2;
        ethread_t*           m_thread;
        peb_t*               m_process_peb;
        std::uint64_t        m_address;
        std::uint64_t        m_address1;
        void*                m_address2;
        std::size_t          m_size;
        bool                 m_remove;
        bool                 m_status;
    };
}
