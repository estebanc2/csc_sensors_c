#pragma once
#include <stdint.h>

void    battery_adc_init(void);
int     battery_read_mv(void);      // voltaje real en mV
uint8_t battery_read_percent(void); // 0–100 para BLE