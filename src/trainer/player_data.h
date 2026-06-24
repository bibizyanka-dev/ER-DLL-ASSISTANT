#pragma once

#include <cstddef>
#include <string>
#include <cstdint>
#include <vector>

struct AOBEntry {
    std::string name;
    std::string pattern;
    int offset;
    int additional;
};

struct GameAOBAddresses {
  uintptr_t WorldChrMan = 0;
  uintptr_t GameDataMan = 0;
};

struct PlayerAttributes {
  int vigor = 0;
  int mind = 0;
  int endurance = 0;
  int strangth = 0;
  int dexterity = 0;
  int intelligence = 0;
  int faith = 0;
  int arcane = 0;
};

struct Player {
  int lvl = 0;
  int health = 0;
  int maxHealth = 0;
  int mana = 0;
  int maxMana = 0;
  int stamina = 0;
  int maxStamina = 0;
  int runes = 0;
  bool godMode = false;
  bool infHealth = false;
  bool infMana = false;
  bool infStamina = false;
  PlayerAttributes attributes;
  GameAOBAddresses aobBases;
};

struct RuntimeAddresses {
  uintptr_t hModule = 0;
  uintptr_t lvl = 0;
  uintptr_t health = 0;
  uintptr_t maxHealth = 0;
  uintptr_t mana = 0;
  uintptr_t maxMana = 0;
  uintptr_t stamina = 0;
  uintptr_t maxStamina = 0;
  uintptr_t runes = 0;
  uintptr_t vigor = 0;
  uintptr_t mind = 0;
  uintptr_t endurance = 0;
  uintptr_t strangth = 0;
  uintptr_t dexterity = 0;
  uintptr_t intelligence = 0;
  uintptr_t faith = 0;
  uintptr_t arcane = 0;
};

namespace GameOffsets {
inline constexpr uintptr_t kGameDataMan = 0x03D5DF38;
inline constexpr uintptr_t kCSChrDataModule[] = {0x10EF8, 0x0, 0x190, 0x0};
inline constexpr uintptr_t kHealth[] = {0x138};
inline constexpr uintptr_t kMaxHealth[] = {0x144};
inline constexpr uintptr_t kMana[] = {0x148};
inline constexpr uintptr_t kMaxMana[] = {0x14C};
inline constexpr uintptr_t kStamina[] = {0x154};
inline constexpr uintptr_t kMaxStamina[] = {0x15C};
inline constexpr uintptr_t kLvl[] = {0x8, 0x68};
inline constexpr uintptr_t kRunes[] = {0x8, 0x6C};
inline constexpr uintptr_t kVigor[] = {0x8, 0x3C};
inline constexpr uintptr_t kMind[] = {0x8, 0x40};
inline constexpr uintptr_t kEndurance[] = {0x8, 0x44};
inline constexpr uintptr_t kStrength[] = {0x8, 0x48};
inline constexpr uintptr_t kDexterity[] = {0x8, 0x4C};
inline constexpr uintptr_t kIntelligence[] = {0x8, 0x50};
inline constexpr uintptr_t kFaith[] = {0x8, 0x54};
inline constexpr uintptr_t kArcane[] = {0x8, 0x58};
}  // namespace GameOffsets


