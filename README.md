# Airnova: A Smart Medical Device for Automated Nebulizer and Ventilation Control

Airnova is an automated, sensor-driven medical system designed to control nebulizer and ventilation devices based on real-time vital signs monitoring. It enhances patient care by adapting to heart rate and blood oxygen levels, providing a responsive solution for respiratory assistance.

## Features

- **Vital Signs Monitoring**: Real-time tracking of heart rate (HR) and blood oxygen saturation (SpO2).
- **Automated Control**:
  - Ventilation activation for critical conditions (HR > 120 or SpO2 < 85%).
  - Nebulizer activation for moderate oxygen levels (SpO2 85-92%).
  - Safety interlocks to prevent conflicts between devices.
- **Display Interface**: LCD screen for real-time data and status display.
- **Wireless Communication**: ESP-NOW for device-to-device data transfer.
- **Power Management**: Designed for reliable operation in medical settings.

## Tech Stack

### Embedded Firmware
- **Microcontroller**: ESP32
- **Framework**: Arduino
- **Communication**: ESP-NOW (Device-to-Device), WiFi (optional for future extensions)
- **Libraries**: LiquidCrystal_I2C, ESP32Servo, esp_now

### Hardware Integration
- **Sensors**: Heart Rate and SpO2 (via connected sensor device)
- **Actuators**: Solid State Relay (SSR) for nebulizer, Servo motor for ventilation
- **Display**: I2C LCD (20x4)

## Hardware

- **Sensors**: Heart Rate and SpO2 sensors (connected via ESP-NOW)
- **Actuators**: Nebulizer control (SSR), Ventilation servo
- **Display**: 20x4 I2C LCD
- **Microcontroller**: ESP32 Dev Board
- **Power**: Standard power supply (adaptable for medical-grade)

## System Architecture

The system consists of two ESP32 devices communicating via ESP-NOW:
- **Sensor Node**: Collects HR and SpO2 data.
- **Controller Node**: Receives data, processes logic, controls actuators, and displays status.

Block Diagram: [Add diagram here if available]

## Project Structure

```
airnova/
├── esp32_main/
│   └── airnova/
│       ├── airnova.ino          # Main controller firmware
│       └── testing.ino          # Testing utilities
├── esp32_c3_mini/
│   └── airnova_c3_mini/
│       └── airnova_c3_mini.ino  # ESP32-C3 Mini variant
└── README.md                    # Project overview
```

## Getting Started

### Firmware (ESP32)

1. Install the Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software).
2. Install the ESP32 board support in Arduino IDE (Tools > Board > Boards Manager > Search for ESP32).
3. Install required libraries: LiquidCrystal_I2C, ESP32Servo.
4. Open the appropriate `.ino` file in Arduino IDE.
5. Select the correct board (ESP32 Dev Module or ESP32C3 Dev Module).
6. Upload the sketch to your board.
7. Ensure the sensor device is configured to send data via ESP-NOW.

## License



## Acknowledgements

Developed as part of the Airnova initiative to improve automated medical assistance using IoT and embedded systems.
