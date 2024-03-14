#pragma once
#include <Arduino.h>
#include <span>

namespace sysex {
enum class InboundMessageType {
  EDIT_CONFIG_TRS = 0x0B,     // 0B - c0nfig trs edit - here is a new config just for trs
  EDIT_CONFIG_USB = 0x0C,     // 0C - c0nfig usb edit - here is a new config just for usb
  EDIT_CONFIG_DEVICE = 0x0D,  // 0D - c0nfig Device edit - new config just for device opts
  EDIT_CONFIG = 0x0E,         // 0E - c0nfig Edit - here is a new config
  INITIALIZE = 0x1A,          // 1A - 1nitiAlize - blank EEPROM and reset to factory settings.
  REQUEST_INFO = 0x1F,        // 1F = "1nFo" - please send me your current config
};

struct OutboundMessageType {
  enum {
    CONFIG = 0x0F,  // 0F - "c0nFig" - outputs its config:
  };
};

struct Layout {
  enum {

  };
};

void Parse(uint8_t* sysexData, size_t size);
}  // namespace sysex
