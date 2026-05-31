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
#include "battery.h"

#define GPIO_WHEEL  3 // cable blanco
#define GPIO_CRANK  4 // cable verde
#if defined(CONFIG_IDF_TARGET_ESP32S3)
	#define RGB_LED_GPIO   21
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
	#define RGB_LED_GPIO   10
#else
	#error "Placa no soportada"
#endif	
#define MANUFACTURER "www.miratucuadra.com"
#define MODEL "MTC_CSC_01"

typedef struct {
	uint16_t cscs, cp, conn, cc;
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

