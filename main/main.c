#/*
    Copyright (C) 2026  Esteban Castro  ecastro@miratucuadra.com

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
 
#include "ble.h"
#include "config.h"
#include "esp_app_desc.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "memory.h"
#include "sdkconfig.h"
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "esp_timer.h"
#include "sensors.h"
                           
static const char *bykeTrainerLogo =  " _           _             _             _                 \n" 
                                       "| |         | |           | |           (_)                \n"
                                       "| |__  _   _| | _____     | |_ _ __ __ _ _ _ __   ___ _ __ \n"
                                       "| '_ \\| | | | |/ / _ \\    | __| '__/ _` | | '_ \\ / _ \\ '__|\n"
                                       "| |_) | |_| |   <  __/    | |_| | | (_| | | | | |  __/ |   \n"
                                       "|_.__/ \\__, |_|\\_\\___|     \\__|_|  \\__,_|_|_| |_|\\___|_|   \n"
                                       "        __/ |                                              \n"
                                       "       |___/                                               \n";                   
//graph done in: https://www.asciiart.eu/text-to-ascii-art, with ANSI Shadow
const char *TAG = "TRAINER";
char version[32] = {0};
bool connected = false;
handle_t handle = {0};

static void main_loop(void *arg) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000);
    while (1) {
        if (connected) {
            uint32_t wr, cr;
            uint16_t wt, ct;
            sensors_get(&wr, &wt, &cr, &ct);
            ble_notify_new_data(wr, wt, cr, ct);
            ESP_LOGI(TAG, "wheel: %lu  crank: %lu", wr, cr);
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
/*
uint32_t cumulative_wheel_revs = 0;
uint32_t cumulative_crank_revs = 0;
uint16_t last_wheel_event_time = 0;    // 1/100 segundos
uint16_t last_crank_event_time = 0;    // 1/1024 segundos
static void main_loop(void *arg) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  TickType_t xFrequency = pdMS_TO_TICKS(1000); // 1 seg
  while (1) {
    int64_t start_us = esp_timer_get_time();
    if (connected) {
            // Simular revoluciones (REEMPLAZAR CON DATOS REALES DEL SENSOR)
            cumulative_wheel_revs++;
        last_wheel_event_time = 100;  // 1 segundo (100 × 0.01s)
        
        cumulative_crank_revs++;
        last_crank_event_time = 1024; // 1 segundo (1024 × 1/1024s)
        
        // Enviar medición (~1 Hz)
        ble_notify_new_data(cumulative_wheel_revs, last_wheel_event_time,
                              cumulative_crank_revs, last_crank_event_time);
    }
    //vTaskDelay(SAMPLE_PERIOD / portTICK_PERIOD_MS);
    int64_t elapsed_us = esp_timer_get_time() - start_us;
    int32_t elapsed_ms = (int32_t)(elapsed_us / 1000);
    int32_t period_ms = (int32_t)pdTICKS_TO_MS(xFrequency);
    if (elapsed_ms > period_ms) {
      ESP_LOGW(TAG, "Loop took longer than period! (%ld ms > %ld ms)", 
          (long)elapsed_ms, 
          (long)period_ms);
    }
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}
*/
void app_main(void) {
  printf("%s", bykeTrainerLogo);
  const esp_app_desc_t *appDesc = esp_app_get_description();
  strcpy(version, appDesc->version);
  nvs_read_write_init();
  uint8_t efuse_mac[7] = {0};
  esp_efuse_mac_get_default(&efuse_mac[0]);
  printf("version %s, built on %s at %s using ESP-IDF %s\n", 
  appDesc -> version, __DATE__, __TIME__, appDesc -> idf_ver);
  ble_init();
  sensors_init();
  xTaskCreate(main_loop, "main_loop", 4096, NULL, 10, NULL);
}
