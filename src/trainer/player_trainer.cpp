#include "player_trainer.h"

#include <windows.h>

#include "../../ImGui/imgui.h"
#include "memory.h"
#include "player_data.h"
#include "../../src/utils/logger.h"

PlayerTrainer& PlayerTrainer::Instance() {
  static PlayerTrainer instance;
  return instance;
}

void PlayerTrainer::ResolveAddresses(HMODULE moduleBase) {
  const uintptr_t module = reinterpret_cast<uintptr_t>(moduleBase);
  const uintptr_t baseAttributeAddr = module + GameOffsets::kBasePlayerAttribute;
  const uintptr_t baseWorldChrManAddr = module + GameOffsets::kBaseWorldChrMan;
  const uintptr_t csChrDataModuleAddr = Memory::ResolvePointerChain(baseWorldChrManAddr, GameOffsets::kCSChrDataModule);

  uintptr_t playerInsAddr = 0;
  if (!Memory::Read(csChrDataModuleAddr, &playerInsAddr, sizeof(uintptr_t))) {
    return;
  }

  if (playerInsAddr == 0 || playerInsAddr == 0xFFFFFFFFFFFFFFFFULL) {
    return;
  }

  addresses_.hModule = module;
  addresses_.lvl = Memory::ResolvePointerChain(baseAttributeAddr, GameOffsets::kLvl);
  addresses_.health = Memory::ResolvePointerChain(csChrDataModuleAddr, GameOffsets::kHealth);
  addresses_.maxHealth = Memory::ResolvePointerChain(csChrDataModuleAddr, GameOffsets::kMaxHealth);
  addresses_.mana = Memory::ResolvePointerChain(csChrDataModuleAddr, GameOffsets::kMana);
  addresses_.maxMana = Memory::ResolvePointerChain(csChrDataModuleAddr, GameOffsets::kMaxMana);
  addresses_.stamina = Memory::ResolvePointerChain(csChrDataModuleAddr, GameOffsets::kStamina);
  addresses_.maxStamina = Memory::ResolvePointerChain(csChrDataModuleAddr, GameOffsets::kMaxStamina);
  addresses_.runes = Memory::ResolvePointerChain(baseAttributeAddr, GameOffsets::kRunes);
  addresses_.vigor = Memory::ResolvePointerChain(baseAttributeAddr, GameOffsets::kVigor);
  addresses_.mind = Memory::ResolvePointerChain(baseAttributeAddr, GameOffsets::kMind);
  addresses_.endurance = Memory::ResolvePointerChain(baseAttributeAddr, GameOffsets::kEndurance);
  addresses_.strangth = Memory::ResolvePointerChain(baseAttributeAddr, GameOffsets::kStrength);
  addresses_.dexterity = Memory::ResolvePointerChain(baseAttributeAddr, GameOffsets::kDexterity);
  addresses_.intelligence = Memory::ResolvePointerChain(baseAttributeAddr, GameOffsets::kIntelligence);
  addresses_.faith = Memory::ResolvePointerChain(baseAttributeAddr, GameOffsets::kFaith);
  addresses_.arcane = Memory::ResolvePointerChain(baseAttributeAddr, GameOffsets::kArcane);
}

void PlayerTrainer::ReadPlayerState() {
  player_.lvl = *reinterpret_cast<int*>(addresses_.lvl);
  player_.health = *reinterpret_cast<int*>(addresses_.health);
  player_.maxHealth = *reinterpret_cast<int*>(addresses_.maxHealth);
  player_.mana = *reinterpret_cast<int*>(addresses_.mana);
  player_.maxMana = *reinterpret_cast<int*>(addresses_.maxMana);
  player_.stamina = *reinterpret_cast<int*>(addresses_.stamina);
  player_.maxStamina = *reinterpret_cast<int*>(addresses_.maxStamina);
  player_.runes = *reinterpret_cast<int*>(addresses_.runes);

  player_.attributes.vigor = *reinterpret_cast<int*>(addresses_.vigor);
  player_.attributes.mind = *reinterpret_cast<int*>(addresses_.mind);
  player_.attributes.endurance = *reinterpret_cast<int*>(addresses_.endurance);
  player_.attributes.strangth = *reinterpret_cast<int*>(addresses_.strangth);
  player_.attributes.dexterity = *reinterpret_cast<int*>(addresses_.dexterity);
  player_.attributes.intelligence = *reinterpret_cast<int*>(addresses_.intelligence);
  player_.attributes.faith = *reinterpret_cast<int*>(addresses_.faith);
  player_.attributes.arcane = *reinterpret_cast<int*>(addresses_.arcane);
}

void PlayerTrainer::MemoryUpdate(HMODULE moduleBase) {
  if (!moduleBase) {
    Log("[-] eldenring.exe module not found");
    return;
  }

  ResolveAddresses(moduleBase);
  if (addresses_.lvl == 0) {
    return;
  }

  ReadPlayerState();
}

void PlayerTrainer::ChangeAttributeValue(char symbol, uintptr_t attrAddress) {
  if (!attrAddress) {
    return;
  }

  if (symbol == '+') {
    *reinterpret_cast<int*>(attrAddress) += 1;
    *reinterpret_cast<int*>(addresses_.lvl) += 1;
  } else if (symbol == '-') {
    *reinterpret_cast<int*>(attrAddress) -= 1;
    *reinterpret_cast<int*>(addresses_.lvl) -= 1;
  }
}

void PlayerTrainer::PlayerUpdate() {
  if (player_.infHealth) {
    *reinterpret_cast<int*>(addresses_.health) = player_.maxHealth;
  }
  if (player_.infMana) {
    *reinterpret_cast<int*>(addresses_.mana) = player_.maxMana;
  }
  if (player_.infStamina) {
    *reinterpret_cast<int*>(addresses_.stamina) = player_.maxStamina;
  }
}

void PlayerTrainer::RenderModesTab() {
  if (ImGui::Checkbox("God Mode", &player_.godMode)) {
    player_.infHealth = player_.godMode;
    player_.infMana = player_.godMode;
    player_.infStamina = player_.godMode;
  }

  if (ImGui::Checkbox("Infinity Health", &player_.infHealth));
  
  if (ImGui::Checkbox("Infinity Mana", &player_.infMana));

  if (ImGui::Checkbox("Infinity Stamina", &player_.infStamina));
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
    *reinterpret_cast<int*>(addresses_.runes) += runesToAdd_;
  }

  ImGui::SameLine();

  if (ImGui::Button("Remove")) {
    *reinterpret_cast<int*>(addresses_.runes) -= runesToAdd_;
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
