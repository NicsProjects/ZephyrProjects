#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>

/* Retrieve devices */
static const struct gpio_dt_spec led_green = GPIO_DT_SPEC_GET(DT_ALIAS(led_green), gpios);
static const struct gpio_dt_spec led_red = GPIO_DT_SPEC_GET(DT_ALIAS(led_red), gpios);
static const struct device *const dht_dev = DEVICE_DT_GET(DT_NODELABEL(dht_sensor));
static const struct device *const sonic_dev = DEVICE_DT_GET(DT_NODELABEL(hc_sr04));

int main(void)
{
	struct sensor_value temp, humidity, distance;

	/* Verify devices */
	if (!gpio_is_ready_dt(&led_green) || !gpio_is_ready_dt(&led_red)) {
		printk("Error: GPIO LEDs not ready.\n");
		return -1;
	}

	if (!device_is_ready(dht_dev) || !device_is_ready(sonic_dev)) {
		printk("Error: One or both sensors not ready.\n");
		gpio_pin_set_dt(&led_red, 1);
		return -1;
	}

	/* Configure LEDs */
	gpio_pin_configure_dt(&led_green, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led_red, GPIO_OUTPUT_INACTIVE);

	printk("--- Weather & Radar Station Online ---\n");

	while (1) {
		int dht_ok = sensor_sample_fetch(dht_dev);
		int sonic_ok = sensor_sample_fetch(sonic_dev);

		if (dht_ok == 0 && sonic_ok == 0) {
			/* Success! Blink Green LED */
			gpio_pin_set_dt(&led_green, 1);
			gpio_pin_set_dt(&led_red, 0);

			/* Get data channels */
			sensor_channel_get(dht_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
			sensor_channel_get(dht_dev, SENSOR_CHAN_HUMIDITY, &humidity);
			sensor_channel_get(sonic_dev, SENSOR_CHAN_DISTANCE, &distance);
			
			/* Print Dashboard */
			printk("Temp: %d.%02d C | Hum: %d.%02d %% | Dist: %d.%02d m\n", 
			       temp.val1, temp.val2 / 10000,
			       humidity.val1, humidity.val2 / 10000,
			       distance.val1, distance.val2 / 10000);
			
			/* Turn off green LED after a brief flash */
			k_msleep(100);
			gpio_pin_set_dt(&led_green, 0);
			k_msleep(1900); /* Wait the remaining time */

		} else {
			/* Error occurred! Light Red LED */
			gpio_pin_set_dt(&led_green, 0);
			gpio_pin_set_dt(&led_red, 1);
			
			printk("Error reading sensors! Retrying...\n");
			k_msleep(2000);
		}
	}

	return 0;
} 
