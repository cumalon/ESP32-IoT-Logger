# ESP32-IoT-Logger

Firmware for an ESP32-based temperature and humidity logger using a DHT11 sensor, a 16x2 I2C LCD and ThingSpeak.

The device reads temperature and humidity, displays the latest values on the LCD and periodically uploads them to ThingSpeak.

## Hardware

Main components:

- ESP32 WROOM-32
- DHT11 temperature and humidity sensor
- 16x2 LCD with I2C backpack
- WiFi connection
- ThingSpeak channel

## Wiring

Current tested wiring:

| Component | ESP32 |
|---|---|
| DHT11 VCC | 3V3 |
| DHT11 DATA | GPIO15 |
| DHT11 GND | GND |
| LCD VCC | 5V |
| LCD GND | GND |
| LCD SDA | GPIO21 |
| LCD SCL | GPIO22 |

## Firmware path

The Arduino sketch is located at:

```text
firmware/ESP32-IoT-Logger/ESP32-IoT-Logger.ino
