/*
 * 16n Faderbank Utility Functions
 * (c) 2020 by Tom Armitage
 * MIT License
 */
#include "utils.hpp"

namespace eeprom {

void write(std::span<uint8_t> buffer, int offset) {
  for (size_t i = 0; i < buffer.size(); i++) {
    EEPROM.write(offset + i, buffer[i]);
  }
}
}  // namespace eeprom

namespace debug {
void printHex(uint8_t num) {
  if constexpr (!DEBUG) {
    return;
  }

  std::array<char, 3> hex;
  snprintf(hex.data(), hex.size(), "%02X", num);
  Serial.print(hex.data());
}
}  // namespace debug
