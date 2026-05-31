#pragma once

#include "config.h"
#include "memory.h"

#define GATT_CSC_SERVICE_UUID 0x1816
#define GATT_CSC_MEASUREMENT_CHAR_UUID 0x2A5B //notify
#define GATT_CSC_FEATURE_CHAR_UUID 0x2A5C     //read   
#define GATT_CSC_SENSOR_LOCATION_CHAR_UUID 0x2A50     //read   
#define GATT_CSC_CONTROL_POINT_CHAR_UUID 0x2A55     //read   
#define CSC_SENSOR_LOCATION_LEFT_CRANK 5

#define GATT_DEVICE_INFO_UUID 0x180A
#define GATT_MANUFACTURER_NAME_UUID 0x2A29   //read
#define GATT_FIRMWARE_UUID 0x2A26            //read
#define GATT_MODEL_NUMBER_UUID 0x2A24        //read

#define GATT_BATTERY_SERVICE_UUID 0x180F
#define GATT_BATTERY_LEVEL_UUID 0x2A19      //read
#define GATT_CLIENT_CONFIG_UUID 0x2902      //notify

// Appearance: Cycling Speed and Cadence Sensor
#define CSCS_APPEARANCE            0x1814

void ble_init(void);
void ble_notify_new_data(uint32_t, uint16_t, uint16_t, uint16_t);
void ble_notify_bat_level(uint8_t);