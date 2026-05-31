// battery_adc.c

#include "battery.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

#define TAG "BATTERY"

// --- Configuración del divisor ---
// Ajustá estos valores según TUS resistencias reales
#define BAT_R1_KOHM         150.0f   // R1 en kΩ (hacia batería)
#define BAT_R2_KOHM          68.0f   // R2 en kΩ (hacia GND)
#define BAT_DIVISOR_RATIO   ((BAT_R1_KOHM + BAT_R2_KOHM) / BAT_R2_KOHM)  // = 3.647

// --- Límites de la batería (en mV) ---
// Ajustá según tu química de batería
#define BAT_VMAX_MV         7500.0f  // 2S LiPo full
#define BAT_VMIN_MV         5500.0f  // 2S LiPo vacía (corte)

// --- ADC ---
#define BAT_ADC_CHANNEL     ADC_CHANNEL_0   // GPIO1 en ESP32-C3
#define BAT_ADC_UNIT        ADC_UNIT_1
#define BAT_ADC_ATTEN       ADC_ATTEN_DB_12 // Rango ~0–3.1V

static adc_oneshot_unit_handle_t adc_handle;
static adc_cali_handle_t cali_handle;
static bool cali_enable = false;

void battery_adc_init(void) {
    // Inicializar ADC oneshot
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = BAT_ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

    // Configurar canal
    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten    = BAT_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT, // 12 bits en ESP32-C3
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, BAT_ADC_CHANNEL, &chan_cfg));

    // Calibración (curve fitting si está disponible, sino line fitting)
    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id  = BAT_ADC_UNIT,
        .chan     = BAT_ADC_CHANNEL,
        .atten    = BAT_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    esp_err_t ret = adc_cali_create_scheme_curve_fitting(&cali_cfg, &cali_handle);
    if (ret == ESP_OK) {
        cali_enable = true;
        ESP_LOGI(TAG, "Calibración curve fitting OK");
    } else {
        ESP_LOGW(TAG, "Sin calibración ADC, usando raw");
    }
}

// Retorna voltaje de batería en mV (ya corregido por divisor)
int battery_read_mv(void) {
    int raw = 0;
    // Promedio de 8 muestras para reducir ruido
    for (int i = 0; i < 8; i++) {
        int sample;
        adc_oneshot_read(adc_handle, BAT_ADC_CHANNEL, &sample);
        raw += sample;
    }
    raw /= 8;

    int adc_mv = 0;
    if (cali_enable) {
        adc_cali_raw_to_voltage(cali_handle, raw, &adc_mv);
    } else {
        // Sin calibración: aproximación lineal cruda
        // 4095 counts → 3100 mV (con ADC_ATTEN_DB_11)
        adc_mv = (raw * 3100) / 4095;
    }

    // Corregir divisor para obtener Vbat real
    int vbat_mv = (int)(adc_mv * BAT_DIVISOR_RATIO);
    ESP_LOGD(TAG, "ADC raw=%d  Vadc=%dmV  Vbat=%dmV", raw, adc_mv, vbat_mv);
    return vbat_mv;
}

// Retorna porcentaje 0–100 para el BLE Battery Service
uint8_t battery_read_percent(void) {
    int vbat_mv = battery_read_mv();

    if (vbat_mv >= (int)BAT_VMAX_MV) return 100;
    if (vbat_mv <= (int)BAT_VMIN_MV) return 0;

    uint8_t percent = (uint8_t)(
        (float)(vbat_mv - BAT_VMIN_MV) /
        (float)(BAT_VMAX_MV - BAT_VMIN_MV) * 100.0f
    );
    ESP_LOGI(TAG, "Batería: %d mV → %d%%", vbat_mv, percent);
    return percent;
}