#pragma once
#include "config.h"

void sensors_init(void);
void sensors_get(uint32_t *wheel_revs, uint16_t *wheel_time,
                 uint16_t *crank_revs, uint16_t *crank_time);
void sensors_reset(uint32_t);