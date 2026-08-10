#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>

/* Retrieve devices from overlay aliases */
static const struct gpio_dt_spec led_green  = GPIO_DT_SPEC_GET(DT_ALIAS(led_green), gpios);
static const struct gpio_dt_spec led_red    = GPIO_DT_SPEC_GET(DT_ALIAS(led_red), gpios);
static const struct gpio_dt_spec switch_pin = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct device *const sensor    = DEVICE_DT_GET(DT_NODELABEL(hc_sr04));

int main(void)
{
	struct sensor_value distance;

	if (!gpio_is_ready_dt(&led_green) || !gpio_is_ready_dt(&led_red) || 
	    !gpio_is_ready_dt(&switch_pin)) {
		printk("Error: GPIO pin setup failed.\n");
		return -1;
	}

	if (!device_is_ready(sensor)) {
		printk("Error: HC-SR04 sensor driver not ready.\n");
		return -1;
	}

	gpio_pin_configure_dt(&led_green, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led_red, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&switch_pin, GPIO_INPUT);

	printk("System Initialized Successfully!\n");

	while (1) {
		int state = gpio_pin_get_dt(&switch_pin);

		if (state > 0) {
			/* --- SYSTEM ON --- */
			gpio_pin_set_dt(&led_green, 1);
			gpio_pin_set_dt(&led_red, 0);


			if (sensor_sample_fetch(sensor) == 0) {
				sensor_channel_get(sensor, SENSOR_CHAN_DISTANCE, &distance);
				printk("Distance: %d.%02d meters\n", 
				       distance.val1, distance.val2 / 10000);
			} else {
				printk("Sensor measurement timeout or error.\n");
			}

			k_msleep(500); /* Sensor update rate (500ms) */
		} else {
			/* --- SYSTEM OFF --- */
			gpio_pin_set_dt(&led_green, 0);
			gpio_pin_set_dt(&led_red, 1);

			k_msleep(100);
		}
	}

	return 0;
}
