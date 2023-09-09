// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "mem.h"
#include "playerEnt.h"

#include <Windows.h>
#include <math.h>

#define PI 3.14159265359

uintptr_t calculateHex(int hex) {
    if (hex >= 0) return (uintptr_t)hex;
    if (hex < 0) return 0xFFFFFFFF + hex + 1;
}

void fly(uintptr_t localPlayer) {
    playerEnt* player = *(playerEnt**)localPlayer;
    if (GetAsyncKeyState(87)) {
        //std::cout << "x-movement: " << sinf(player->Rotation.x * (PI / 180)) << "\n" << "y-movement: " << cosf(player->Rotation.x * (PI / 180)) << "\n\n";
        player->Position.x += 0.7f * sinf(player->Rotation.x * (PI / 180));
        player->Position.y -= 0.7f * cosf(player->Rotation.x * (PI / 180));

        player->HeadPos.x = player->Position.x;
        player->HeadPos.y = player->Position.y;
    }
    if (GetAsyncKeyState(VK_SPACE)) {
        player->Position.z += 1;

        player->HeadPos.z = player->Position.z + player->CurrentHeight;
    }
    if (GetAsyncKeyState(VK_SHIFT)) {
        player->Position.z -= 1;

        player->HeadPos.z = player->Position.z + player->CurrentHeight;
    }
}

void generateShellCodePlayer(uintptr_t modBaseAddr, uintptr_t functionOffset, char* cmpCode, uintptr_t cmpCodeSize, char* playerAction, uintptr_t playerActionSize, char* originalCode, uintptr_t originalCodeSize, char* shellCodePtr, uintptr_t additionalPlayerOffset = 0) {
    // shellcode for exploit
   // cmp [ESI+0x08], [playerAddr]
    memcpy(shellCodePtr, cmpCode, cmpCodeSize);
    *(int*)(shellCodePtr + cmpCodeSize) = (*(int*)(modBaseAddr + 0x10F4F4)) + additionalPlayerOffset;
    // jne shellCode+playerActionSize (default code)
    memcpy(shellCodePtr + cmpCodeSize + 4, "\x0f\x85", 2);
    *(int*)(shellCodePtr + cmpCodeSize + 6) = calculateHex(((uintptr_t)shellCodePtr + cmpCodeSize + 10 + playerActionSize + 5) - ((uintptr_t)shellCodePtr + cmpCodeSize + 4 + 6));
    // perform player specific code
    memcpy(shellCodePtr + cmpCodeSize + 10, playerAction, playerActionSize);
    unsigned int offset = cmpCodeSize + 10 + playerActionSize;
    // jmp originalAddr
    memcpy(shellCodePtr + offset, "\xe9", 1);
    *(int*)(shellCodePtr + offset + 1) = calculateHex((modBaseAddr + functionOffset + originalCodeSize) - ((uintptr_t)shellCodePtr + offset + 5));
    // perform code for all other entities
    memcpy(shellCodePtr + offset + 5, originalCode, originalCodeSize);
    offset = offset + 5 + originalCodeSize;
    // jmp originalAddr
    memcpy(shellCodePtr + offset, "\xe9", 1);
    *(int*)(shellCodePtr + offset + 1) = calculateHex((modBaseAddr + functionOffset + originalCodeSize) - ((uintptr_t)shellCodePtr + offset + 5));
}

bool generateHook(uintptr_t addr, uintptr_t target, char* shellCode, unsigned int size) {
    if (size < 5) return false;

    memset(shellCode, 0x90, size);
    // jmp [newAddr]
    memcpy(shellCode, "\xe9", 1);
    *(int*)(shellCode + 1) = target - (addr + 5);

    return true;
}

