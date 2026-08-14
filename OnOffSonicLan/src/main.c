#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/net/socket.h>
#include <zephyr/posix/arpa/inet.h>
#include <zephyr/posix/sys/socket.h>
#include <zephyr/sys/printk.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

/*
 * Physical wiring (see boards/nucleo_h563zi.overlay for the DT side):
 *   led_green -> onboard LD1, PB0
 *   led_red   -> onboard LD3, PG4
 *   sw0       -> SPDT toggle switch, common pin on D2, throws on 3V3/GND
 *   hc_sr04   -> TRIG on D3, ECHO on D4 (via 1k/2k divider down to 3.3V)
 */

#define UDP_PORT 5005
#define UDP_MSG_MAX 128

static const struct gpio_dt_spec led_green =
	GPIO_DT_SPEC_GET(DT_ALIAS(led_green), gpios);

static const struct gpio_dt_spec led_red =
	GPIO_DT_SPEC_GET(DT_ALIAS(led_red), gpios);

static const struct gpio_dt_spec switch_pin =
	GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

static const struct device *const sensor =
	DEVICE_DT_GET(DT_NODELABEL(hc_sr04));

static int udp_socket_init(void)
{
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		printk("Failed to create UDP socket: %d\n", errno);
		return -1;
	}

	return sock;
}

static int send_distance_udp(int sock, int32_t distance_mm)
{
	char payload[UDP_MSG_MAX];
	struct sockaddr_in remote_addr;
	int ret;

	memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(UDP_PORT);
	inet_pton(AF_INET, CONFIG_NET_CONFIG_PEER_IPV4_ADDR, &remote_addr.sin_addr);

	snprintk(payload, sizeof(payload),
		 "sensor=hc_sr04,state=on,distance_mm=%d\n",
		distance_mm);

	ret = sendto(sock, payload, strlen(payload), 0,
			(struct sockaddr *)&remote_addr, sizeof(remote_addr));
	if (ret < 0) {
		printk("UDP send failed: %d\n", errno);
		return -1;
	}

	printk("Sent: %s", payload);
	return 0;
}

int main(void)
{
	struct sensor_value distance;
	int udp_sock;
	int state;
	int ready = 0;

	if (!gpio_is_ready_dt(&led_green) ||
	    !gpio_is_ready_dt(&led_red) ||
	    !gpio_is_ready_dt(&switch_pin)) {
		printk("ERROR: GPIO setup failed\n");
		return -1;
	}

	if (!device_is_ready(sensor)) {
		printk("ERROR: HC-SR04 driver not ready\n");
		return -1;
	}

	gpio_pin_configure_dt(&led_green, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led_red, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&switch_pin, GPIO_INPUT);

	udp_sock = udp_socket_init();
	if (udp_sock < 0) {
		return -1;
	}

	printk("\n");
	printk("*** OnOffSonicLan ***\n");
	printk("System Initialized Successfully!\n");
	printk("HC-SR04 driver ready.\n");

	while (1) {
		state = gpio_pin_get_dt(&switch_pin);

		if (state > 0) {
			gpio_pin_set_dt(&led_green, 1);
			gpio_pin_set_dt(&led_red, 0);

			if (sensor_sample_fetch(sensor) == 0) {
				sensor_channel_get(sensor, SENSOR_CHAN_DISTANCE, &distance);
				int32_t distance_mm = (distance.val1 * 1000) + (distance.val2 / 1000);

				printk("Distance: %d.%02d meters\n",
				       distance.val1, distance.val2 / 10000);

				if (ready == 0) {
					printk("UDP sender enabled for LAN destination %s:%d\n",
					       CONFIG_NET_CONFIG_PEER_IPV4_ADDR, UDP_PORT);
					ready = 1;
				}

				send_distance_udp(udp_sock, distance_mm);
			} else {
				printk("Sensor measurement timeout or error.\n");
			}

			k_msleep(500);
		} else {
			gpio_pin_set_dt(&led_green, 0);
			gpio_pin_set_dt(&led_red, 1);
			ready = 0;
			k_msleep(100);
		}
	}

	return 0;
}
