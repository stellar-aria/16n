/*
 * 16n Faderbank Configuration Sysex Processing
 * (c) 2020 by Tom Armitage
 * MIT License
 */
#include "sysex.hpp"

#include <algorithm>
#include "config.h"
#include "configuration.hpp"
#include "midi.hpp"
#include "state.hpp"
#include "utils.hpp"

namespace sysex {
void UpdateConfig(Config::Address eeprom_position, std::span<byte> data) {
  // write new Data
  eeprom::write(data, eeprom_position);

  // now load that.
  config.Load();
}

void SendConfig() {
  std::array<byte, Config::SIZE + 8> sysex;

  sysex[0] = 0x7d;  // manufacturer
  sysex[1] = 0x00;
  sysex[2] = 0x00;

  sysex[3] = OutboundMessageType::CONFIG;  // ConFig;

  sysex[4] = DEVICE_ID;      // Device 01, ie, dev board
  sysex[5] = MAJOR_VERSION;  // major version
  sysex[6] = MINOR_VERSION;  // minor version
  sysex[7] = POINT_VERSION;  // point version

  // So that's 3 for the mfg + 1 for the message + 80 bytes
  // can be done with a simple "read eighty bytes and send them."
  std::array buffer = eeprom::read<Config::SIZE>();

  // Clamp our EEPROM values and stick them in the output SysEx message with the proper offset
  std::transform(buffer.begin(), buffer.end(), sysex.begin() + 8, [](byte data) {
    return std::clamp<uint8_t>(data, 0x00, 0x7F);  // 0x7F is the max a Sysex Data value can have
  });

  DEBUG_PRINTLN("Sending this data");
  debug::printArray(sysex);

  usbMIDI.sendSysEx(Config::SIZE + 8, sysex.data(), false);
  MIDI::force_write();
}

void Parse(uint8_t* sysex, size_t size) {
  DEBUG_PRINTLN("Ooh, sysex");
  debug::printArray(std::span{sysex, size});
  DEBUG_PRINTLN();

  if (size < 3) {
    DEBUG_PRINTLN("That's an empty sysex, bored now");
    return;
  }

  if (!(sysex[1] == 0x7d && sysex[2] == 0x00 && sysex[3] == 0x00)) {
    DEBUG_PRINTLN("That's not a sysex message for us");
    return;
  }

  InboundMessageType message_type{sysex[4]};

  // the SysEx data without the 5 byte header
  std::span data{sysex + 5, size - 5};

  using enum InboundMessageType;
  switch (message_type) {
    case REQUEST_INFO:
      DEBUG_PRINTLN("Got an 1nFo request");
      SendConfig();

      // also, in 1s time, please send the current state
      // of the controls, so the editor looks nice.
      state.forced_control_update.should_send = true;
      state.forced_control_update.send_at = millis() + 1000;
      break;

    case EDIT_CONFIG:
      DEBUG_PRINTLN("Incoming c0nfig Edit");
      DEBUG_PRINTF("Received a new config with size %zu\n", size);

      // The full config includes the SysEx prelude of device ID and version (4 bytes)
      // We don't need that, so remove it from the front
      UpdateConfig(Config::Address(0), data.subspan(4, 80));
      break;

    case EDIT_CONFIG_DEVICE:
      DEBUG_PRINTLN("Incoming c0nfig Device edit");
      DEBUG_PRINTF("Received a new device config with size %zu\n", size);
      UpdateConfig(Config::Address(0), data.first(Config::DEVICE_CONFIG_SIZE));
      break;

    case EDIT_CONFIG_USB:
      DEBUG_PRINTLN("Incoming c0nfig usb edit");
      UpdateConfig(Config::MIDI_USB_CHANNEL, data.first(16));   // store channels
      UpdateConfig(Config::MIDI_USB_CC, data.subspan(16, 16));  // store CCs
      break;

    case EDIT_CONFIG_TRS:
      DEBUG_PRINTLN("Incoming c0nfig trs edit");
      UpdateConfig(Config::MIDI_TRS_CHANNEL, data.first(16));   // store channels
      UpdateConfig(Config::MIDI_TRS_CC, data.subspan(16, 16));  // store CCs
      break;

    case INITIALIZE:
      DEBUG_PRINTLN("Incoming 1nitiAlize request");
      config.FactoryReset();
      config.Load();
      break;
  }
}
}  // namespace sysex
