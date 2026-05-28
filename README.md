# CSC Sensors C

Proyecto ESP-IDF para un sensor de velocidad y cadencia de ciclismo (Cycling Speed and Cadence, CSC) basado en ESP32.

## Descripción

Este firmware lee pulsos de sensores Hall en dos entradas GPIO y expone la medición a través de BLE usando el servicio estándar CSC.

- `GPIO_WHEEL` = GPIO 4
- `GPIO_CRANK` = GPIO 5
- BLE usa el servicio `0x1816` (CSC)
- Notificaciones en la característica `0x2A5B`
- Información del dispositivo disponible vía GATT Device Information

## Estructura del proyecto

- `main/main.c`: punto de entrada, loop principal y notificaciones BLE.
- `main/ble.c`, `main/ble.h`: inicialización BLE y envío de datos CSC.
- `main/sensors.c`, `main/sensors.h`: lectura y cálculo de revoluciones / tiempos de evento.
- `main/config.h`: configuración de pines, constantes y datos de dispositivo.
- `sdkconfig.defaults`: configuración de compilación recomendada (PM, BLE, NimBLE).

## Requisitos

- ESP-IDF 5.5 o compatible
- ESP32-C3 u otro ESP32 compatible con BLE
- Sensores Hall o reed switches para rueda y biela

## Conexiones de hardware

Se espera un sensor Hall típico con salida digital:

- VCC → 3.3V
- GND → GND
- OUT → GPIO 4 o GPIO 5
- Pull-up recomendado: 10 kΩ a 3.3V (o usar pull-up interno del ESP32)

### Nota sobre el sensor

El código actual asume detección por flanco negativo (`GPIO_INTR_NEGEDGE`) porque algunos sensores Hall activos en baja reaccionan cuando el imán pasa.

## Bajo consumo

El firmware implementa **automatic light sleep** de ESP-IDF, lo que permite mantener la conexión BLE activa mientras el chip duerme entre eventos. No se usa deep sleep porque cortaría la conexión BLE.

Características de la estrategia de ahorro:

- FreeRTOS tickless idle: el scheduler suprime ticks cuando no hay tareas listas, el chip entra en light sleep automáticamente durante el `vTaskDelayUntil` del loop principal (1 segundo entre notificaciones).
- GPIO wakeup: cualquier pulso de rueda o biela despierta el chip en ~200 µs para ejecutar la ISR.
- Dynamic frequency scaling (DFS): la CPU escala entre 40 MHz (idle) y 80 MHz (activo) según la carga.
- NimBLE sleep habilitado: la radio BLE también duerme entre connection events.
- Connection parameters: al conectar se solicita al master (Garmin u otro) un connection interval largo (500–1000 ms) para maximizar el tiempo dormido entre eventos BLE.

El consumo promedio esperado con BLE conectado y sensor quieto es de 2–4 mA, frente a ~80 mA en modo activo continuo. El valor real depende del connection interval negociado por el dispositivo central.

### Alimentación con batería de litio

El ESP32-C3 opera entre 3.0 V y 3.6 V. Una celda de litio varía entre 4.2 V (cargada) y 3.0 V (descargada), por lo que **se requiere regulación de voltaje**:

- **LDO** (ej. AMS1117-3.3): simple, pero disipa la diferencia de voltaje como calor (15–30% de pérdida).
- **Buck converter** (ej. TPS62xxx): 85–95% de eficiencia, recomendado para maximizar la autonomía.

## Compilar e instalar

   En VS Code con la extensión ESP-IDF, usar la paleta de comandos (`Cmd+Shift+P`):

- `ESP-IDF: Build your project`

- `ESP-IDF: Flash your project`
  
- `ESP-IDF: Monitor your device`

1. Si modificás `sdkconfig.defaults`, borrá el `sdkconfig` generado y hacé fullclean antes de rebuildar:

```bash
   idf.py fullclean
   idf.py build
```

## Personalización

- Cambiar pines en `main/config.h` si usás otros GPIO.
- Ajustar el filtro de rebote en `main/sensors.c` según tu hardware (actualmente 100 ms para rueda, 200 ms para biela).
- Los parámetros de connection interval BLE se pueden ajustar en `main/ble.c` (`CONN_ITVL_MIN_MS` / `CONN_ITVL_MAX_MS`). Un intervalo más largo ahorra más energía pero puede aumentar la latencia de la primera notificación.
- Las frecuencias mínima y máxima de CPU se configuran en `app_main()` vía `esp_pm_configure()`.

## Contacto

Firmware desarrollado por Esteban Castro — ecastro@miratucuadra.com

---

`csc_sensors_c` está diseñado para proyectos de entrenamiento y ciclismo con BLE CSC estándar, facilitando la integración con aplicaciones y dispositivos deportivos compatibles como Garmin, Wahoo, y aplicaciones móviles de ciclismo.

## Configurar para bateria

Cuando llegue ese momento, el camino correcto para ESP32-C3 con BLE + light sleep es:

CONFIG_BT_CTRL_MODEM_SLEEP=y + CONFIG_BT_CTRL_MODEM_SLEEP_MODE_1=y (ya lo tenés)
CONFIG_FREERTOS_USE_TICKLESS_IDLE=y — pero solo habilitado junto con lo anterior
CONFIG_PM_POWER_DOWN_CPU_IN_LIGHT_SLEEP=n — el CPU no se apaga, solo el modem
Connection interval largo — los 500ms-1000ms que ya tenés en ble.c son perfectos para esto

Con esa combinación el ESP32-C3 puede bajar a 3-5mA promedio con BLE conectado, que con una batería 18650 de 3000mAh te da varios meses de autonomía para un sensor de bici que no está en uso 23hs por día.

## Desglose de lo que se manda en la notify de ble

Ese es tu paquete CSC de 13 bytes. Lo desglosamos:

```
03 08 00 00 00 A9 6F 00 00 00 00 00 00
```

**Byte 0 — Flags: `0x03`**
- Bit 0 = 1 → Wheel data presente ✓
- Bit 1 = 1 → Crank data presente ✓

**Bytes 1-4 — Cumulative Wheel Revolutions: `0x08 0x00 0x00 0x00`**
- Little-endian → 8 revoluciones ✓ coincide con lo que decís

**Bytes 5-6 — Last Wheel Event Time: `0xA9 0x6F`**
- Little-endian → `0x6FA9` = 28585 unidades de 1/1024s
- En segundos: 28585 / 1024 = **27.9 segundos** desde el arranque ✓ tiene sentido

**Bytes 7-10 — Cumulative Crank Revolutions: `0x00 0x00 0x00 0x00`**
- 0 revoluciones ✓

**Bytes 11-12 — Last Crank Event Time: `0x00 0x00`**
- 0, nunca hubo evento de crank ✓

Todo tiene sentido y el formato es correcto. El Garmin va a calcular la velocidad comparando la diferencia de revoluciones y de timestamps entre dos notificaciones consecutivas.