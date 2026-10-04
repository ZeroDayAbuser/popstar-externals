#pragma once

#include <Windows.h>
#include <cstdint>

using wnf_state_name_t = ULONG64;
using wnf_change_stamp_t = ULONG;

struct wnf_state_name_internal_t
{
    ULONG data [ 2 ];
};

using nt_update_wnf_state_data_t = NTSTATUS( NTAPI* )(
    const wnf_state_name_t* state_name ,
    const void* buffer ,
    ULONG length ,
    const void* type_id ,
    const void* explicit_scope ,
    wnf_change_stamp_t matching_change_stamp ,
    LOGICAL check_stamp
    );

struct eprocess_t;
struct peb_t;
struct ethread_t;

typedef union _pml4e
{
    struct
    {
        std::uint64_t present : 1;
        std::uint64_t read_write : 1;
        std::uint64_t user_supervisor : 1;
        std::uint64_t page_write_through : 1;
        std::uint64_t cached_disable : 1;
        std::uint64_t accessed : 1;
        std::uint64_t ignored0 : 1;
        std::uint64_t large_page : 1;
        std::uint64_t ignored1 : 4;
        std::uint64_t pfn : 36;
        std::uint64_t reserved : 4;
        std::uint64_t ignored2 : 11;
        std::uint64_t no_execute : 1;
    } hard;
    std::uint64_t value;
} pml4e , * ppml4e;

typedef union _pdpte
{
    struct
    {
        std::uint64_t present : 1;
        std::uint64_t read_write : 1;
        std::uint64_t user_supervisor : 1;
        std::uint64_t page_write_through : 1;
        std::uint64_t cached_disable : 1;
        std::uint64_t accessed : 1;
        std::uint64_t dirty : 1;
        std::uint64_t page_size : 1;
        std::uint64_t ignored1 : 4;
        std::uint64_t pfn : 36;
        std::uint64_t reserved : 4;
        std::uint64_t ignored2 : 11;
        std::uint64_t no_execute : 1;
    } hard;
    std::uint64_t value;
} pdpte , * ppdpte;

typedef union _pde
{
    struct
    {
        std::uint64_t present : 1;
        std::uint64_t read_write : 1;
        std::uint64_t user_supervisor : 1;
        std::uint64_t page_write_through : 1;
        std::uint64_t cached_disable : 1;
        std::uint64_t accessed : 1;
        std::uint64_t dirty : 1;
        std::uint64_t page_size : 1;
        std::uint64_t global : 1;
        std::uint64_t ignored1 : 3;
        std::uint64_t pfn : 36;
        std::uint64_t reserved : 4;
        std::uint64_t ignored2 : 11;
        std::uint64_t no_execute : 1;
    } hard;
    std::uint64_t value;
} pde , * ppde;

typedef union _pte
{
    struct
    {
        std::uint64_t present : 1;
        std::uint64_t read_write : 1;
        std::uint64_t user_supervisor : 1;
        std::uint64_t page_write_through : 1;
        std::uint64_t cached_disable : 1;
        std::uint64_t accessed : 1;
        std::uint64_t dirty : 1;
        std::uint64_t pat : 1;
        std::uint64_t global : 1;
        std::uint64_t ignored1 : 3;
        std::uint64_t pfn : 36;
        std::uint64_t reserved : 4;
        std::uint64_t ignored2 : 7;
        std::uint64_t protection_key : 4;
        std::uint64_t no_execute : 1;
    } hard;
    std::uint64_t value;
} pte , * ppte;
