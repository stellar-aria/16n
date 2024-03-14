#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include "config.h"

struct Config {
  /// The location of the config data in both the EEPROM
  /// and the SysEx message (after manufacturer/device prelude)
  enum Address {
    LED_POWER = 0,   // bool
    LED_DATA = 1,    // bool
    ROTATE = 2,      // bool
    I2C_MASTER = 3,  // bool

    // FADER MIN VALUE
    FADERMIN_LSB = 4,
    FADERMIN_MSB = 5,

    // FADER MAX VALUE
    FADERMAX_LSB = 6,
    FADERMAX_MSB = 7,

    MIDI_THRU = 8,  // bool

    MIDI_USB_CHANNEL = 16,  // 16x uint8_t
    MIDI_TRS_CHANNEL = 32,  // 16x uint8_t
    MIDI_USB_CC = 48,       // 16x uint8_t
    MIDI_TRS_CC = 64,       // 16x uint8_t
  };
  constexpr static size_t DEVICE_CONFIG_SIZE = MIDI_USB_CHANNEL;  // the size of a device config block
  constexpr static size_t MIDI_CONFIG_SIZE = 16;                  // the size of a midi config block
  constexpr static size_t SIZE = MIDI_TRS_CC + 16;

  // MIDI Channel to send each fader data on
  std::array<uint8_t, kNumChannels> usb_channels;
  std::array<uint8_t, kNumChannels> trs_channels;

  // CC codes to send for each fader
  std::array<uint8_t, kNumChannels> usb_ccs;
  std::array<uint8_t, kNumChannels> trs_ccs;

  std::array<uint8_t, kNumChannels> legacy_ports;  // for V125 only

  bool rotate;
  bool led_power;
  bool led_data;
  bool i2c_master;
  bool midi_thru;

  // Fader limits
  uint16_t fader_min;
  uint16_t fader_max;

 public:
  void Check();
  void FactoryReset();
  void Load();
};

extern Config config;
