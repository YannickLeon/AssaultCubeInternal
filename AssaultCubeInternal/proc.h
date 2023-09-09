#pragma once

#include <Windows.h>
#include <vector>

namespace proc {
	DWORD GetProcessIdByName(const wchar_t* procName);
	uintptr_t GetModBaseAddr(DWORD procId, const wchar_t* modName);
};