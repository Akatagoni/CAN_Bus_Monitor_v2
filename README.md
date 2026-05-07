# STM32F446RE Multi-Protocol Embedded Communication System

> Real-time sensor monitoring, CAN Bus communication, and SD card logging using FreeRTOS on STM32F446RE

## 🔧 Hardware
| Component | Details |
|-----------|---------|
| MCU | STM32F446RE Nucleo @ 180MHz (HSE 8MHz → PLL) |
| Sensor | BME280 — Temperature, Pressure, Humidity via I2C |
| Storage | SDHC SD Card with FatFs over SPI3 |
| Protocols | UART, I2C, SPI, CAN Bus |
| Power | 10µF capacitor on SD module for brownout protection |

## ⚙️ FreeRTOS Task Architecture
| Task | Priority | Stack | Function |
|------|----------|-------|----------|
| SensorTask | High | 4096B | Reads BME280 via I2C every 1s |
| CANTxTask | High+1 | 2048B | Transmits sensor data via CAN at 500kbps |
| CANRxTask | High+2 | 1024B | Receives and decodes CAN frames |
| LogTask | Normal | 8192B | Logs sensor data to SD card every 5s |
| ErrorTask | BelowNormal | 1024B | Error monitoring |
| UARTTask | Low | 1024B | Debug output |
| DefaultTask | Normal | 512B | System idle |

## 📡 Communication Protocols
| Protocol | Purpose | Pins | Speed |
|----------|---------|------|-------|
| UART2 | Debug output | PA2 TX, PA3 RX | 115200 baud |
| I2C1 | BME280 sensor | PB8 SCL, PB9 SDA | 100kHz |
| SPI3 | SD card logging | PC10 SCK, PC11 MISO, PC12 MOSI | APB1/8 |
| CAN2 | Sensor transmission | PB12 RX, PB13 TX | 500kbps |

## 📦 CAN Bus Configuration
- Mode: Loopback
- Prescaler: 6, BS1: 11TQ, BS2: 2TQ
- Filter Bank: 14, SlaveStartFilterBank: 14

| Message ID | Payload |
|------------|---------|
| 0x100 | Temperature (IEEE754 float, 4 bytes) |
| 0x101 | Pressure (IEEE754 float, 4 bytes) |
| 0x102 | Humidity (IEEE754 float, 4 bytes) |

## ⏰ Clock Configuration
- HSE: 8MHz external crystal
- PLL: PLLM=4, PLLN=180, PLLP=2
- SYSCLK: 180MHz
- APB1: 45MHz, APB2: 90MHz
- HAL Timebase: TIM1 (not SysTick — reserved for FreeRTOS)

## 🗂️ FreeRTOS Configuration
- Total Heap: 40960 bytes
- Inter-task communication via osMessageQueue
- Sensor data shared via global volatile floats

## 📁 SD Card Log Sample

Temperature:23.19C, Pressure:976.27hPa, Hum:29.23%
Temperature:23.18C, Pressure:976.28hPa, Hum:29.19%
Temperature:23.19C, Pressure:976.29hPa, Hum:29.13%
Temperature:23.18C, Pressure:976.26hPa, Hum:29.09%

## 💻 UART Output Sample

Received_Temp: 23.21 C
Received_Pres: 976.71 hPa
Received_Hum: 29.29 %
CAN TX OK!
T:23.22C P:976.75hPa H:29.29%
Received_Temp: 23.22 C
Received_Pres: 976.75 hPa
Received_Hum: 29.29 %
T:23.21C P:976.74hPa H:29.15%
CAN TX OK!
Received_Temp: 23.21 C
Received_Pres: 976.74 hPa
Received_Hum: 29.15 %
SD Write OK!
T:23.21C P:976.68hPa H:29.27%
CAN TX OK!
Received_Temp: 23.21 C
Received_Pres: 976.68 hPa
Received_Hum: 29.27 %

## 🐛 Key Debugging Challenges Solved
- **SPI1 conflict with LD2** — SPI1 SCK on PA5 conflicted with onboard LED, resolved by switching to SPI3 (PC10/PC11/PC12)
- **SD card brownout reset** — SD module caused periodic resets, resolved with 10µF capacitor on VCC and osDelay instead of HAL_Delay
- **SDHC sector addressing** — removed byte multiplication (`sector *= 512`) in USER_read/USER_write for SDHC compatibility
- **FreeRTOS stack overflow** — increased LogTask stack from 512 to 2048 words and total heap to 40960 bytes
- **TIM1 priority conflict** — HAL timebase TIM1 priority set to 6 to avoid FreeRTOS scheduler conflict
- **CAN2 RX pullup** — CAN2 RX pin PB12 required internal pullup to maintain bus idle state in loopback mode
- **FreeRTOS heap tuning** — increased configTOTAL_HEAP_SIZE to 40960 bytes to support 7 concurrent tasks
- **BME280 soldering** — sensor required careful soldering of I2C pins for reliable communication at address 0x76
- **BME280 register order** — humidity register 0xF2 must be written before mode register 0xF4
- **FR_NO_FILESYSTEM error** — SD card required FAT32 format with 32KB allocation unit for FatFs compatibility

## 🛠️ Tools & Libraries
- STM32CubeIDE 2.1.1
- STM32CubeMX — peripheral and clock configuration
- STM32 HAL Library — hardware abstraction layer
- FreeRTOS CMSIS-RTOS v2 — real-time operating system
- FatFs Middleware — FAT32 filesystem for SD card
- PuTTY — UART serial monitor for debugging

## ✅ Skills Demonstrated
- Embedded C programming on ARM Cortex-M4 using STM32 HAL
- FreeRTOS multitasking — 7 concurrent tasks with priority scheduling
- Inter-task communication via osMessageQueue
- Multi-protocol communication — UART, I2C, SPI3, CAN Bus
- SDHC card initialization with custom SPI driver and FatFs integration
- CAN Bus frame transmission and reception with message filtering
- BME280 sensor driver implementation — calibration, compensation formulas
- Clock tree configuration — HSE 8MHz to 180MHz via PLL
- Hardware debugging — brownout analysis, SPI signal tracing, pin conflict resolution
- Power stability — decoupling capacitor selection for SD module
- FreeRTOS memory management — heap and stack size tuning for 7 tasks
