#include "midi.hpp"

#include <Arduino.h>
#include <MIDI.h>
#include "configuration.hpp"
#include "i2c.hpp"
#include "state.hpp"
#include "sysex.hpp"

midi::SerialMIDI<HardwareSerial> serialserialMIDI{Serial1};
midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> serialMIDI{
    static_cast<midi::SerialMIDI<HardwareSerial>&>(serialserialMIDI)};

static bool needs_write = false;
static bool needs_read = false;

static bool force_write_ = false;

static bool had_activity = false;

// MIDI timers
static IntervalTimer write_timer;
static IntervalTimer read_timer;

static std::array<int, kNumChannels> history;

namespace MIDI {

uint32_t last_activity_at;

void force_write() {
  force_write_ = true;
}

bool get_and_clear_activity() {
  bool value = had_activity;
  had_activity = false;
  return value;
}

void ReadInternal();
void WriteInternal();

void Setup() {
  usbMIDI.setHandleSystemExclusive(sysex::Parse);
  usbMIDI.setHandleRealTimeSystem([](uint8_t realtimebyte) {
    serialMIDI.sendRealTime(static_cast<midi::MidiType>(realtimebyte));  //<
  });

  if (config.midi_thru) {
    usbMIDI.setHandleNoteOff([](uint8_t channel, uint8_t note, uint8_t velocity) {  //<
      serialMIDI.sendNoteOff(note, velocity, channel);
    });

    usbMIDI.setHandleNoteOn([](uint8_t channel, uint8_t note, uint8_t velocity) {  //<
      serialMIDI.sendNoteOn(note, velocity, channel);
    });

    usbMIDI.setHandleAfterTouchPoly([](uint8_t channel, uint8_t note, uint8_t velocity) {  //<
      serialMIDI.sendAfterTouch(note, velocity, channel);
    });

    usbMIDI.setHandleControlChange([](uint8_t channel, uint8_t control, uint8_t value) {  //<
      serialMIDI.sendControlChange(control, value, channel);
    });

    usbMIDI.setHandleProgramChange([](uint8_t channel, uint8_t program) {  //<
      serialMIDI.sendProgramChange(program, channel);
    });

    usbMIDI.setHandleAfterTouch([](uint8_t channel, uint8_t pressure) {  //<
      serialMIDI.sendAfterTouch(pressure, channel);
    });

    usbMIDI.setHandleTimeCodeQuarterFrame([](uint8_t data) {  //<
      serialMIDI.sendTimeCodeQuarterFrame(data);
    });

    usbMIDI.setHandleSongPosition([](uint16_t beats) {  //<
      serialMIDI.sendSongPosition(beats);
    });

    usbMIDI.setHandleSongSelect([](uint8_t songNumber) {  //<
      serialMIDI.sendSongSelect(songNumber);
    });

    usbMIDI.setHandleTuneRequest([] {  //<
      serialMIDI.sendTuneRequest();
    });
  }
}

void Start() {
  // turn on the MIDI party
  serialMIDI.begin();
  write_timer.begin([] { needs_write = true; }, interval);
  read_timer.begin([] { needs_read = true; }, interval);
}

void Read() {
  if (!needs_read) {
    return;
  }

  serialMIDI.read();
  usbMIDI.read();

  noInterrupts();
  needs_read = false;
  interrupts();
}

void Write() {
  if (!needs_write) {
    return;
  }

  WriteInternal();
  noInterrupts();
  needs_write = false;
  interrupts();
}

/*
 * The function that writes changes in slider positions out the midi ports
 * Called when needs_write flag is HIGH
 */
void WriteInternal() {
  // midi write helpers
  static int shiftyTemp;
  static int notShiftyTemp;

  for (size_t c = 0; c < kNumChannels; c++) {
    notShiftyTemp = state.current[c];

    // shift for MIDI precision (0-127)
    shiftyTemp = notShiftyTemp >> 7;

    // if there was a change in the midi value
    if ((shiftyTemp != history[c]) || force_write_) {
      if (config.led_data && !had_activity) {
        last_activity_at = millis();
        had_activity = true;
      }
      // send the message over USB and physical MIDI
      usbMIDI.sendControlChange(config.usb_ccs[c], shiftyTemp, config.usb_channels[c]);
      serialMIDI.sendControlChange(config.trs_ccs[c], shiftyTemp, config.trs_channels[c]);

      // store the shifted value for future comparison
      history[c] = shiftyTemp;

      DEBUG_PRINTF("MIDI[%d]: %d\n", c, shiftyTemp);
    }

    if (config.i2c_master) {
      // we send out to all three supported i2c slave devices
      // keeps the firmware simple :)

      if (notShiftyTemp != state.last[c]) {
        DEBUG_PRINTF("i2c Master[%d]: %d\n", c, notShiftyTemp);

        // for 4 output devices
        uint8_t port = c % 4;
        uint8_t device = c / 4;

        // TXo
        if (state.has_txo) {
          i2c::Send(i2c::addresses::txo, device, 0x11, port, notShiftyTemp);
        }

        // ER-301
        if (state.has_er301) {
          i2c::Send(i2c::addresses::er301, 0, 0x11, c, notShiftyTemp);
        }

        // ANSIBLE
        if (state.has_ansible) {
          i2c::Send(0x20, device << 1, 0x06, port, notShiftyTemp);
        }

        state.last[c] = notShiftyTemp;
      }
    }
  }
  force_write_ = false;
}
}  // namespace MIDI
