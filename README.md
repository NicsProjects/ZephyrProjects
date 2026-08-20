# Zephyr RTOS Learning Projects

## Overview

This repository is a small collection of Zephyr RTOS learning projects built around a common STM32 development board. Each project lives in its own folder and demonstrates a different Zephyr concept in a compact, self-contained application.

The code in these projects is intentionally focused on learning and experimentation rather than being a full production application. The files show the use of device tree, GPIO, sensor drivers, threads, and simple input/output control on an embedded board.

All six projects in this README are intended as learning exercises for Zephyr RTOS.

## Project summary

| Project | Purpose | Main Zephyr concepts demonstrated |
| --- | --- | --- |
| MyBlinky | Simple multi-LED blinking demo | GPIO output, threads, `K_THREAD_DEFINE()`, device-tree aliases for LEDs |
| On&OffBlinky | LED control based on a switch input | GPIO input/output, polling, `gpio_pin_get_dt()`, `gpio_pin_set_dt()` |
| OnOffSensors | Combined temperature/humidity and distance monitoring | Sensor API, DHT sensor driver, HC-SR04 driver, GPIO status LEDs, serial logging |
| OnOffSonic | Ultrasonic distance measurement with on/off behavior | HC-SR04 sensor driver, switch-controlled system state, GPIO output status indications |
| OnOffTemp | Temperature and humidity monitoring with switch-controlled operation | DHT sensor driver, GPIO input/output, periodic sensor sampling |
| OnOffSonicLogicAnalyzer | Companion pulse-timing probe for HC-SR04 signals | GPIO interrupts, pulse-width measurement, logic-analyzer verification |

## Relationship between the projects

These are sibling Zephyr sample applications in the same workspace, not separate modules of one larger app. Each folder has its own:

- `CMakeLists.txt`
- `prj.conf`
- `west.yml`
- `boards/nucleo_h563zi.overlay`
- `src/main.c`

They share the same board target and similar hardware layout, but each project solves a different learning objective. In practical terms, the repository progresses from basic LED blinking to GPIO input handling and finally to sensor-based monitoring systems.

## Hardware platform used by the projects

The project files consistently target the STM32 NUCLEO-H563ZI board. This is visible in the board overlay files under each project, which include `nucleo_h563zi.overlay` and define GPIO mappings for onboard LEDs and Arduino-compatible header pins.

Key board facts confirmed by the files:

- Board target: `nucleo_h563zi`
- MCU family: STM32 NUCLEO-H563ZI
- Onboard LEDs used by multiple apps: Green LED on `PB0` and Red LED on `PG4`
- Additional Arduino-style header pins are used for buttons and external sensors

## Repository structure

```text
zephyrproject/
├── MyBlinky/
│   ├── CMakeLists.txt
│   ├── prj.conf
│   ├── west.yml
│   ├── boards/
│   │   └── nucleo_h563zi.overlay
│   └── src/
│       └── main.c
├── On&OffBlinky/
│   ├── CMakeLists.txt
│   ├── prj.conf
│   ├── west.yml
│   ├── boards/
│   │   └── nucleo_h563zi.overlay
│   └── src/
│       └── main.c
├── OnOffSensors/
│   ├── CMakeLists.txt
│   ├── prj.conf
│   ├── west.yml
│   ├── boards/
│   │   └── nucleo_h563zi.overlay
│   └── src/
│       └── main.c
├── OnOffSonic/
│   ├── CMakeLists.txt
│   ├── prj.conf
│   ├── west.yml
│   ├── README.md
│   ├── boards/
│   │   └── nucleo_h563zi.overlay
│   └── src/
│       └── main.c
├── OnOffTemp/
│   ├── CMakeLists.txt
│   ├── prj.conf
│   ├── west.yml
│   ├── boards/
│   │   └── nucleo_h563zi.overlay
│   └── src/
│       └── main.c
├── OnOffSonicLogicAnalyzer/
│   ├── CMakeLists.txt
│   ├── prj.conf
│   ├── west.yml
│   ├── README.md
│   ├── boards/
│   │   └── nucleo_h563zi.overlay
│   └── src/
│       └── main.c
├── bootloader/
├── build/
├── modules/
├── tools/
├── zephyr/
├── .west/
├── .gitignore
└── README.md   <- this file
```

> The workspace also contains build output, Zephyr sources, and tool directories. The project applications themselves are the six folders listed above.

---

## Project details

### 1) MyBlinky

#### What it does

This project creates three independent Zephyr threads that each blink a different onboard LED at a different rate. The application does not use a switch or sensor; it focuses on GPIO toggling from multiple threads.

