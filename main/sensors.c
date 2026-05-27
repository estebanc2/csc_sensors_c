#include "sensors.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_sleep.h"

static const char *TAG = "SENSORS";

static volatile uint32_t wheel_revs = 0;
static volatile uint32_t crank_revs = 0;
static volatile int64_t  last_wheel_us = 0;
static volatile int64_t  last_crank_us = 0;
static portMUX_TYPE sensor_mux = portMUX_INITIALIZER_UNLOCKED;  // ← renombrado

static void IRAM_ATTR wheel_isr(void *arg) {
    int64_t now = esp_timer_get_time();
    if (now - last_wheel_us > 100000) {
        portENTER_CRITICAL_ISR(&sensor_mux);
        wheel_revs++;
        last_wheel_us = now;
        portEXIT_CRITICAL_ISR(&sensor_mux);
        ledSetFromISR(blink1, green); 
    }
}

static void IRAM_ATTR crank_isr(void *arg) {
    int64_t now = esp_timer_get_time();
    if (now - last_crank_us > 200000) {
        portENTER_CRITICAL_ISR(&sensor_mux);
        crank_revs++;
        last_crank_us = now;
        portEXIT_CRITICAL_ISR(&sensor_mux);
        ledSetFromISR(blink1, blue); 
    }
}

void sensors_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_WHEEL) | (1ULL << GPIO_CRANK),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_WHEEL, wheel_isr, NULL);
    gpio_isr_handler_add(GPIO_CRANK, crank_isr, NULL);
    ESP_LOGI(TAG, "sensors init ok — wheel GPIO%d  crank GPIO%d",
             GPIO_WHEEL, GPIO_CRANK);
}

void sensors_get(uint32_t *wheel_revs_out, uint16_t *wheel_time_out,
                 uint32_t *crank_revs_out, uint16_t *crank_time_out) {
    taskENTER_CRITICAL(&sensor_mux);
    uint32_t wr = wheel_revs;
    uint32_t cr = crank_revs;
    int64_t  wt = last_wheel_us;
    int64_t  ct = last_crank_us;
    taskEXIT_CRITICAL(&sensor_mux);

    *wheel_revs_out = wr;
    *wheel_time_out = (uint16_t)((wt / 1000) * 1024 / 1000);
    *crank_revs_out = cr;
    *crank_time_out = (uint16_t)((ct / 1000) * 1024 / 1000);
}