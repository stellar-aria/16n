/*
 * 16n Faderbank Firmware Configuration
 * (c) 2017,2018,2020 by Brian Crabtree, Sean Hellfritsch, Tom Armitage, and Brendon Cassidy
 * MIT License
 */
#pragma once
#include <cstdint>

/*
 * firmware metadata
 */

constexpr int MAJOR_VERSION = 0x02;
constexpr int MINOR_VERSION = 0x01;
constexpr int POINT_VERSION = 0x01;

/*
 * device metadata
 */

#if defined(__MKL26Z64__) || defined(__MK20DX128__) || defined(_LC_DEBUG)
constexpr int DEVICE_ID = 0x03;  // 16nLC, do not change, needed by editor
#else
constexpr int DEVICE_ID = 0x02;  // 16n, do not change, needed by editor
#endif

// restricts output to only channel 1 for development purposes
// #define DEV 1

// activates printing of debug messages
// #define DEBUG 1

// enables legacy compatibility with non-multiplexer boards
// #define V125

// define startup delay in milliseconds for i2c Leader devices
// this gives follower devices time to boot up.
constexpr int BOOTDELAY = 10000;

// I2C Address for Faderbank. 0x34 unless you ABSOLUTELY know what you are doing.
constexpr uint8_t I2C_ADDRESS = 0x34;

constexpr int kNumChannels = 16;

// wrap code to be executed only under DEBUG conditions in D()
#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG 0
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(...)
#endif


#ifndef V125
#define V125 0
#endif
