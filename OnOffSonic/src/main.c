#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>

/*
 * Physical wiring (see boards/nucleo_h563zi.overlay for the DT side):
 *   led_green -> onboard LD1, PB0
 *   led_red   -> onboard LD3, PG4
 *   sw0       -> SPDT toggle switch, common pin on D2, throws on 3V3/GND
 *   hc_sr04   -> TRIG on D3, ECHO on D4 (via 1k/2k divider down to 3.3V)
 */
static const struct gpio_dt_spec led_green =
	GPIO_DT_SPEC_GET(DT_ALIAS(led_green), gpios);

static const struct gpio_dt_spec led_red =
	GPIO_DT_SPEC_GET(DT_ALIAS(led_red), gpios);

static const struct gpio_dt_spec switch_pin =
	GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

static const struct device *const sensor =
	DEVICE_DT_GET(DT_NODELABEL(hc_sr04));

int main(void)
{
	struct sensor_value distance;

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

	printk("\n");
	printk("*** OnOffSonic ***\n");
	printk("System Initialized Successfully!\n");
	printk("HC-SR04 driver ready.\n");

	while (1) {

		int state = gpio_pin_get_dt(&switch_pin);

		if (state > 0) {
			/* SYSTEM ON */
			gpio_pin_set_dt(&led_green, 1);
			gpio_pin_set_dt(&led_red, 0);

			int ret = sensor_sample_fetch(sensor);

			if (ret == 0) {
				ret = sensor_channel_get(
					sensor,
					SENSOR_CHAN_DISTANCE,
					&distance);

				if (ret == 0) {
					printk("Distance: %d.%06d m\n",
					       distance.val1,
					       distance.val2);
				} else {
					printk("Distance channel error: %d\n", ret);
				}
			} else {
				printk("Sensor measurement error: %d\n", ret);
			}

			k_msleep(500);

		} else {
			/* SYSTEM OFF */
			gpio_pin_set_dt(&led_green, 0);
			gpio_pin_set_dt(&led_red, 1);

			k_msleep(100);
		}
	}
}