bool prepareFunctionCall(uintptr_t hookAddr, unsigned int hookSize, char* hookShellCode, char* shellCode, uintptr_t functionPtr, uintptr_t returnAddr, uintptr_t argument) {
    if (!generateHook(hookAddr, (uintptr_t)shellCode, hookShellCode, hookSize)) return false;
    
    memset(shellCode, 0x90, 64);
    // push argument
    memcpy(shellCode, "\x68", 1);
    *(int*)(shellCode + 1) = argument;
    // call function
    memcpy(shellCode + 5, "\xE8", 1);
    *(int*)(shellCode + 6) = functionPtr - (((uintptr_t)shellCode + 5) + 5);
    // pop into edi(careful, might override values) to adjust for pushed argument
    memcpy(shellCode + 10, "\x5F", 1);
    // jmp to returnAddr
    memcpy(shellCode + 11, "\xe9", 1);
    *(int*)(shellCode + 12) = returnAddr - (((uintptr_t)shellCode + 11) + 5);

    return true;
}

const char* printBool(bool b) {
    if (b) return "ON ";
    else return "OFF";
}

void printInfo(bool bInvincible, bool bFlying, bool bAmmo, bool bGrenades, bool bRecoil, bool bRapidFire, bool bOneShot, bool bAutoFire, bool bDrone) {
    system("cls");
    std::cout << "======= Yannick's internal AssaultCube Trainer ===========================\n";
    std::cout << "[Numpad 1] Invincibility     < " << printBool(bInvincible) << " >\n";
    std::cout << "[Numpad 2] Flight Mode       < " << printBool(bFlying) << " >\n";
    std::cout << "[Numpad 3] Infinite Ammo     < " << printBool(bAmmo) << " >\n";
    std::cout << "[Numpad 4] Infinite Grenades < " << printBool(bGrenades) << " >\n";
    std::cout << "[Numpad 5] No Recoil         < " << printBool(bRecoil) << " >\n";
    std::cout << "[Numpad 6] Rapid Fire        < " << printBool(bRapidFire) << " >\n";
    std::cout << "[Numpad 7] One Shot Kills    < " << printBool(bOneShot) << " >\n";
    std::cout << "[Numpad 8] Automatic Fire    < " << printBool(bAutoFire) << " >\n";
    std::cout << "[Numpad 9] Drone mode        < " << printBool(bDrone) << " >\n";
    std::cout << "[End     ] Quit\n";
    std::cout << "===========================================================================\n";
}

