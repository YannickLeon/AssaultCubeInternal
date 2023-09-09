#pragma once

#include "Vector3.h"
#include "Weapon.h"

struct playerEnt
{
    DEFINE_MEMBER_N(Vector3, HeadPos, 4);// 0x4
    DEFINE_MEMBER_N(Vector3, Position, 36);// 0x34
    Vector3 Rotation; // 0x40
    DEFINE_MEMBER_N(float, Width, 12); // 0x58
    float CurrentHeight; // 0x5C
    float BaseHeight; // 0x60
    float BaseHeight2; // 0x64
    DEFINE_MEMBER_N(int, Health, 144); // 0xF8
    int Armor; // 0xFC
    DEFINE_MEMBER_N(Weapon*, CurrentWeapon, 628); // 374
};