#### Purpose

To demonstrate:

- GPIO output control
- Zephyr thread creation with `K_THREAD_DEFINE()`
- Parallel execution in a simple embedded application

#### Main Zephyr concepts demonstrated

- `gpio_dt_spec` and `GPIO_DT_SPEC_GET()`
- `gpio_pin_configure_dt()`
- `gpio_pin_toggle_dt()`
- `k_msleep()`
- kernel threads

#### Hardware/board

- Target board: `nucleo_h563zi`
- LEDs defined in `boards/nucleo_h563zi.overlay`:
  - Green LED: `&gpiob 0`
  - Yellow LED: `&gpiof 4`
  - Red LED: `&gpiog 4`

#### Peripherals used

- GPIO only
- Three board LEDs

#### Important source files

- `src/main.c` — creates three threads and toggles LED states at different intervals
- `boards/nucleo_h563zi.overlay` — maps the aliases `led0`, `led1`, and `led2` to physical LEDs
- `CMakeLists.txt` — builds the application and includes `src/main.c`
- `prj.conf` — enables GPIO support

#### Important settings in `prj.conf`

```ini
CONFIG_GPIO=y
```

#### Build

From inside the project folder:

```bash
cd MyBlinky
west build -p auto -b nucleo_h563zi .
```

#### Flash

```bash
west flash
```

You can also use:

```bash
west build -t flash
```

#### Expected behavior at runtime

When running on the target board, the green, yellow, and red LEDs blink independently. The source code uses different timing periods:

- Green and red LEDs: 200 ms
- Yellow LED: 6000 ms

The overall effect is a simple asynchronous three-LED blinking pattern.

---

### 2) On&OffBlinky

#### What it does

This project reads the state of a switch and drives three LEDs to the same state as that switch. In other words, if the switch reads high, all three LEDs are turned on; if it reads low, all three LEDs are turned off.

#### Purpose

To demonstrate:

- GPIO input reading
- GPIO output driving
- A simple switch-controlled LED state machine

#### Main Zephyr concepts demonstrated

- `GPIO_INPUT` configuration
- `GPIO_OUTPUT_INACTIVE` configuration
- `gpio_pin_get_dt()`
- `gpio_pin_set_dt()`
- Simple polling loop with `k_msleep(50)`

#### Hardware/board

- Target board: `nucleo_h563zi`
- The overlay defines:
  - LED A: `&gpiob 0`
  - LED B: `&gpiof 4`
  - LED C: `&gpiog 4`
  - Switch: `&arduino_header 8` (D2)

#### Peripherals used

- GPIO input from a toggle switch on D2
- Three GPIO outputs driving LEDs

#### Important source files

- `src/main.c` — reads the switch state and mirrors it to all LEDs
- `boards/nucleo_h563zi.overlay` — creates `led-a`, `led-b`, `led-c`, and `sw0` aliases
- `CMakeLists.txt` — builds the application
- `prj.conf` — enables GPIO support

#### Important settings in `prj.conf`

```ini
CONFIG_GPIO=y
```

#### Build

```bash
cd "On&OffBlinky"
west build -p auto -b nucleo_h563zi .
```

#### Flash

```bash
west flash
```

#### Expected behavior at runtime

The board initializes the switch as an input and the LEDs as outputs. The program then repeatedly reads the switch and drives the three LEDs to the same logic value. The code includes a 50 ms delay while polling, which acts as a simple debounce/settling delay.

---

### 3) OnOffSensors

#### What it does

This project reads both an environmental sensor and an ultrasonic distance sensor, then prints the readings to the console while indicating status with LEDs. It is a combined weather-and-distance monitoring example.

#### Purpose

To demonstrate reading multiple sensors from different device drivers and presenting the data in a single monitoring loop.

#### Main Zephyr concepts demonstrated

- Sensor device access via `DEVICE_DT_GET()`
- `sensor_sample_fetch()`
- `sensor_channel_get()`
- GPIO status LEDs
- Error handling when devices are not ready

#### Hardware/board

- Target board: `nucleo_h563zi`
- The overlay defines:
  - Green LED on `&gpiob 0`
  - Red LED on `&gpiog 4`
  - DHT sensor on `&arduino_header 8` (associated with D2 in comments)
  - HC-SR04 trigger on `&arduino_header 9`
  - HC-SR04 echo on `&arduino_header 10`

#### Peripherals used

- GPIO LEDs
- DHT temperature/humidity sensor
- HC-SR04 ultrasonic distance sensor
- Serial console output via `printk()`

#### Important source files

