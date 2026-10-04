#pragma once

#include <windows.h>
#include "../../resources/resource.h"

namespace embedded_assets
{
	inline bool load(int id, const void** ptr, DWORD* size)
	{
		if (!ptr || !size)
			return false;
		*ptr = nullptr;
		*size = 0;

		HMODULE self = nullptr;
		if (!::GetModuleHandleExW(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			reinterpret_cast<LPCWSTR>(&load),
			&self) || !self)
			return false;

		const HRSRC h = ::FindResourceW(self, MAKEINTRESOURCEW(id), RT_RCDATA);
		if (!h)
			return false;
		const HGLOBAL g = ::LoadResource(self, h);
		if (!g)
			return false;
		const void* p = ::LockResource(g);
		const DWORD n = ::SizeofResource(self, h);
		if (!p || !n)
			return false;
		*ptr = p;
		*size = n;
		return true;
	}
}
