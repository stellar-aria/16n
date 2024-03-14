#include "i2c.hpp"
#include <Arduino.h>
#include <i2c_t3.h>
#include <array>
#include "TxHelper.hpp"
#include "config.h"
#include "configuration.hpp"
#include "state.hpp"


#if V125
auto& wire = Wire1;
constexpr auto I2C_PINS = I2C_PINS_29_30;
#else
auto& wire = Wire;
constexpr auto I2C_PINS = I2C_PINS_29_30;
#endif

// helper values for i2c reading and future expansion
int activeInput = 0;
int activeMode = 0;

namespace i2c {

// the i2c message buffer we are sending
std::array<uint8_t, 4> messageBuffer;

void Setup() {
  // i2c using the default I2C pins on a Teensy 3.2
  if (config.i2c_master) {
    DEBUG_PRINTLN("Enabling i2c in MASTER mode");
    wire.begin(I2C_MASTER, I2C_ADDRESS, I2C_PINS, I2C_PULLUP_EXT, 400000);
    wire.setDefaultTimeout(10000);  // 10ms

    DEBUG_PRINTLN("Scanning I2C bus");

    wire.begin();

    for (byte i = 8; i < 120; i++) {
      wire.beginTransmission(i);
      if (wire.endTransmission() == 0) {
        if (i == addresses::ansible) {
          state.has_ansible = true;
          DEBUG_PRINTLN("Found ansible");
        }

        if (i == addresses::txo) {
          state.has_txo = true;
          DEBUG_PRINTLN("Found TXO");
        }

        if (i == addresses::er301) {
          state.has_er301 = true;
          DEBUG_PRINTLN("Found ER301");
        }
        delay(1);  // maybe unneeded?
      }            // end of good response
    }              // end of for loop
    DEBUG_PRINTLN("I2C scan complete.");
  }

  // non-master mode
  else {
    DEBUG_PRINTLN("Enabling i2c enabled in SLAVE mode");
    wire.begin(I2C_SLAVE, I2C_ADDRESS, I2C_PINS, I2C_PULLUP_EXT, 400000);
    wire.onReceive(i2c::Write);
    wire.onRequest(i2c::ReadRequest);
  }
}

/*
 * Sends an i2c command out to a slave when running in master mode
 */
void Send(uint8_t model, uint8_t deviceIndex, uint8_t cmd, uint8_t devicePort, uint16_t value) {
  messageBuffer[2] = value >> 8;
  messageBuffer[3] = value & 0xff;

  wire.beginTransmission(model + deviceIndex);
  messageBuffer[0] = cmd;
  messageBuffer[1] = (uint8_t)devicePort;
  wire.write(messageBuffer.data(), messageBuffer.size());
  wire.endTransmission();
}

/*
 * The function that responds to a command from i2c.
 * In the first version, this simply sets the port to be read from.
 */
void Write(size_t len) {
  DEBUG_PRINTF("i2c Write (%d)\n", len);

  // parse the response
  TxResponse response = TxHelper::Parse(len);

  // true command our setting of the input for a read?
  if (len == 1) {
    // use a helper to decode the command
    TxIO io = TxHelper::DecodeIO(response.Command);

    DEBUG_PRINTF("Port: %d; Mode: %d [%d]\n", io.Port, io.Mode, response.Command);

    // this is the single byte that sets the active input
    activeInput = io.Port;
    activeMode = io.Mode;
  }
  else {
    // act on the command
    actOnCommand(response.Command, response.Output, response.Value);
  }
}

/*
 * The function that responds to read requests over i2c.
 * This uses the port from the write request to determine which slider to send.
 */
void ReadRequest() {
  DEBUG_PRINT("i2c Read\n");

  // get and cast the value
  auto shiftReady = static_cast<uint16_t>(state.current[activeInput]);

  DEBUG_PRINTF("delivering: %d; value: %d [%d]\n", activeInput, state.current[activeInput], shiftReady);

  // send the puppy as a pair of bytes
  wire.write(shiftReady >> 8);
  wire.write(shiftReady & 255);
}

/*
 * Future function if we add more i2c capabilities beyond reading values.
 */
void actOnCommand(uint8_t cmd, uint8_t out, int value) {
}
}  // namespace i2c
