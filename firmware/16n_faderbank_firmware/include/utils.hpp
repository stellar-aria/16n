#pragma once
#include <cstddef>
#include <cstdint>
#include <span>

#include <Arduino.h>
#include <EEPROM.h>

#include "config.h"

namespace eeprom {

template <size_t size>
std::array<uint8_t, size> read(int offset = 0) {
  std::array<uint8_t, size> buffer;
  for (size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = EEPROM.read(offset + i);
  }
  return buffer;
}

void write(std::span<uint8_t> buffer, int position = 0);
}  // namespace eeprom

namespace debug {
void printHex(uint8_t num);

template <typename T, size_t N>
void printArray(const std::span<T, N> array) {
  if constexpr (!DEBUG) {
    return;
  }

  for (T e : array) {
    printHex(static_cast<uint8_t>(e));
    Serial.print(" ");
  }
  Serial.println();
}

template <typename T, size_t N>
void printArray(const std::array<T, N>& array) {
  if constexpr (!DEBUG) {
    return;
  }

  printArray(std::span{array.data(), array.size()});
}
}  // namespace debug
