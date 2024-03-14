#pragma once
#include "configuration.hpp"

namespace MIDI {
constexpr static int interval = 1000;  // 1ms

constexpr uint32_t flash_duration = 50;
extern uint32_t last_activity_at;

void Setup();
void Start();
void Read();
void Write();

bool get_and_clear_activity();
void force_write();
};  // namespace MIDI