DWORD WINAPI HackThread(HMODULE hModule) {
    // Create console
    AllocConsole();
    FILE *f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    
    // Get module base addr
    uintptr_t modBaseAddr = (uintptr_t)GetModuleHandle(L"ac_client.exe");
    
    bool bInvincible = false, bFlying = false, bAmmo = false, bGrenades = false, bRecoil = false, bRapidFire = false, bOneShot = false, bAutoFire = false, bDrone = false;

    // generate shellcode for rapidFire
    char rapidShellCode[50];
    memset(&rapidShellCode, 0x90, 50);
    generateShellCodePlayer(modBaseAddr, 0x637DA, (char*)"\x81\x7E\x08", 3, (char*)"\xb9\x0a\x00\x00\x00", 5, (char*)"\x0F\xBF\x88\x0A\x01\x00\x00", 7, rapidShellCode);

    // create jump to activate rapidFire exploit
    char rapidJump[7];
    generateHook(modBaseAddr + 0x637DA, (uintptr_t)&rapidShellCode, rapidJump, 7);

    // generate shellcode for one-shots
    char oneShotShellCode[50];
    memset(&oneShotShellCode, 0x90, 50);
    generateShellCodePlayer(modBaseAddr, 0x613C9, (char*)"\x81\xFE", 2, (char*)"\xbb\xE8\x03\x00\x00", 5, (char*)"\x0F\xBF\x99\x0c\x01\x00\x00", 7, oneShotShellCode);

    // create jump to activate one-shot exploit
    char oneShotJump[7];
    generateHook(modBaseAddr + 0x613C9, (uintptr_t)&oneShotShellCode, oneShotJump, 7);

    // generate shellcode for automatic fire
    char autoFireShellCode[60];
    memset(&autoFireShellCode, 0x90, 60);
    generateShellCodePlayer(modBaseAddr, 0x6371B, (char*)"\x3D", 1, (char*)"\xC7\x80\x21\x02\x00\x00\x00\x00\x00\x01", 10, (char*)"\x88\x98\x24\x02\x00\x00", 6, autoFireShellCode);

    // create jump to activate one-shot exploit
    char autoFireJump[6];
    generateHook(modBaseAddr + 0x6371B, (uintptr_t)&autoFireShellCode, autoFireJump, 6);

    // generate shellcode for invincibility
    char invShellCode[60];
    memset(&invShellCode, 0x90, 60);
    generateShellCodePlayer(modBaseAddr, 0x29D1F, (char*)"\x81\xFB", 2, (char*)"\x90", 1, (char*)"\x29\x7B\x04\x8B\xC7", 5, invShellCode, 0xF4);

    // create jump to activate invincibility exploit
    char invJump[5];
    generateHook(modBaseAddr + 0x29D1F, (uintptr_t)&invShellCode, invJump, 5);

    // generate shellcode for flying
    char flyJump[5];
    char flyShellCode[64];
    void* flyFunction = fly;
    prepareFunctionCall(modBaseAddr + 0x25810, 5, flyJump, flyShellCode, (uintptr_t)flyFunction, modBaseAddr + 0x2582A, modBaseAddr + 0x10F4F4);
   
    printInfo(bInvincible, bFlying, bAmmo, bGrenades, bRecoil, bRapidFire, bOneShot, bAutoFire, bDrone);
    std::cout << flyFunction << std::endl;

    // Hack loop
    while (true) {
        playerEnt* localPlayer = *(playerEnt**)(modBaseAddr + 0x10F4F4);

        if (GetAsyncKeyState(VK_NUMPAD1) & 0x01) {
            bInvincible = !bInvincible;
            printInfo(bInvincible, bFlying, bAmmo, bGrenades, bRecoil, bRapidFire, bOneShot, bAutoFire, bDrone);
            if (bInvincible)
            {
                mem::Patch((BYTE*)(modBaseAddr + 0x29D1F), (BYTE*)invJump, 5);
            }
            else
            {
                mem::Patch((BYTE*)(modBaseAddr + 0x29D1F), (BYTE*)"\x29\x7B\x04\x8B\xC7", 5);
            }
        }
        if (GetAsyncKeyState(VK_NUMPAD2) & 0x01) {
            bFlying = !bFlying;
            printInfo(bInvincible, bFlying, bAmmo, bGrenades, bRecoil, bRapidFire, bOneShot, bAutoFire, bDrone);
            if (bFlying) 
            {
                mem::Patch((BYTE*)(modBaseAddr + 0x25810), (BYTE*)flyJump, 5);
            }
            else
            {
                mem::Patch((BYTE*)(modBaseAddr + 0x25810), (BYTE*)"\x75\x18\x6A\x0A\xE8", 5);
            }
        }
        if (GetAsyncKeyState(VK_NUMPAD3) & 0x01) {
            bAmmo = !bAmmo;
            printInfo(bInvincible, bFlying, bAmmo, bGrenades, bRecoil, bRapidFire, bOneShot, bAutoFire, bDrone);
            if (bAmmo)
            {
                mem::Nop((BYTE*)(modBaseAddr + 0x637E9), 2);
            }
            else
            {
                mem::Patch((BYTE*)(modBaseAddr + 0x637E9), (BYTE*)"\xFF\x0E", 2);
            }
        }
        if (GetAsyncKeyState(VK_NUMPAD4) & 0x01) {
            bGrenades = !bGrenades;
            printInfo(bInvincible, bFlying, bAmmo, bGrenades, bRecoil, bRapidFire, bOneShot, bAutoFire, bDrone);
            if (bGrenades)
            {
                mem::Nop((BYTE*)(modBaseAddr + 0x63378), 2);
            }
            else
            {
                mem::Patch((BYTE*)(modBaseAddr + 0x63378), (BYTE*)"\xFF\x08", 2);
            }
        }
        if (GetAsyncKeyState(VK_NUMPAD5) & 0x01) {
            bRecoil = !bRecoil;
            printInfo(bInvincible, bFlying, bAmmo, bGrenades, bRecoil, bRapidFire, bOneShot, bAutoFire, bDrone);
            if (bRecoil)
            {
                mem::Nop((BYTE*)(modBaseAddr + 0x63786), 10);
            }
            else
            {
                mem::Patch((BYTE*)(modBaseAddr + 0x63786), (BYTE*)"\x50\x8D\x4C\x24\x1C\x51\x8B\xCE\xFF\xD2", 10);
            }
        }
        if (GetAsyncKeyState(VK_NUMPAD6) & 0x01) {
            bRapidFire = !bRapidFire;
            printInfo(bInvincible, bFlying, bAmmo, bGrenades, bRecoil, bRapidFire, bOneShot, bAutoFire, bDrone);
            if (bRapidFire)
            {
                mem::Patch((BYTE*)(modBaseAddr + 0x637DA), (BYTE*)rapidJump,7);
            }
            else
            {
                mem::Patch((BYTE*)(modBaseAddr + 0x637DA), (BYTE*)"\x0F\xBF\x88\x0A\x01\x00\x00", 7);
            }
        }
        if (GetAsyncKeyState(VK_NUMPAD7) & 0x01) {
            bOneShot = !bOneShot;
            printInfo(bInvincible, bFlying, bAmmo, bGrenades, bRecoil, bRapidFire, bOneShot, bAutoFire, bDrone);
            if (bOneShot)
            {
                mem::Patch((BYTE*)(modBaseAddr + 0x613C9), (BYTE*)oneShotJump, 7);
            }
            else
            {
                mem::Patch((BYTE*)(modBaseAddr + 0x613C9), (BYTE*)"\x0F\xBF\x99\x0c\x01\x00\x00", 7);
            }
        }
        if (GetAsyncKeyState(VK_NUMPAD8) & 0x01) {
            bAutoFire = !bAutoFire;
            printInfo(bInvincible, bFlying, bAmmo, bGrenades, bRecoil, bRapidFire, bOneShot, bAutoFire, bDrone);
            if (bAutoFire)
            {
                mem::Patch((BYTE*)(modBaseAddr + 0x6371B), (BYTE*)autoFireJump, 6);
            }
            else
            {
                mem::Patch((BYTE*)(modBaseAddr + 0x6371B), (BYTE*)"\x88\x98\x24\x02\x00\x00", 6);
            }
        }
        if (GetAsyncKeyState(VK_NUMPAD9) & 0x01) {
            bDrone = !bDrone;
            printInfo(bInvincible, bFlying, bAmmo, bGrenades, bRecoil, bRapidFire, bOneShot, bAutoFire, bDrone);
            if (bDrone)
            {
                localPlayer->CurrentHeight = 25;
                localPlayer->BaseHeight = 25;
            }
            else
            {
                localPlayer->CurrentHeight = 4.5;
                localPlayer->BaseHeight = 4.5;
            }
        }
        if (GetAsyncKeyState(VK_END) & 0x01) {
            break;
        }


        // Freeze Values
        Sleep(5);
    }

    // remove hooks
    mem::Patch((BYTE*)(modBaseAddr + 0x613C9), (BYTE*)"\x0F\xBF\x99\x0c\x01\x00\x00", 7);
    mem::Patch((BYTE*)(modBaseAddr + 0x637DA), (BYTE*)"\x0F\xBF\x88\x0A\x01\x00\x00", 7);
    mem::Patch((BYTE*)(modBaseAddr + 0x6371B), (BYTE*)"\x88\x98\x24\x02\x00\x00", 6);
    mem::Patch((BYTE*)(modBaseAddr + 0x29D1F), (BYTE*)"\x29\x7B\x04\x8B\xC7", 5);
    mem::Patch((BYTE*)(modBaseAddr + 0x25810), (BYTE*)"\x75\x18\x6A\x0A\xE8", 5);
    // Clean up
    fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

