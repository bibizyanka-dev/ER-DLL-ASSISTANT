#include "player_trainer.h"

#include <windows.h>

#include <cstdint>

#include "../../ImGui/imgui.h"
#include "../../src/utils/logger.h"
#include "aob_scanner.h"
#include "memory.h"
#include "player_data.h"

PlayerTrainer& PlayerTrainer::Instance() {
  static PlayerTrainer instance;
  return instance;
}

bool PlayerTrainer::EnsureAOBBases(HMODULE moduleBase) {
  if (aobBasesResolved_) {
    return player_.aobBases.WorldChrMan != 0;
  }

  auto& registry = GetAOBRegistry();
  if (!registry.EnsureScanned(moduleBase)) {
    return false;
  }

  registry.ApplyTo(player_.aobBases);
  aobBasesResolved_ = true;

  if (player_.aobBases.WorldChrMan) {
    Log("[+] AOB bases resolved");
  }

  return player_.aobBases.WorldChrMan != 0;
}

void PlayerTrainer::ClearRuntimeAddresses() {
  const uintptr_t module = addresses_.hModule;
  addresses_ = RuntimeAddresses{};
  addresses_.hModule = module;
  runtimeAddressesValid_ = false;
}

bool PlayerTrainer::ResolveRuntimeAddresses(HMODULE moduleBase) {
  if (!player_.aobBases.WorldChrMan) {
    ClearRuntimeAddresses();
    return false;
  }

  const uintptr_t module = reinterpret_cast<uintptr_t>(moduleBase);
  const uintptr_t GameDataManAddr = module + GameOffsets::kGameDataMan;
  const uintptr_t csChrDataModuleAddr = Memory::ResolvePointerChain(player_.aobBases.WorldChrMan, GameOffsets::kCSChrDataModule);

  uintptr_t playerInsAddr = 0;
  if (!Memory::Read(csChrDataModuleAddr, &playerInsAddr, sizeof(uintptr_t))) {
    ClearRuntimeAddresses();
    return false;
  }

  if (playerInsAddr == 0 || playerInsAddr == 0xFFFFFFFFFFFFFFFFULL) {
    ClearRuntimeAddresses();
    return false;
  }

  addresses_.hModule = module;
  addresses_.health = Memory::ResolvePointerChain(csChrDataModuleAddr, GameOffsets::kHealth);
  addresses_.maxHealth = Memory::ResolvePointerChain(csChrDataModuleAddr, GameOffsets::kMaxHealth);
  addresses_.mana = Memory::ResolvePointerChain(csChrDataModuleAddr, GameOffsets::kMana);
  addresses_.maxMana = Memory::ResolvePointerChain(csChrDataModuleAddr, GameOffsets::kMaxMana);
  addresses_.stamina = Memory::ResolvePointerChain(csChrDataModuleAddr, GameOffsets::kStamina);
  addresses_.maxStamina = Memory::ResolvePointerChain(csChrDataModuleAddr, GameOffsets::kMaxStamina);
  addresses_.lvl = Memory::ResolvePointerChain(GameDataManAddr, GameOffsets::kLvl);
  addresses_.runes = Memory::ResolvePointerChain(GameDataManAddr, GameOffsets::kRunes);
  addresses_.vigor = Memory::ResolvePointerChain(GameDataManAddr, GameOffsets::kVigor);
  addresses_.mind = Memory::ResolvePointerChain(GameDataManAddr, GameOffsets::kMind);
  addresses_.endurance = Memory::ResolvePointerChain(GameDataManAddr, GameOffsets::kEndurance);
  addresses_.strangth = Memory::ResolvePointerChain(GameDataManAddr, GameOffsets::kStrength);
  addresses_.dexterity = Memory::ResolvePointerChain(GameDataManAddr, GameOffsets::kDexterity);
  addresses_.intelligence = Memory::ResolvePointerChain(GameDataManAddr, GameOffsets::kIntelligence);
  addresses_.faith = Memory::ResolvePointerChain(GameDataManAddr, GameOffsets::kFaith);
  addresses_.arcane = Memory::ResolvePointerChain(GameDataManAddr, GameOffsets::kArcane);

  if (addresses_.lvl == 0) {
    ClearRuntimeAddresses();
    return false;
  }

  runtimeAddressesValid_ = true;
  return true;
}

void PlayerTrainer::ReadPlayerState() {
  Memory::ReadInt(addresses_.lvl, &player_.lvl);
  Memory::ReadInt(addresses_.health, &player_.health);
  Memory::ReadInt(addresses_.maxHealth, &player_.maxHealth);
  Memory::ReadInt(addresses_.mana, &player_.mana);
  Memory::ReadInt(addresses_.maxMana, &player_.maxMana);
  Memory::ReadInt(addresses_.stamina, &player_.stamina);
  Memory::ReadInt(addresses_.maxStamina, &player_.maxStamina);
  Memory::ReadInt(addresses_.runes, &player_.runes);

  Memory::ReadInt(addresses_.vigor, &player_.attributes.vigor);
  Memory::ReadInt(addresses_.mind, &player_.attributes.mind);
  Memory::ReadInt(addresses_.endurance, &player_.attributes.endurance);
  Memory::ReadInt(addresses_.strangth, &player_.attributes.strangth);
  Memory::ReadInt(addresses_.dexterity, &player_.attributes.dexterity);
  Memory::ReadInt(addresses_.intelligence, &player_.attributes.intelligence);
  Memory::ReadInt(addresses_.faith, &player_.attributes.faith);
  Memory::ReadInt(addresses_.arcane, &player_.attributes.arcane);
}

