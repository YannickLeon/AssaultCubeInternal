#include "pch.h"

#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>

#include "proc.h"

DWORD proc::GetProcessIdByName(const wchar_t* procName) {
	unsigned long processId = 0;
	HANDLE snapshot = nullptr;

	// retrieve list of processes for both x64 and x86
	snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) return 0;

	// instatiate ProcessEntry obj and set size
	PROCESSENTRY32 process;
	process.dwSize = sizeof(PROCESSENTRY32);

	// check whether processes have been found
	if (Process32First(snapshot, &process)) {

		//loop through all modules
		do {
			// break if module matches desired module name
			if (!_wcsicmp(procName, process.szExeFile)) {
				processId = process.th32ProcessID;
				break;
			}
		} while (Process32Next(snapshot, &process));
	}

	CloseHandle(snapshot);
	return processId;
}

uintptr_t proc::GetModBaseAddr(DWORD procId, const wchar_t* modName) {
	uintptr_t baseAddr = 0;

	// retrieve list of modules for both x64 and x86
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, procId);
	if (snapshot == INVALID_HANDLE_VALUE) return 0;

	// instatiate ModuleEntry obj and set size
	MODULEENTRY32 module;
	module.dwSize = sizeof(MODULEENTRY32);

	// check whether modules have been found
	if (Module32First(snapshot, &module)) {
		
		//loop through all modules
		do {
			// break if module matches desired module name
			if (!_wcsicmp(modName, module.szModule)) {
				baseAddr = (uintptr_t)module.modBaseAddr;
				break;
			}
		} while (Module32Next(snapshot, &module));
	}

	CloseHandle(snapshot);
	return baseAddr;
}