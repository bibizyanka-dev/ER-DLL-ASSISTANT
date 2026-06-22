#pragma once

#include <windows.h>

#include <atomic>
#include <thread>

#include "player_data.h"

class PlayerTrainer {
 public:
  static PlayerTrainer& Instance();

  void Update(HMODULE moduleBase);
  void RenderTab();

  Player& GetPlayer() { return player_; }
  const RuntimeAddresses& GetAddresses() const { return addresses_; }

 private:
  PlayerTrainer() = default;

  void ResolveAddresses(HMODULE moduleBase);
  void ReadPlayerState();
  void ChangeAttributeValue(char symbol, uintptr_t attrAddress);
  void EnableGodMode();
  void StartGodMode();
  void StopGodMode();

  Player player_;
  RuntimeAddresses addresses_;
  int runesToAdd_ = 0;

  std::thread godModeThread_;
  std::atomic_bool godModeRunning_{false};
};
