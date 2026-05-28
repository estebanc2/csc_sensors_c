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
#include "sdkconfig.h"
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "esp_timer.h"
#include "sensors.h"
#include "simulator.h"
#include "esp_pm.h"

                           
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
    static int j = 0;
    while (1) {
        if (connected) {
            uint32_t wr;
            uint16_t cr, wt, ct;
            //sensors_get(&wr, &wt, &cr, &ct);
            simulator_get(&wr, &wt, &cr, &ct);
            ble_notify_new_data(wr, wt, cr, ct);
            //ESP_LOGI(TAG, "wheel: %lu  crank: %lu", wr, cr);
            ledSet(off, red);
        } else {
            ledSet(off, red);
            j++;
            if (j ==5) {
                j = 0;
                ledSet(blink2,red);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void) {
  printf("%s", bykeTrainerLogo);
  const esp_app_desc_t *appDesc = esp_app_get_description();
  strcpy(version, appDesc->version);
  ledInit();
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGW(TAG, " partition was truncated and needs to be erased");
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  uint8_t efuse_mac[7] = {0};
  esp_efuse_mac_get_default(&efuse_mac[0]);
  printf("version %s, built on %s at %s using ESP-IDF %s\n", 
  appDesc -> version, __DATE__, __TIME__, appDesc -> idf_ver);
  ble_init();
  //sensors_init();
  simulator_init();
  xTaskCreate(main_loop, "main_loop", 4096, NULL, 10, NULL);
}
