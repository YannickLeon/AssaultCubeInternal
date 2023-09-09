#include "pch.h"

#include <iostream>
#include <vector>

#include "mem.h"

void mem::Patch(BYTE* dst, BYTE* src, unsigned int size) 
{
	DWORD oldProtect = 0;
	// set new access rights
	if (!VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		std::cout << "[Error] VirtualProtect - " << GetLastError() << std::endl;
	}
	// set memory
	memcpy(dst, src, size);
	// reset access rights
	if (!VirtualProtect(dst, size, oldProtect, &oldProtect)) {
		std::cout << "[Error] VirtualProtect - " << GetLastError() << std::endl;
	}
}

void mem::Nop(BYTE* dst, unsigned int size)
{
	DWORD oldProtect = 0;
	// set new access rights
	if (!VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		std::cout << "[Error] VirtualProtect - " << GetLastError() << std::endl;
	}
	// set memory
	memset(dst, 0x90, size);
	// reset access rights
	if (!VirtualProtect(dst, size, oldProtect, &oldProtect)) {
		std::cout << "[Error] VirtualProtect - " << GetLastError() << std::endl;
	}
}

uintptr_t mem::ResolveMultiLvlPtr(uintptr_t basePtr, std::vector<unsigned int> offsets) {
	uintptr_t addr = basePtr;
	for (unsigned int i = 0; i < offsets.size(); i++) {
		addr = *(uintptr_t*)addr + offsets[i];
	}
	return addr;
}