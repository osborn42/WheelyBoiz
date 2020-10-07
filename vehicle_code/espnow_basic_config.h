#ifndef ESPNOW_BASIC_CONFIG_H
#define ESPNOW_BASIC_CONFIG_H

#include <inttypes.h>
#include <stdbool.h>

// Define the structure of your data
typedef struct __attribute__((packed)) {
    uint32_t random_value;
    bool button_pushed;
    bool switch_flicked;
    uint32_t xcar;
    uint32_t ycar;
} my_data_t;

// Destination MAC address
// The default address is the broadcast address, which will work out of the box, but the slave will assume every tx succeeds.
// Setting to the master's address will allow the slave to determine if sending succeeded or failed.
//   note: with default config, the master's WiFi driver will log this for you. eg. I (721) wifi:mode : sta (12:34:56:78:9a:bc)
#define MY_RECEIVER_MAC {0x4c, 0x11, 0xae, 0x70, 0xff, 0x7c}

#define MY_ESPNOW_PMK "pmk1234567890123"
#define MY_ESPNOW_CHANNEL 1

// #define MY_ESPNOW_ENABLE_LONG_RANGE 1

#define MY_SLAVE_DEEP_SLEEP_TIME_MS 1

#endif // ESPNOW_BASIC_CONFIG_H
