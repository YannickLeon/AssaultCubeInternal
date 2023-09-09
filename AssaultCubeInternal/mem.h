#pragma once

#include <Windows.h>
#include <vector>

namespace mem
{
	void Patch(BYTE* dst, BYTE* src, unsigned int size);
	void Nop(BYTE* dst, unsigned int size);

	uintptr_t ResolveMultiLvlPtr(uintptr_t basePtr, std::vector<unsigned int> offsets);
}