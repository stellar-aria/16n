/*
 * 16n Faderbank EEPROM-based configuration
 * (c) 2020 by Tom Armitage
 * MIT License
 */
#include "configuration.hpp"
#include <Arduino.h>
#include <EEPROM.h>
#include <array>
#include "utils.hpp"

constexpr std::array default_ccs = {32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47};

void Config::Check() {
  // if byte1 of EEPROM is FF for whatever reason, let's assume the machine needs initializing
  int firstByte = EEPROM.read(0x00);

  if (firstByte > 0x01) {
    DEBUG_PRINTLN("First Byte is > 0x01, probably needs initialising");
    FactoryReset();
    return;
  }

  DEBUG_PRINTF("First Byte is set to: %02X\n", firstByte);
  std::array buffer = eeprom::read<Config::SIZE>();

  DEBUG_PRINTLN("Config found:");
  debug::printArray(buffer);
}

void Config::FactoryReset() {
  // set default config flags (LED POWER, LED DATA, ROTATE, etc)
  // fadermin/max are based on "works for me" for twra2. Your mileage may vary.
  EEPROM.write(Config::LED_POWER, 1);      // LED POWER
  EEPROM.write(Config::LED_DATA, 1);       // LED DATA
  EEPROM.write(Config::ROTATE, 0);         // ROTATE
  EEPROM.write(Config::I2C_MASTER, 0);     // I2C follower by default
  EEPROM.write(Config::FADERMIN_LSB, 15);  // fadermin LSB
  EEPROM.write(Config::FADERMIN_MSB, 0);   // fadermin MSB
  EEPROM.write(Config::FADERMAX_LSB, 71);  // fadermax LSB
  EEPROM.write(Config::FADERMAX_MSB, 63);  // fadermax MSB
  EEPROM.write(Config::MIDI_THRU, 0);      // Soft midi thru

  // blank remaining config slots.
  for (size_t i = Config::MIDI_THRU; i < Config::DEVICE_CONFIG_SIZE; i++) {
    EEPROM.write(i, 0);
  }

  // set default MIDI values
  for (int i = 0; i < kNumChannels; i++) {
    // All sliders to Midi CH 1
    EEPROM.write(Config::MIDI_USB_CHANNEL + i, 1);
    EEPROM.write(Config::MIDI_TRS_CHANNEL + i, 1);

    // Use default CC values
    EEPROM.write(Config::MIDI_USB_CC + i, default_ccs[i]);
    EEPROM.write(Config::MIDI_TRS_CC + i, default_ccs[i]);
  }

  // serial dump that config.
  std::array buffer = eeprom::read<Config::SIZE>();
  DEBUG_PRINTLN("Config Instantiated.");
  debug::printArray(buffer);
}

void Config::Load() {
  for (int i = 0; i < kNumChannels; i++) {
    usb_channels[i] = EEPROM.read(Config::MIDI_USB_CHANNEL + i);  // load usb channels
    trs_channels[i] = EEPROM.read(Config::MIDI_TRS_CHANNEL + i);  // load TRS channels

    usb_ccs[i] = EEPROM.read(Config::MIDI_USB_CC + i);  // load USB ccs
    trs_ccs[i] = EEPROM.read(Config::MIDI_TRS_CC + i);  // load TRS ccs
  }

  DEBUG_PRINTLN("USB Channels loaded:");
  debug::printArray(usb_channels);

  DEBUG_PRINTLN("TRS Channels loaded:");
  debug::printArray(trs_channels);

  DEBUG_PRINTLN("USB CCs loaded:");
  debug::printArray(usb_ccs);

  DEBUG_PRINTLN("TRS CCs loaded:");
  debug::printArray(trs_ccs);

  // load other config
  led_power = EEPROM.read(Config::LED_POWER);
  led_data = EEPROM.read(Config::LED_DATA);
  rotate = EEPROM.read(Config::ROTATE);
  midi_thru = EEPROM.read(Config::MIDI_THRU);

  // i2c_master only read at startup
  int faderminLSB = EEPROM.read(Config::FADERMIN_LSB);
  int faderminMSB = EEPROM.read(Config::FADERMIN_MSB);

  DEBUG_PRINT("Setting fadermin to ");
  DEBUG_PRINTLN((faderminMSB << 7) + faderminLSB);
  fader_min = (faderminMSB << 7) + faderminLSB;

  int fadermaxLSB = EEPROM.read(Config::FADERMAX_LSB);
  int fadermaxMSB = EEPROM.read(Config::FADERMAX_MSB);

  DEBUG_PRINT("Setting fadermax to ");
  DEBUG_PRINTLN((fadermaxMSB << 7) + fadermaxLSB);
  fader_max = (fadermaxMSB << 7) + fadermaxLSB;
}
