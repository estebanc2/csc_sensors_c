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

#include "memory.h"
#include "ble.h"

static const char *TAG = "MEMORY";

void nvs_read_write(int read_or_write, void *to_from_storage,
                    size_t to_from_storage_len) {
  static nvs_handle_t my_handle;
  esp_err_t err;

  // Open
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "open error\n");
  }
  switch (read_or_write) {
  case READ:
    err = nvs_get_blob(my_handle, "cal_data", to_from_storage,
                       &to_from_storage_len);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "get error\n");
    }
    break;
  case WRITE:
    err = nvs_set_blob(my_handle, "cal_data", to_from_storage,
                       to_from_storage_len);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "set error\n");
    }
    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "commit error\n");
    }
    //memcpy(&cal, to_from_storage, to_from_storage_len);
    break;
  }
  nvs_close(my_handle);
}

void nvs_read_write_init() {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGW(TAG, " partition was truncated and needs to be erased");
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
}