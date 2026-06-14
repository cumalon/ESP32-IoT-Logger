# ESP32-IoT-Logger

Firmware for an ESP32-based IoT environmental data logger.

## Features

- ESP32 WROOM32 support
- DHT11 temperature and humidity sensor
- 16x2 LCD display over I2C
- WiFi configuration portal
- Persistent configuration using ESP32 Preferences
- ThingSpeak data upload

## Hardware

- AZDelivery ESP32 WROOM32
- DHT11 sensor module
- 16x2 LCD I2C display

## Current pinout

| Component | ESP32 pin |
|---|---|
| DHT11 DATA | GPIO15 |
| LCD SDA | GPIO21 |
| LCD SCL | GPIO22 |

## Configuration mode

If no WiFi configuration is saved, the ESP32 creates an access point:

```text
SSID: ESP32_DHT_Config
Password: 12345678
URL: http://192.168.4.1

### Default configuration

- DHT sampling interval: 300 seconds
- ThingSpeak upload interval: 300 seconds
- Values can be changed from the configuration portal.

## LCD I2C Notes

During development, intermittent ESP32 resets and LCD corruption were observed when using a standard 16x2 LCD with I2C backpack.

The following configuration has proven stable:

- LCD powered from 5V
- DHT11 powered from 3.3V
- SDA -> GPIO21
- SCL -> GPIO22
- DHT DATA -> GPIO15

Additional firmware changes that improved stability:

- LCD backlight is enabled only when displaying information.
- The display remains active for a few seconds and then turns off.
- The LCD is no longer refreshed continuously.
- ThingSpeak transmission is not performed during setup.
- Temperature and humidity are refreshed at configurable intervals.

### Important

If LCD corruption, random characters or ESP32 resets reappear:

1. Check LCD power wiring.
2. Verify SDA and SCL connections.
3. Consider using an I2C level shifter if the LCD backpack uses pull-ups to 5V.
4. Test with LCD disconnected to isolate hardware issues.
