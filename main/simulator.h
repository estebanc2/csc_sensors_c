#pragma once
#include "config.h"

void simulator_init(void);
void simulator_get(uint32_t *wheel_revs, uint16_t *wheel_time,
                 uint16_t *crank_revs, uint16_t *crank_time);
