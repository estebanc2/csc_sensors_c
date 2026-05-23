#include "sensors.h"
#include "driver/gpio.h"
#include "esp_timer.h"

static const char *TAG = "SENSORS";

static volatile uint32_t wheel_revs = 0;
static volatile uint32_t crank_revs = 0;
static volatile int64_t  last_wheel_us = 0;
static volatile int64_t  last_crank_us = 0;

static void IRAM_ATTR wheel_isr(void *arg) {
    int64_t now = esp_timer_get_time();
    if (now - last_wheel_us > 100000) {  // debounce 100ms
        wheel_revs++;
        last_wheel_us = now;
    }
}

static void IRAM_ATTR crank_isr(void *arg) {
    int64_t now = esp_timer_get_time();
    if (now - last_crank_us > 200000) {  // debounce 200ms
        crank_revs++;
        last_crank_us = now;
    }
}

void sensors_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_WHEEL) | (1ULL << GPIO_CCRANK),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_WHEEL,  wheel_isr, NULL);
    gpio_isr_handler_add(GPIO_CCRANK, crank_isr, NULL);
    ESP_LOGI(TAG, "sensors init ok — wheel GPIO%d  crank GPIO%d",
             GPIO_WHEEL, GPIO_CCRANK);
}

void sensors_get(uint32_t *wheel_revs_out, uint16_t *wheel_time_out,
                 uint32_t *crank_revs_out, uint16_t *crank_time_out) {
    portDISABLE_INTERRUPTS();
    uint32_t wr = wheel_revs;
    uint32_t cr = crank_revs;
    int64_t  wt = last_wheel_us;
    int64_t  ct = last_crank_us;
    portENABLE_INTERRUPTS();

    // Convertir microsegundos a unidades de 1/1024 s (protocolo CSC)
    *wheel_revs_out = wr;
    *wheel_time_out = (uint16_t)((wt / 1000) * 1024 / 1000);
    *crank_revs_out = cr;
    *crank_time_out = (uint16_t)((ct / 1000) * 1024 / 1000);
}