- `src/main.c` — fetches and logs DHT and HC-SR04 readings; toggles green/red LED status
- `boards/nucleo_h563zi.overlay` — declares `dht_sensor` and `hc_sr04` nodes and LED aliases
- `CMakeLists.txt` — app entry point
- `prj.conf` — enables GPIO, sensor, DHT, HC-SR04 support, and floating-point output formatting

#### Important settings in `prj.conf`

```ini
CONFIG_GPIO=y
CONFIG_SENSOR=y
CONFIG_DHT=y
CONFIG_HC_SR04=y
CONFIG_CBPRINTF_FP_SUPPORT=y
```

#### Build

```bash
cd OnOffSensors
west build -p auto -b nucleo_h563zi .
```

#### Flash

```bash
west flash
```

#### Expected behavior at runtime

The program repeatedly samples the DHT sensor and the HC-SR04 sensor. On successful reads, the green LED is briefly turned on and the code prints values similar to:

```text
Temp: 22.34 C | Hum: 48.12 % | Dist: 0.35 m
```

If one of the sensors is unavailable or fails to sample, the red LED is lit and the code prints an error message before retrying.

> The overlay file is the authoritative source for hardware wiring in this project. It defines the DHT and HC-SR04 connections used by the app.

---

### 4) OnOffSonic

#### What it does

This project measures distance using an HC-SR04 ultrasonic sensor, but only when a switch is in the ON state. When the switch is closed, the system reads the distance and prints it. When the switch is off, the system stays idle and indicates the off-state with the red LED.

#### Purpose

To demonstrate:

- A sensor-driven system controlled by a switch
- GPIO input/output control
- Sensor reading and logging with a simple state machine

#### Main Zephyr concepts demonstrated

- `gpio_pin_get_dt()` to read the switch
- `sensor_sample_fetch()` for the HC-SR04 driver
- `sensor_channel_get()` to retrieve distance data
- LED status indication using GPIO

#### Hardware/board

- Target board: `nucleo_h563zi`
- The overlay defines:
  - Green LED on `PB0`
  - Red LED on `PG4`
  - Switch on Arduino D2
  - HC-SR04 trigger on `&arduino_header 9`
  - HC-SR04 echo on `&arduino_header 10`

#### Peripherals used

- GPIO input: switch on D2
- GPIO outputs: board LEDs
- HC-SR04 ultrasonic distance sensor

#### Important source files

- `src/main.c` — polls the switch, reads the sensor, and prints distance values
- `boards/nucleo_h563zi.overlay` — configures the LEDs, switch, and HC-SR04 sensor pins
- `CMakeLists.txt` — application target definition
- `prj.conf` — enables required drivers

#### Important settings in `prj.conf`

```ini
CONFIG_GPIO=y
CONFIG_SENSOR=y
CONFIG_HC_SR04=y
CONFIG_CBPRINTF_FP_SUPPORT=y
```

#### Build

```bash
cd OnOffSonic
west build -p auto -b nucleo_h563zi .
```

#### Flash

```bash
west flash
```

#### Expected behavior at runtime

- With the input switch active, the green LED turns on and the code reads the distance sensor every 500 ms.
- The app prints values in the format:

```text
Distance: 0.34 meters
```

- With the switch inactive, the system turns on the red LED and pauses in the off-state.

> The project also contains a README file at the project level, but the source code and overlay file are the authoritative source for behavior.

---

### 5) OnOffTemp

#### What it does

This project monitors ambient temperature and humidity using a DHT sensor while a switch controls whether the monitoring loop is active. When the switch is set to ON, the code fetches temperature and humidity values and prints them. When it is off, the system remains in an idle state with the red LED active.

#### Purpose

To demonstrate a simple environmental monitoring system that turns on only when enabled by a switch.

#### Main Zephyr concepts demonstrated

- GPIO input using a switch
- GPIO output using board LEDs
- DHT sensor access with `sensor_sample_fetch()` and `sensor_channel_get()`
- Periodic polling with `k_msleep()`

#### Hardware/board

- Target board: `nucleo_h563zi`
- The overlay defines:
  - Green LED on `PB0`
  - Red LED on `PG4`
  - Switch on Arduino D2
  - DHT sensor connected via Arduino header pin 9

#### Peripherals used

- GPIO input: switch on D2
- GPIO outputs: board LEDs
- DHT sensor (temperature and humidity)

#### Important source files

- `src/main.c` — reads switch state and prints DHT values when enabled
- `boards/nucleo_h563zi.overlay` — defines the LEDs, switch, and DHT node
- `CMakeLists.txt` — build definition
- `prj.conf` — enables GPIO, sensor, DHT support, and floating-point output formatting

