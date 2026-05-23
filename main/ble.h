/*
    ANEMOMETER_C - An app to manage force and magnetic sensors to
    get wind speed and direction

    Copyright (C) 2024  Esteban Castro  ecastro@miratucuadra.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include "config.h"
#include "memory.h"

#define GATT_CSC_SERVICE_UUID 0x1816
#define GATT_CSC_MEASUREMENT_CHAR_UUID 0x2A5B //notify
#define GATT_CSC_FEATURE_CHAR_UUID 0x2A5C     //read/write     

#define GATT_DEVICE_INFO_UUID 0x180A
#define GATT_MANUFACTURER_NAME_UUID 0x2A29   //read
#define GATT_FIRMWARE_UUID 0x2A26            //read
#define GATT_MODEL_NUMBER_UUID 0x2A24        //read

// Appearance: Cycling Speed and Cadence Sensor
#define CSCS_APPEARANCE            0x1814

void ble_init(void);
void ble_notify_new_data(uint32_t, uint16_t, uint32_t, uint16_t);