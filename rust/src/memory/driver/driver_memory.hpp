#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <winternl.h>

#include <cstdint>
#include <string>

#include "transport_types.hpp"
#include "hypervisor/hvre.hpp"

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

class DriverMemory {
public:
	DriverMemory() = default;
	~DriverMemory();

	bool attach(uint32_t process_id);
	bool attach(const std::wstring& process_name);
	void detach();

	bool is_attached() const { return hvre::attached && m_proc_data.process_id != 0; }

	bool read(uint64_t src, void* dest, uint64_t size);
	bool write(void* src, uint64_t dest, uint64_t size);

	template <typename T>
	T read(uint64_t src) {
		T buffer{};
		if (!read(src, &buffer, sizeof(T)))
			return T{};
		return buffer;
	}

	template <typename T>
	bool write(uint64_t dest, const T& buffer) {
		return write(const_cast<T*>(&buffer), dest, sizeof(T));
	}

	bool fetch_proc_info(uint32_t process_id, proc_info_request* out_data);
	uint32_t get_process_id(const std::wstring& process_name) const;
	uint64_t get_module_base(const std::wstring& module_name);

	const proc_info_request* get_proc_data() const { return &m_proc_data; }

private:
	void fill_proc_data();

	proc_info_request m_proc_data{};
};
