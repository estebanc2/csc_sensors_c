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

#include "esp_app_desc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "stdio.h"
//#include "utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAGIC 0x12345672

#define GPIO_CCRANK  8 
#define GPIO_WHEEL   9 
#define MANUFACTURER "www.miratucuadra.com"
#define MODEL "e-windsock"



typedef struct {
	uint16_t cscs, conn;
} handle_t;

enum nvs_action { READ, WRITE };
extern char version[32];
extern handle_t handle;
extern bool connected;
extern uint16_t conn_id;
extern uint32_t cumulative_wheel_revs;
extern uint32_t cumulative_crank_revs;
extern uint16_t last_wheel_event_time;    // 1/100 segundos
extern uint16_t last_crank_event_time;    // 1/1024 segundos