void PlayerTrainer::MemoryUpdate(HMODULE moduleBase) {
  if (!moduleBase) {
    Log("[-] eldenring.exe module not found");
    return;
  }

  if (!EnsureAOBBases(moduleBase)) {
    return;
  }

  if (!ResolveRuntimeAddresses(moduleBase)) {
    return;
  }

  ReadPlayerState();
}

void PlayerTrainer::ChangeAttributeValue(char symbol, uintptr_t attrAddress) {
  if (!runtimeAddressesValid_ || !attrAddress || !addresses_.lvl) {
    return;
  }

  int value = 0;
  int lvl = 0;
  if (!Memory::ReadInt(attrAddress, &value) || !Memory::ReadInt(addresses_.lvl, &lvl)) {
    return;
  }

  if (symbol == '+') {
    Memory::WriteInt(attrAddress, value + 1);
    Memory::WriteInt(addresses_.lvl, lvl + 1);
  } else if (symbol == '-') {
    Memory::WriteInt(attrAddress, value - 1);
    Memory::WriteInt(addresses_.lvl, lvl - 1);
  }
}

void PlayerTrainer::PlayerUpdate() {
  if (!runtimeAddressesValid_) {
    return;
  }

  if (player_.infHealth) {
    Memory::WriteInt(addresses_.health, player_.maxHealth);
  }
  if (player_.infMana) {
    Memory::WriteInt(addresses_.mana, player_.maxMana);
  }
  if (player_.infStamina) {
    Memory::WriteInt(addresses_.stamina, player_.maxStamina);
  }
}

void PlayerTrainer::RenderModesTab() {
  if (ImGui::Checkbox("God Mode", &player_.godMode)) {
    player_.infHealth = player_.godMode;
    player_.infMana = player_.godMode;
    player_.infStamina = player_.godMode;
  }

  if (ImGui::Checkbox("Infinity Health", &player_.infHealth)) {
  }

  if (ImGui::Checkbox("Infinity Mana", &player_.infMana)) {
  }

  if (ImGui::Checkbox("Infinity Stamina", &player_.infStamina)) {
  }
}

void PlayerTrainer::RenderPlayerTab() {
  ImGui::Text("Lvl: %d", player_.lvl);
  ImGui::Text("Health: %d/%d", player_.health, player_.maxHealth);
  ImGui::Text("Mana: %d/%d", player_.mana, player_.maxMana);
  ImGui::Text("Stamina: %d/%d", player_.stamina, player_.maxStamina);
  ImGui::Text("Runes: %d", player_.runes);

  ImGui::Separator();

  ImGui::InputInt("Add Runes", &runesToAdd_);

  if (ImGui::Button("Add")) {
    int runes = 0;
    if (Memory::ReadInt(addresses_.runes, &runes)) {
      Memory::WriteInt(addresses_.runes, runes + runesToAdd_);
    }
  }

  ImGui::SameLine();

  if (ImGui::Button("Remove")) {
    int runes = 0;
    if (Memory::ReadInt(addresses_.runes, &runes)) {
      Memory::WriteInt(addresses_.runes, runes - runesToAdd_);
    }
  }

  ImGui::Separator();

  if (ImGui::BeginTable("AttributesTable", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)) {
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Value");
    ImGui::TableSetupColumn("Actions");

    auto row = [&](const char* name, int& value, uintptr_t addr) {
      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted(name);

      ImGui::TableNextColumn();
      ImGui::Text("%d", value);

      ImGui::TableNextColumn();
      ImGui::PushID(name);

      if (ImGui::SmallButton("+")) {
        ChangeAttributeValue('+', addr);
      }

      ImGui::SameLine();

      if (ImGui::SmallButton("-")) {
        ChangeAttributeValue('-', addr);
      }

      ImGui::PopID();
    };

    row("Vigor", player_.attributes.vigor, addresses_.vigor);
    row("Mind", player_.attributes.mind, addresses_.mind);
    row("Endurance", player_.attributes.endurance, addresses_.endurance);
    row("Strength", player_.attributes.strangth, addresses_.strangth);
    row("Dexterity", player_.attributes.dexterity, addresses_.dexterity);
    row("Intelligence", player_.attributes.intelligence, addresses_.intelligence);
    row("Faith", player_.attributes.faith, addresses_.faith);
    row("Arcane", player_.attributes.arcane, addresses_.arcane);

    ImGui::EndTable();
  }
}
