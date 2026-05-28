#include "simulator.h"
#include "esp_timer.h"

static const char *TAG = "SENSORS";

static uint32_t wheel_revs = 0;
static uint16_t crank_revs = 0;
static int64_t  last_wheel_us = 0;
static int64_t  last_crank_us = 0;

static void simulator_loop(void *arg) {
    static int wheel_tick = 0;
    static int crank_tick = 0;
    while (1) {
        wheel_tick++;
        crank_tick++;
        if (wheel_tick == 4){
            wheel_tick = 0;
            last_wheel_us = esp_timer_get_time();
            wheel_revs++;
        }
        if (crank_tick == 7){
            crank_tick = 0;
            last_crank_us = esp_timer_get_time();
            crank_revs++;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void simulator_get(uint32_t *wheel_revs_out, uint16_t *wheel_time_out,
                 uint16_t *crank_revs_out, uint16_t *crank_time_out) {
    uint32_t wr = wheel_revs;
    uint16_t cr = crank_revs;
    int64_t  wt = last_wheel_us;
    int64_t  ct = last_crank_us;
    *wheel_revs_out = wr;
    *wheel_time_out = (uint16_t)((wt * 1024) / 1000000);
    *crank_revs_out = cr;
    *crank_time_out = (uint16_t)((ct * 1024) / 1000000);
}

void simulator_init(void) {
    xTaskCreate(simulator_loop, "simulator_loop", 4096, NULL, 10, NULL);
    ESP_LOGI(TAG,"Simulator iniciado a 2,5 vueltas de rueda/seg y a 1,42 vueltas de plato/seg");
}