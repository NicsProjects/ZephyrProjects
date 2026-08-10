# OnOffSonicLan - Ethernet UDP sensor transmitter

This project keeps the original OnOffSonic behavior:

- HC-SR04 ultrasonic distance measurement
- switch-controlled system ON/OFF state
- green/red LED status indication

It adds a LAN-based UDP sender so the measured distance can be sent over Ethernet to a local router, ideally using a physical LAN cable.

## Hardware

- MCU: STM32 NUCLEO-H563ZI
- Sensor: HC-SR04
- Switch: D2
- LEDs: onboard green and red
- Ethernet: NUCLEO-H563ZI RMII Ethernet interface connected to the LAN

## Router setup

Use the Tenda AC1200MU-MIMO Dual Band Gigabit router as the LAN destination.

Typical example:

- Board IP: 192.168.0.50
- Router LAN IP: 192.168.0.1
- UDP destination port: 5005

If your Tenda uses a different LAN subnet, change the values in `prj.conf`:

- `CONFIG_NET_CONFIG_MY_IPV4_ADDR`
- `CONFIG_NET_CONFIG_PEER_IPV4_ADDR`

## Receiving the data on a PC

On a Linux or Windows machine connected to the same LAN, run a simple UDP listener:

```bash
nc -u -l -p 5005
```

or in Python:

```python
import socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", 5005))
while True:
    data, addr = sock.recvfrom(256)
    print(data.decode("utf-8", errors="replace"))
```

## Notes

- The project is configured for a static IP on the board and a UDP packet to the router.
- For real deployment, confirm the router is configured to accept the traffic and that the two devices share the same LAN subnet.
- This is intended for local network sending over Ethernet, not internet publication.
