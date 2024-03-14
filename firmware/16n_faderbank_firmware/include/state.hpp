#pragma once

/// @brief Represents the runtime state of the device
struct State {
  bool has_er301 = false;
  bool has_ansible = false;
  bool has_txo = false;

  // the current value of the faders
  std::array<int, kNumChannels> current;

  // memory of the last unshifted value
  std::array<int, kNumChannels> last;

  struct Message {
    bool should_send = false;
    uint32_t send_at = 0;
  };

  Message forced_control_update{};
};

extern State state;
