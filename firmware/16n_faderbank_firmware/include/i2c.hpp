#pragma once
#include <cstddef>
#include <cstdint>


namespace i2c {

namespace addresses {
constexpr int ansible = 0x20;
constexpr int er301 = 0x31;
constexpr int txo = 0x60;
}  // namespace addresses

void Setup();

/*
 * Sends an i2c command out to a slave when running in master mode
 */
void Send(uint8_t model, uint8_t deviceIndex, uint8_t cmd, uint8_t devicePort, uint16_t value);

/*
 * The function that responds to a command from i2c.
 * In the first version, this simply sets the port to be read from.
 */
void Write(size_t len);

/*
 * The function that responds to read requests over i2c.
 * This uses the port from the write request to determine which slider to send.
 */
void ReadRequest();

/*
 * Future function if we add more i2c capabilities beyond reading values.
 */
void actOnCommand(uint8_t cmd, uint8_t out, int value);
}  // namespace i2c
