#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>

/* Retrieve devices from overlay aliases/nodes */
static const struct gpio_dt_spec led_green  = GPIO_DT_SPEC_GET(DT_ALIAS(led_green), gpios);
static const struct gpio_dt_spec led_red    = GPIO_DT_SPEC_GET(DT_ALIAS(led_red), gpios);
static const struct gpio_dt_spec switch_pin = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct device *const dht_dev   = DEVICE_DT_GET(DT_NODELABEL(dht_sensor));

int main(void)
{
	struct sensor_value temp, humidity;

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

	printk("Environment Monitor Initialized!\n");

	while (1) {
		int state = gpio_pin_get_dt(&switch_pin);

		if (state > 0) {
			/* --- SYSTEM ON --- */
			gpio_pin_set_dt(&led_green, 1);
			gpio_pin_set_dt(&led_red, 0);

			/* Fetch data from the DHT sensor */
			if (sensor_sample_fetch(dht_dev) == 0) {
				sensor_channel_get(dht_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
				sensor_channel_get(dht_dev, SENSOR_CHAN_HUMIDITY, &humidity);
				
				printk("Temp: %d.%02d C  |  Humidity: %d.%02d %%\n", 
				       temp.val1, temp.val2 / 10000,
				       humidity.val1, humidity.val2 / 10000);
			} else {
				printk("Sensor read error. (Waiting 2 seconds to reset)\n");
			}

			/* DHT sensors are slow; they need at least 2 seconds between reads */
			k_msleep(2000); 
		} else {
			/* --- SYSTEM OFF --- */
			gpio_pin_set_dt(&led_green, 0);
			gpio_pin_set_dt(&led_red, 1);

			k_msleep(100);
		}
	}

	return 0;
}
