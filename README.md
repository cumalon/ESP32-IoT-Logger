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
