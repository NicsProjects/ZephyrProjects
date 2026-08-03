#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* Retrieve GPIO configs from the overlay */
static const struct gpio_dt_spec led1       = GPIO_DT_SPEC_GET(DT_ALIAS(led_a), gpios);
static const struct gpio_dt_spec led2       = GPIO_DT_SPEC_GET(DT_ALIAS(led_b), gpios);
static const struct gpio_dt_spec led3       = GPIO_DT_SPEC_GET(DT_ALIAS(led_c), gpios);
static const struct gpio_dt_spec switch_pin = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

int main(void)
{
	/* Verify hardware drivers are ready */
	if (!gpio_is_ready_dt(&led1) || !gpio_is_ready_dt(&led2) || 
	    !gpio_is_ready_dt(&led3) || !gpio_is_ready_dt(&switch_pin)) {
		return -1;
	}

	/* Configure LED pins as outputs (default OFF) */
	gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);

	/* Configure Switch pin D2 as an input */
	gpio_pin_configure_dt(&switch_pin, GPIO_INPUT);

	while (1) {
		/* Read the position of the 3-pin switch */
		int state = gpio_pin_get_dt(&switch_pin);

		/* Drive all 3 LEDs based on switch state (1 = ON, 0 = OFF) */
		gpio_pin_set_dt(&led1, state);
		gpio_pin_set_dt(&led2, state);
		gpio_pin_set_dt(&led3, state);

		k_msleep(50); /* Debounce delay */
	}

	return 0;
}
