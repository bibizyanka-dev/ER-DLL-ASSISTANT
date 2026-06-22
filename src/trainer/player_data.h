#pragma once

#include <cstddef>
#include <cstdint>

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
  PlayerAttributes attributes;
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
inline constexpr uintptr_t kBasePlayerAttribute = 0x03D5DF38;
inline constexpr uintptr_t kBaseWorldChrMan = 0x03D65F88;
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
