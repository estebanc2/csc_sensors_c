# CSC Sensors C

Proyecto ESP-IDF para un sensor de velocidad y cadencia de ciclismo (Cycling Speed and Cadence, CSC) basado en ESP32.

## Descripción

Este firmware lee pulsos de sensores Hall en dos entradas GPIO y expone la medición a través de BLE usando el servicio estándar CSC.

- `GPIO_CCRANK` = GPIO 8
- `GPIO_WHEEL` = GPIO 9
- BLE usa el servicio `0x1816` (CSC)
- Notificaciones en la característica `0x2A5B`
- Información del dispositivo disponible vía GATT Device Information

## Estructura del proyecto

- `main/main.c`: punto de entrada, loop principal y notificaciones BLE.
- `main/ble.c`, `main/ble.h`: inicialización BLE y envío de datos CSC.
- `main/sensors.c`, `main/sensors.h`: lectura y cálculo de revoluciones / tiempos de evento.
- `main/config.h`: configuración de pines, constantes y datos de dispositivo.

## Requisitos

- ESP-IDF compatible (este repositorio esta configurado para ESP-IDF 5.5)
- ESP32 compatible con BLE
- Sensores Hall o reed switches para rueda y biela

## Conexiones de hardware

Se espera un sensor Hall típico con salida digital:

- VCC → 3.3V
- GND → GND
- OUT → GPIO 8 o GPIO 9
- Pull-up recomendado: 10 kΩ a 3.3V (o usar pull-up interno del ESP32)

### Nota sobre el sensor

El código actual asume detección por flanco negativo (`GPIO_INTR_NEGEDGE`) porque algunos sensores Hall activos en baja reaccionan cuando el imán pasa.

## Compilar e instalar

1. Configurar el entorno ESP-IDF:
   ```bash
   . $IDF_PATH/export.sh
   ```

2. Desde la raíz del proyecto:
   ```bash
   idf.py build
   idf.py flash
   idf.py monitor
   ```

## Personalización

- Cambiar pines en `main/config.h` si usas otros GPIO.
- Ajustar el filtro de rebote y la lógica del sensor en `main/sensors.c` según tu hardware.

## Contacto

Firmware desarrollado por Esteban Castro.

---

`csc_sensors_c` está diseñado para proyectos de entrenamiento y ciclismo con BLE CSC estándar, facilitando la integración con aplicaciones y dispositivos deportivos compatibles.