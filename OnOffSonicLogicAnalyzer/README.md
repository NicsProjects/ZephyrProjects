# OnOffSonicLogicAnalyzer

Companion Zephyr project for measuring the HC-SR04 ECHO pulse from
`OnOffSonic` with a second NUCLEO-H563ZI. It reports the pulse width in
microseconds, which can be compared with a hardware logic-analyzer capture.

## Wiring

- DUT `OnOffSonic` D4 (HC-SR04 ECHO) -> probe board D4
- DUT GND -> probe board GND
- Keep the existing HC-SR04 1k/2k echo divider in place.

The probe board does not drive the signal. Do not connect its D4 to the
sensor or to a 5 V signal without the level divider.

## Build and flash

From the workspace root:

```bash
west build -b nucleo_h563zi -d OnOffSonicLogicAnalyzer/build OnOffSonicLogicAnalyzer
west flash -d OnOffSonicLogicAnalyzer/build
```

For this STM32 board, use the board's normal serial console:

```bash
screen /dev/ttyACM0 115200
```

When `OnOffSonic` measures, the probe prints lines such as:

```text
echo: 1843 us
```

For a hardware logic analyzer, connect its ground to DUT ground and probe
`OnOffSonic` D3 (TRIG) and D4 (ECHO). A valid trigger is at least 10 us high;
OnOffSonic D3 (TRIG) and D4 (ECHO). For an 8-channel, 24 MHz analyzer, use:

```text
DUT NUCLEO-H563ZI       Logic analyzer
--------------------------------------
GND ------------------- GND
D3 / HC-SR04 TRIG ----- CH0
D4 / divided ECHO ----- CH1
```

Connect CH1 at the same point as D4, after the existing 1k/2k voltage
divider. Do not connect the analyzer to the raw 5 V HC-SR04 ECHO signal.
Leave the other analyzer channels disconnected and configure the analyzer
for 3.3 V digital logic. A valid trigger is at least 10 us high; the ECHO
high-time is proportional to distance.