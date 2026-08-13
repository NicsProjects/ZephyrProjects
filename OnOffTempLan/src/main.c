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

#define UDP_PORT 5006
#define UDP_MSG_MAX 128

static const struct gpio_dt_spec led_green  = GPIO_DT_SPEC_GET(DT_ALIAS(led_green), gpios);
static const struct gpio_dt_spec led_red    = GPIO_DT_SPEC_GET(DT_ALIAS(led_red), gpios);
static const struct gpio_dt_spec switch_pin = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct device *const dht_dev   = DEVICE_DT_GET(DT_NODELABEL(dht_sensor));

static int udp_socket_init(void)
{
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		printk("Failed to create UDP socket: %d\n", errno);
		return -1;
	}

	return sock;
}

static int send_sensor_data_udp(int sock, int32_t temp_c, int32_t humidity_pct)
{
	char payload[UDP_MSG_MAX];
	struct sockaddr_in remote_addr;
	int ret;

	memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(UDP_PORT);
	inet_pton(AF_INET, CONFIG_NET_CONFIG_PEER_IPV4_ADDR, &remote_addr.sin_addr);

	snprintk(payload, sizeof(payload),
		 "sensor=dht22,state=on,temp_c=%d,humidity_pct=%d\n",
		temp_c, humidity_pct);

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
	struct sensor_value temp, humidity;
	int udp_sock;
	int state;
	int ready = 0;

	if (!gpio_is_ready_dt(&led_green) || !gpio_is_ready_dt(&led_red) ||
	    !gpio_is_ready_dt(&switch_pin)) {
		printk("Error: GPIO pin setup failed.\n");
		return -1;
	}

	if (!device_is_ready(dht_dev)) {
		printk("Error: DHT sensor driver not ready.\n");
		return -1;
	}

	gpio_pin_configure_dt(&led_green, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led_red, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&switch_pin, GPIO_INPUT);

	udp_sock = udp_socket_init();
	if (udp_sock < 0) {
		return -1;
	}

	printk("Environment Monitor Initialized!\n");

	while (1) {
		int state = gpio_pin_get_dt(&switch_pin);

		if (state > 0) {
			gpio_pin_set_dt(&led_green, 1);
			gpio_pin_set_dt(&led_red, 0);

			if (sensor_sample_fetch(dht_dev) == 0) {
				sensor_channel_get(dht_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
				sensor_channel_get(dht_dev, SENSOR_CHAN_HUMIDITY, &humidity);

				int32_t temp_c = temp.val1;
				int32_t humidity_pct = humidity.val1;

				printk("Temp: %d.%02d C  |  Humidity: %d.%02d %%\n",
				       temp.val1, temp.val2 / 10000,
				       humidity.val1, humidity.val2 / 10000);

				if (ready == 0) {
					printk("UDP sender enabled for LAN destination %s:%d\n",
					       CONFIG_NET_CONFIG_PEER_IPV4_ADDR, UDP_PORT);
					ready = 1;
				}

				send_sensor_data_udp(udp_sock, temp_c, humidity_pct);
			} else {
				printk("Sensor read error. (Waiting 2 seconds to reset)\n");
			}

			k_msleep(2000);
		} else {
			gpio_pin_set_dt(&led_green, 0);
			gpio_pin_set_dt(&led_red, 1);
			ready = 0;
			k_msleep(100);
		}
	}

	return 0;
}