#### Important settings in `prj.conf`

```ini
CONFIG_GPIO=y
CONFIG_SENSOR=y
CONFIG_DHT=y
CONFIG_CBPRINTF_FP_SUPPORT=y
```

#### Build

```bash
cd OnOffTemp
west build -p auto -b nucleo_h563zi .
```

#### Flash

```bash
west flash
```

#### Expected behavior at runtime

- When the switch is enabled, the green LED turns on and the code reads the DHT sensor
- Output is printed in a format such as:

```text
Temp: 23.11 C  |  Humidity: 44.22 %
```

- When the switch is off, the red LED is set and the loop pauses briefly before checking the switch again

> The overlay contains a comment indicating the sensor is attached to a Grove signal pin on D3, while the actual `dio-gpios` entry points to `&arduino_header 9`. The source code and overlay are the most reliable indication of the actual wiring used by the project.

---

### 6) OnOffSonicLogicAnalyzer

#### What it does

This companion project runs on a second NUCLEO-H563ZI and measures the
HC-SR04 ECHO pulse from `OnOffSonic`. It reports the pulse width in
microseconds so the result can be compared with a hardware logic-analyzer
capture.

#### Purpose

To demonstrate:

- GPIO edge interrupts
- Pulse-width measurement with Zephyr cycle timestamps
- Hardware verification with an 8-channel, 24 MHz logic analyzer

#### Hardware/board

- Target board: `nucleo_h563zi` used as the probe board
- DUT: the separate `OnOffSonic` NUCLEO-H563ZI board
- The probe board reads Arduino D4 as its ECHO input
- Keep the HC-SR04 1k/2k ECHO voltage divider in place

#### Logic-analyzer wiring

Connect the logic analyzer to the DUT as follows:

```text
DUT NUCLEO-H563ZI       8CH, 24 MHz logic analyzer
--------------------------------------------------
GND ------------------- GND
D3 / HC-SR04 TRIG ----- CH0
D4 / divided ECHO ----- CH1
```

Connect CH1 at the same point as DUT D4, after the existing voltage divider.
Configure the analyzer for 3.3 V digital logic and leave the other channels
disconnected. Do not connect the analyzer to the raw 5 V HC-SR04 ECHO signal.

The probe board wiring is:

```text
OnOffSonic DUT D4 ----- probe board D4
DUT GND --------------- probe board GND
```

#### Peripherals used

- GPIO input with both-edge interrupt detection
- Serial console output via `printk()`
- External 8-channel logic analyzer for TRIG and ECHO capture

#### Important source files

- `src/main.c` — timestamps ECHO rising and falling edges and prints pulse width
- `boards/nucleo_h563zi.overlay` — maps the probe ECHO input to Arduino D4
- `README.md` — documents probe-board and logic-analyzer wiring

#### Build and flash

From the workspace root:

```bash
west build -b nucleo_h563zi -d OnOffSonicLogicAnalyzer/build OnOffSonicLogicAnalyzer
west flash -d OnOffSonicLogicAnalyzer/build
```

For the probe board's serial console:

```bash
screen /dev/ttyACM0 115200
```

#### Expected behavior at runtime

The probe prints pulse widths in a format similar to:

```text
echo: 1843 us
```

The ECHO high-time is proportional to the measured distance. The TRIG pulse
should be at least 10 us high.

---

## Common build and flash workflow

Each project is structured as a standalone Zephyr application with its own `west.yml` manifest. The usual workflow is:

```bash
cd <project-folder>
west build -p auto -b nucleo_h563zi .
west flash
```

If the Zephyr environment has not yet been initialized in the shell, initialize or source the Zephyr environment first according to your local Zephyr setup.

## Notes and limitations

This repository is clearly a learning project set. The project files do not include a single top-level application or shared library; instead, each project is an independent demonstration. The project code and device tree overlays are the authority for board connections and runtime behavior.

Where the files are ambiguous or contradictory, this README states the actual file-based fact rather than guessing. For example, some comments mention pin numbers that differ from the actual `dio-gpios` and trigger/echo mappings in the overlays; the overlay definitions are treated as the precise configuration.

## Conclusion

These six projects demonstrate a natural progression in Zephyr learning:

1. GPIO blinking
2. Switch-driven GPIO control
3. Multi-sensor monitoring
4. Ultrasonic ranging with a control input
5. Temperature/humidity monitoring with a switch-driven lifecycle
6. GPIO pulse measurement and logic-analyzer verification

Together they form a practical introduction to Zephyr RTOS programming on the STM32 NUCLEO-H563ZI board.