// local aobList = {
//     {name = "WorldChrMan", aob = "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 0F 48 39 88", offset = 3, additional = 7},
//     {name = "GameDataMan", aob = "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 05 48 8B 40 58 C3 C3", offset = 3, additional = 7},
//     {name = "NetManImp", aob = "48 8B 05 ???????? 80 78 ?? 00 ???? 48 8D 9F ???????? 48 8B 03", offset = 3, additional = 7},
//     {name = "CSRegulationManagerImp", aob = "48 8B 0D ? ? ? ? 48 85 C9 74 0B 4C 8B C0 48 8B D7", offset = 3, additional = 7},
//     {name = "PARAM", aob = "48 8B 0D ?? ?? ?? ?? 48 85 C9 0F 84 ?? ?? ?? ?? 45 33 C0 BA 8E 00 00 00", offset = 3, additional = 7},
//     {name = "EventFlagMan", aob = "48 8B 3D ???????? 48 85 FF ???? 32 C0 E9", offset = 3, additional = 7},
//     {name = "FieldArea", aob = "48 8B 0D ?? ?? ?? ?? 48 ?? ?? ?? 44 0F B6 61 ?? E8 ?? ?? ?? ?? 48 63 87 ?? ?? ?? ?? 48 ?? ?? ?? 48 85 C0", offset = 3, additional = 7},
//     {name = "MapItemMan", aob = "48 8B 0D ???????? C7 44 24 50 FF FF FF FF C7 45 A0 FF FF FF FF 48 85 C9 75 2E", offset = 3, additional = 7},
//     {name = "CSFlipper", aob = "48 8B 0D ???????? 80 BB D7 00 00 00 00 0F 84 CE 00 00 00 48 85 C9 75 2E", offset = 3, additional = 7},
//     {name = "GameMan", aob = "48 8B 05 ???????? 80 B8 ???????? 0D 0F94 C0 C3", offset = 3, additional = 7},
//     {name = "CSLuaEventManager", aob = "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ?? 41 BE 01 00 00 00 44 89 75", offset = 3, additional = 7},
//     {name = "hudngaddr", aob = "8B 7B 64 48 85 C9", offset = -4, additional = 0},
//     {name = "DamageCtrl", aob = "48 8B 05 ???????? 49 8B D9 49 8B F8 48 8B F2 48 85 C0 75 2E", offset = 3, additional = 7},
//     {name = "MapLight", aob = "48 8B FA 0F 28 05 ?? ?? ?? ?? 48 8B D9 66 0F 7F 45 C7", offset = 6, additional = 10},
//     {name = "CHR_DBG_FLAGS", aob = "80 3D ?? ?? ?? ?? 00 0F 85 ?? ?? ?? ?? 32 C0 48", offset = 2, additional = 7},
//     {name = "CHR_DBG", aob = "48 8B 05 ?? ?? ?? ?? 41 83 FF 02 ?? ?? 48 85 C0", offset = 3, additional = 7},
//     {name = "EmkSystem", aob = "48 8B 05 ???????? 4C 8B 74 24 ?? 48 8B 7C 24 ?? 48 8B 74 24 ?? 48", offset = 3, additional = 7},
//     {name = "MsbPointMan", aob = "48 8B 0D ???????? 41 B0 01 BA 23000000 E8 ???????? 84 C0", offset = 3, additional = 7},
//     {name = "WorldMapMan", aob = "48 8B 0D ???????? E8 ???????? 0FB6 5D 90 84 C0 41 0F44 DD", offset = 3, additional = 7},
//     {name = "WorldHitMan", aob = "48 8B 05 ?? ?? ?? ?? 48 8D 4C 24 ?? 4889 4c 24 ?? 0F 10 44 24 70", offset = 3, additional = 7},
//     {name = "WorldNaviMeshManager", aob = "48 8B 0D ?? ?? ?? ?? 0F B6 84 24 ?? ?? ?? ?? 4C 8D 8C 24 ?? ?? ?? ?? F3 0F 10 05", offset = 3, additional = 7},
//     {name = "WorldGeomMan", aob = "4C 39 3D ?? ?? ?? ?? 0F 84 ?? ?? ?? ?? 4C 89 60 ?? 41 83 CC FF 4C 89 70 ?? 0F 29 ?? ?? 44 0F 29 ?? ?? F3", offset = 3, additional = 7},
//     {name = "WorldTalkMan", aob = "48 8B 05 ???????? F3 0F 10 88 ???????? 0F 57 C0 48 8B 47", offset = 3, additional = 7},
//     {name = "WorldWaypointMan", aob = "48 8B 35 ???????? 49 8B 06 48 8B FE 48 8B D8", offset = 3, additional = 7},
//     {name = "WorldObjActMan", aob = "48 8B 0D ???????? E8 ???????? 48 8B 5F ?? 48 89 5F ?? 48 8B 6C 24 ??", offset = 3, additional = 7},
//     {name = "WorldSfxMan", aob = "48 8B 05 ???????? 48 8D 4D 98 48 89 4C 24 60", offset = 3, additional = 7},
//     {name = "WorldSoundMan", aob = "48 8B 05 ???????? 48 8D 4D 30 48 89 4C 24 38 0F 10", offset = 3, additional = 7},
//     {name = "WorldAiMan", aob = "48 8B 0D ???????? 4C 8D 44 24 38 B2 07 E8 ???????? C7", offset = 3, additional = 7},
//     {name = "WorldAreaWeather", aob = "48 8B 15 ???????? 32 C0 48 85 D2 ???? 8B 82", offset = 3, additional = 7},
//     {name = "WorldAreaTime", aob = "48 8B 05 ???????? 48 85 C0 ???? C6 40 ?? 01 48 8B 05", offset = 3, additional = 7},
//     {name = "ChrSpawnCmpAddr", aob = "80 3D xx xx xx xx 00 0F 28 F0 74 xx 0F 57 C9", offset = 2, additional = 7},
//     {name = "Bullet_Man", aob = "48 8B 0D xx xx xx xx E8 xx xx xx xx 48 8D 44 24 xx 48  89  44  24 xx 48 89 7C 24 xx C7 44 24 xx x xx xx xx 48", offset = 3, additional = 7},
//     {name = "CsDlc", aob = "48 83 3D ?? ?? ?? ?? 00 75 27 48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 4C 8B C8 4C 8D 05 ?? ?? ?? ?? BA B4 00 00 00 48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? C6 40 42 01 BA 01 00 00 00 41 B8 F4 01 00 00 48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? EB 02", offset = 3, additional = 8}
// }
