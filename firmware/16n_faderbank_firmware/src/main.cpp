/*
 * 16n Faderbank Firmware
 * (c) 2017,2018,2020 by Brian Crabtree, Sean Hellfritsch, Tom Armitage, and Brendon Cassidy
 * MIT License
 */

/*
 * NOTES:
 * - Hardware MIDI is on pin 1
 * - You **must** also compile this with Tools->USB type set to MIDI or MIDI/Serial (for debugging)
 * - You also should overclock to 120MHz to make it as snappy as possible
 */

/*
 * Most configuration now hpapens via online editor.
 * config.h is mainly for developer configuration.
 */
#include <CD74HC4067.h>
#include <EEPROM.h>
#include <ResponsiveAnalogRead.h>
#include <algorithm>
#include "TxHelper.hpp"
#include "config.h"
#include "configuration.hpp"
#include "i2c.hpp"
#include "midi.hpp"
#include "state.hpp"
#include "sysex.hpp"

constexpr auto kNumBitsADC = 13;  // 13 bit ADC resolution on Teensy 3.2
constexpr int LED_PIN = 13;

// variables to hold configuration
Config config{};
State state{};

// Input smoothers
std::array<ResponsiveAnalogRead, kNumChannels> analog;

// mux config
CD74HC4067 mux{8, 7, 6, 5};
constexpr std::array mux_map = {0, 1, 2, 3, 4, 5, 6, 7, 15, 14, 13, 12, 11, 10, 9, 8};

/*
 * The function that sets up the application
 */
void setup() {
  DEBUG_PRINTLN("16n Firmware Debug Mode");

  config.Check();
  config.Load();
  config.i2c_master = EEPROM.read(Config::I2C_MASTER);

  if (config.i2c_master) {
    delay(BOOTDELAY);
  }

  if constexpr (V125) {
    // analog ports on the Teensy for the 1.25 board.
    std::array ports = std::array{A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15};

    if (config.rotate) {
      std::reverse(ports.begin(), ports.end());
    }

    for (int i = 0; i < kNumChannels; i++) {
      config.legacy_ports[i] = ports[i];
    }
  }

  // initialize the TX Helper
  TxHelper::UseWire1(V125);
  TxHelper::SetPorts(16);
  TxHelper::SetModes(4);

  // set read resolution to teensy's 13 usable bits
  analogReadResolution(kNumBitsADC);

  // initialize the analog reader
  for (auto& reader : analog) {
    reader = ResponsiveAnalogRead(0, true, .0001);
    reader.setAnalogResolution(1 << kNumBitsADC);

    // ResponsiveAnalogRead is designed for 10-bit ADCs
    // meanining its threshold defaults to 4. Let's bump that for
    // our 13-bit adc by setting it to 4 << (13-10)
    reader.setActivityThreshold(32);
  }

  i2c::Setup();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, config.led_power);
}

int ReadChannel(int channel) {
  if constexpr (V125) {
    return analogRead(config.legacy_ports[channel]);
  }

  // set mux to appropriate channel
  auto mux_channel = config.rotate ? kNumChannels - channel - 1 : channel;
  mux.channel(mux_map[mux_channel]);

  // wait for the mux channel to change
  delayMicroseconds(10);

  // read the value
  return analogRead(0);  // mux goes into A0
}

/*
 * The main read loop that goes through all of the sliders
 */
void loop() {
  // this whole chunk makes the LED flicker on MIDI activity -
  // and inverts that flicker if the power light is on.
  if (config.led_data) {
    if (millis() > (MIDI::last_activity_at + MIDI::flash_duration)) {
      digitalWrite(LED_PIN, config.led_power);
      MIDI::get_and_clear_activity();
    }
    else {
      digitalWrite(LED_PIN, !config.led_power);
    }
  }
  else {
    digitalWrite(LED_PIN, config.led_power);
  }

  if (state.forced_control_update.should_send && (millis() > state.forced_control_update.send_at)) {
    // it's now time to send a forced control update, so...
    state.forced_control_update.should_send = false;  // mark message as sent

    MIDI::force_write();  // force a write the next time the Midi::Write callback fires.
  }

  for (int i = 0; i < kNumChannels; i++) {
    const int raw = ReadChannel(i);

    // put the value into the smoother
    analog[i].update(raw);

    if (analog[i].hasChanged()) {
      // read from the smoother, constrain (to account for tolerances), and map it
      uint16_t value = analog[i].getValue();
      value = std::clamp(value, config.fader_min, config.fader_max);
      value = map(value, config.fader_min, config.fader_max, 0, 16383);

      if (config.rotate) {
        value = 16383 - value;
      }

      // map and update the value
      state.current[i] = value;
    }
  }

  MIDI::Read();
  MIDI::Write();
}
