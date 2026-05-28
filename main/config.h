#pragma once

#include "esp_app_desc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "stdio.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "led.h"

#define RGB_LED_GPIO   10
#define GPIO_WHEEL  4 // cable blanco
#define GPIO_CRANK  5 // cable verde
#define MANUFACTURER "www.miratucuadra.com"
#define MODEL "e-windsock"

typedef struct {
	uint16_t cscs, cp, conn;
} handle_t;

enum nvs_action { READ, WRITE };
extern char version[32];
extern handle_t handle;
extern bool connected;
extern uint16_t conn_id;
extern uint32_t cumulative_wheel_revs;
extern uint16_t cumulative_crank_revs;
extern uint16_t last_wheel_event_time;    // 1/1024 segundos
extern uint16_t last_crank_event_time;    // 1/1024 segundos

