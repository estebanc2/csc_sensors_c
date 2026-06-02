# CSC Sensors C

Firmware ESP-IDF para un sensor de velocidad y cadencia de ciclismo (CSC) sobre ESP32.

## Qué hace

- Lee pulsos de dos sensores Hall:
  - `GPIO_WHEEL` = GPIO 3
  - `GPIO_CRANK` = GPIO 4
- Publica un servicio BLE CSC estándar (`0x1816`)
- Envía notificaciones CSC en la característica `0x2A5B`
- Expone un servicio BLE Battery Service (`0x180F`) con notificaciones periódicas de nivel de batería
- Incluye un servicio GATT Device Information con fabricante, modelo y versión
- Soporta control point para resetear valores acumulados de cadencia/velocidad

## Características actuales

- Servicio CSC con datos de rueda y biela
- Battery Service con notificación cada 60 segundos mientras hay conexión
- Control Point BLE para resetear el contador acumulado
- Ajuste de parámetros de bajo consumo BLE:
  - Intervalo de conexión 500–1000 ms
  - Latencia de conexión configurable
  - NimBLE y el controlador BLE pueden entrar en sleep coordinado
- Simulador de datos disponible en `main/simulator.c`

## Estructura del proyecto

- `main/main.c`: arranque, tareas principales y notificaciones periódicas
- `main/ble.c`, `main/ble.h`: inicialización NimBLE, GATT, notificaciones y eventos BLE
- `main/sensors.c`, `main/sensors.h`: lectura de pulsos y cálculo de revoluciones/tiempo de evento
- `main/battery.c`, `main/battery.h`: lectura de ADC y cálculo de nivel de batería
- `main/config.h`: pines, datos de dispositivo y constantes generales
- `main/led.c`, `main/led.h`: control de LED de estado
- `main/simulator.c`, `main/simulator.h`: simulador de rueda/biela para pruebas
- `sdkconfig.defaults`: configuración recomendada de ESP-IDF

## Requisitos

- ESP-IDF 5.5 o compatible
- ESP32-C3 o ESP32-S3 (el código define LED RGB según target)
- Sensores Hall o reed switches para rueda y biela
- Fuente de alimentación regulada 3.3 V para el ESP32

## Conexiones de hardware

Sensor Hall típico:

- VCC → 3.3V
- GND → GND
- OUT rueda → GPIO 3
- OUT biela → GPIO 4
- Pull-up recomendado: 10 kΩ a 3.3V o usar pull-ups internos

Batería (opcional):

- Se mide con ADC en `main/battery.c` usando un divisor de tensión
- El valor por defecto está calibrado para un divisor con R1 = 150 kΩ y R2 = 68 kΩ
- Ajusta `BAT_R1_KOHM`, `BAT_R2_KOHM`, `BAT_VMAX_MV` y `BAT_VMIN_MV` según tu pack de batería

### Referencias de pines

- Las imágenes de pines para placas ESP32-C3 y ESP32-S3 están disponibles en `assets/esp32c3Z.png` y `assets/esp32s3Z.png`
- Úsalas para verificar las conexiones físicas en las plaquetas antes de soldar o cablear

## BLE

- Nombre de dispositivo: `MTC_CSCS`
- Servicios anunciados:
  - `0x1816` CSC
  - `0x180F` Battery Service
- Características soportadas:
  - CSC Measurement (notify)
  - CSC Feature (read)
  - Sensor Location (read)
  - CSC Control Point (write/indicate)
  - Battery Level (read/notify)
  - Device Information (manufacturer/model/version)

## Uso

Este proyecto está pensado para usarlo desde el entorno ESP-IDF de VS Code.

- `ESP-IDF: Build your project`
- `ESP-IDF: Flash your project`
- `ESP-IDF: Monitor your device`

### Limpieza completa

Si cambias `sdkconfig.defaults` u opciones de configuración, podés usar la extensión de VS Code o el CLI si lo necesitás.

## Configuración y personalización

- Cambia los pines en `main/config.h` si necesitas otros GPIO
- Ajusta filtros de rebote en `main/sensors.c` según tu hardware
- Ajusta conexión BLE en `main/ble.c` con `CONN_ITVL_MIN_MS`, `CONN_ITVL_MAX_MS`, `CONN_LATENCY` y `CONN_TIMEOUT_MS`
- Ajusta el divisor de batería en `main/battery.c`
- Para usar el simulador en vez de los sensores reales, reemplaza `sensors_get(...)` por `simulator_get(...)` en `main/main.c`

## Notas importantes

- El firmware no configura explícitamente un modo de light sleep de ESP en el código; el ahorro de energía se basa en NimBLE y en el scheduler de FreeRTOS.
- La SDK config del proyecto habilita `CONFIG_PM_ENABLE`, `CONFIG_FREERTOS_USE_TICKLESS_IDLE`, `CONFIG_BT_CTRL_MODEM_SLEEP` y `CONFIG_BT_NIMBLE_SLEEP_ENABLE`, pero no hay un `esp_pm_configure()` específico del firmware.
- No se usa deep sleep para mantener la conexión BLE activa.
- El ESP32-C3 debe alimentarse desde 3.3 V; si usas pack de baterías, regula el voltaje apropiadamente
- El ahorro de consumo BLE puede depender de la estabilidad de NimBLE y del controlador, por lo que conviene probar la configuración en tu plataforma antes de darla por definitiva.

## Contacto

Firmware desarrollado por Esteban Castro — ecastro@miratucuadra.com
