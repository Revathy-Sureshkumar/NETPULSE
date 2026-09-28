# NETPULSE – Smart IoT Network Fault Detection and Alert System

## Project Overview

NETPULSE is an ESP32-based IoT system designed to monitor connected Wi-Fi devices and detect network and temperature fault conditions.

The ESP32 operates as a local Wi-Fi access point and monitors connected devices using their MAC addresses. A DHT11 temperature sensor is also used to monitor the surrounding temperature.

## Features

- ESP32-based Wi-Fi access point
- Monitoring of up to 3 registered devices
- Wi-Fi device disconnection detection
- DHT11 temperature monitoring
- Temperature fault detection at 35°C
- 16×2 I2C LCD display
- Green LED for normal operation
- Red LED for fault indication
- Five-second buzzer alert
- Local standalone operation
- No cloud or external server required

## Hardware Components

- ESP32 DevKit V1 (ESP-WROOM-32)
- DHT11 Temperature Sensor
- 16×2 I2C LCD with AiP31068 driver
- Green LED
- Red LED
- Buzzer
- LED resistors
- Connecting wires
- USB power supply

## Pin Configuration

| Component | ESP32 Pin |
|---|---|
| Green LED | GPIO 4 |
| Red LED | GPIO 5 |
| Buzzer | GPIO 18 |
| DHT11 DATA | GPIO 15 |
| LCD SDA | GPIO 21 |
| LCD SCL | GPIO 22 |

## Software Requirements

- Arduino IDE
- ESP32 Board Support Package
- DHT Library
- LiquidCrystal_AIP31068_I2C Library
- WiFi Library
- ESP32 Wi-Fi API

## Working

1. The ESP32 creates the `NETPULSE` local Wi-Fi network.
2. Laptops or Wi-Fi-enabled devices connect to the network.
3. The ESP32 detects connected Wi-Fi stations using their MAC addresses.
4. The first three detected devices are registered for monitoring.
5. The system continuously checks the registered devices.
6. The DHT11 measures the surrounding temperature.
7. A temperature of 35°C or above is treated as a temperature fault.
8. The LCD displays device and temperature information.
9. The green LED indicates normal operation.
10. The red LED indicates a fault condition.
11. The buzzer provides a five-second audible alert when a new fault is detected.

## Fault Conditions

### Network Fault

A network fault is detected when a previously registered Wi-Fi device is no longer present in the ESP32's connected-station list.

### Temperature Fault

A temperature fault is detected when the DHT11 temperature reading reaches or exceeds **35°C**.

## Project Structure

```text
NETPULSE/
├── NETPULSE.ino
└── README.md
```

## How to Run

1. Install Arduino IDE.
2. Install the ESP32 board package in Arduino IDE.
3. Install the required DHT and LCD libraries.
4. Open `NETPULSE.ino`.
5. Select the appropriate ESP32 board and COM/USB port.
6. Upload the program to the ESP32.
7. Open the Serial Monitor at **115200 baud**.
8. Connect laptops or Wi-Fi-enabled devices to the `NETPULSE` Wi-Fi network.
9. Observe device status, temperature, and fault alerts on the LCD and through the LEDs/buzzer.

## Default Network Configuration

| Parameter | Value |
|---|---|
| Wi-Fi SSID | `NETPULSE` |
| Wi-Fi Password | `NetPulse123` |
| Maximum Registered Devices | 3 |
| Temperature Fault Threshold | 35°C |
| Buzzer Alert Duration | 5 seconds |
| LCD Address | `0x3E` |

> **Note:** The Wi-Fi credentials in this repository are intended for the academic prototype. Do not use personal or production network credentials in a public repository.

## Project Purpose

The project demonstrates how an ESP32 can combine local Wi-Fi device monitoring, temperature sensing, embedded processing, LCD visualization, and local fault alerts in a compact and low-cost IoT system.

## Authors

Developed as part of the **Embedded Programming PBL Project**.

## License

This project is developed for academic and educational purposes.
