#pragma once

#include "pch.h"

#include "Vector3.h"
#include "playerEnt.h"

struct WeaponStruct
{                                                                                                                                       
    char weaponName[8];
    DEFINE_MEMBER_N(int16_t, ReloadTime, 256);
    int16_t Delay;
    int16_t Damage;
    DEFINE_MEMBER_N(int16_t, HorizontalRecoil, 8);
    int16_t MagSize;
    int16_t WeaponRecoil;
    int16_t WeaponSpriteMovement;
    DEFINE_MEMBER_N(int16_t, VerticalRecoil, 4);
    DEFINE_MEMBER_N(int16_t, Autofire, 4);
};

struct Weapon
{
    DEFINE_MEMBER_N(int32_t, WeaponId, 4); // 0x04
    //playerEnt* Owner; // 0x08
    DEFINE_MEMBER_N(WeaponStruct*, Info, 4); // 0x0C
    unsigned int* AmmoPointer; // 0x10
    unsigned int* MagPointer; // 0x14
    unsigned int* DelayPtr; // 0x18
};