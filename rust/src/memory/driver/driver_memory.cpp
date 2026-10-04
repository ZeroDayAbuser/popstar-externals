#include "driver_memory.hpp"

#include <TlHelp32.h>

#include <algorithm>
#include <cwctype>
#include <string>
#include <vector>

DriverMemory::~DriverMemory() {
	detach();
}

void DriverMemory::fill_proc_data() {
	m_proc_data = {};
	m_proc_data.process_id = static_cast<uint32_t>(hvre::target_pid);
	m_proc_data.base_address = reinterpret_cast<void*>(hvre::target_base);
	m_proc_data.cr3 = hvre::target_cr3;
	m_proc_data.peb = nullptr;
}

bool DriverMemory::attach(uint32_t process_id) {
	if (!process_id)
		return false;

	HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE)
		return false;

	PROCESSENTRY32W entry{ sizeof(entry) };
	std::wstring name;
	if (::Process32FirstW(snapshot, &entry)) {
		do {
			if (entry.th32ProcessID == process_id) {
				name = entry.szExeFile;
				break;
			}
		} while (::Process32NextW(snapshot, &entry));
	}
	::CloseHandle(snapshot);

	if (name.empty())
		return false;
	return attach(name);
}

bool DriverMemory::attach(const std::wstring& process_name) {
	if (!hvre::available) {
		if (!hvre::initialize())
			return false;
	}

	std::string name_str(process_name.begin(), process_name.end());
	if (!hvre::bind(name_str))
		return false;

	fill_proc_data();
	return m_proc_data.process_id != 0 && m_proc_data.base_address != nullptr;
}

void DriverMemory::detach() {
	m_proc_data = {};
}

bool DriverMemory::read(uint64_t src, void* dest, uint64_t size) {
	if (!src || !dest || !size || !hvre::attached)
		return false;
	return hvre::read(dest, src, size);
}

bool DriverMemory::write(void* src, uint64_t dest, uint64_t size) {
	if (!src || !dest || !size || !hvre::attached)
		return false;
	return hvre::write(dest, src, size);
}

bool DriverMemory::fetch_proc_info(uint32_t process_id, proc_info_request* out_data) {
	if (!out_data)
		return false;
	if (!is_attached() || m_proc_data.process_id != process_id) {
		if (!attach(process_id))
			return false;
	}
	*out_data = m_proc_data;
	return true;
}

uint32_t DriverMemory::get_process_id(const std::wstring& process_name) const {
	HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE)
		return 0;

	PROCESSENTRY32W entry{ sizeof(entry) };
	uint32_t pid = 0;
	if (::Process32FirstW(snapshot, &entry)) {
		do {
			if (::_wcsicmp(entry.szExeFile, process_name.c_str()) == 0) {
				pid = entry.th32ProcessID;
				break;
			}
		} while (::Process32NextW(snapshot, &entry));
	}

	::CloseHandle(snapshot);
	return pid;
}

uint64_t DriverMemory::get_module_base(const std::wstring& target_name) {
	std::string name_str(target_name.begin(), target_name.end());
	if (hvre::modules.empty())
		hvre::refresh_modules();
	return hvre::get_module(name_str).base;
}
