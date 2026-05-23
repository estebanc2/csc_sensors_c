#pragma once
#include "config.h"

void sensors_init(void);
void sensors_get(uint32_t *wheel_revs, uint16_t *wheel_time,
                 uint32_t *crank_revs, uint16_t *crank_time);

/*
Sobre el hardware, un sensor Hall típico como el SS49E o el más común A3144 se conecta así:

VCC → 3.3V
GND → GND
OUT → GPIO + resistor pull-up de 10kΩ a 3.3V (aunque ya activás el interno por software)

El GPIO_INTR_NEGEDGE es porque el A3144 es activo bajo — cuando detecta el imán la salida cae a 0. Si usás uno activo alto cambiás a GPIO_INTR_POSEDGE.
El debounce en la ISR (100ms rueda, 200ms biela) evita rebotes mecánicos. Ajustá según la velocidad máxima esperada — a 200 RPM de cadencia el período es 300ms, así que 200ms de debounce es el límite.
*/