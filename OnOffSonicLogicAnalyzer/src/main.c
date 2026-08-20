#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

static const struct gpio_dt_spec echo_probe =
	GPIO_DT_SPEC_GET(DT_ALIAS(probe_echo), gpios);
static struct gpio_callback echo_callback;
static uint32_t echo_start;

static void echo_isr(const struct device *port,
			 struct gpio_callback *callback, uint32_t pins)
{
	uint32_t now = k_cycle_get_32();

	if (gpio_pin_get_dt(&echo_probe) > 0) {
		echo_start = now;
	} else if (echo_start != 0U) {
		uint32_t width_us = k_cyc_to_us_floor32(now - echo_start);

		printk("echo: %u us\n", width_us);
		echo_start = 0U;
	}
}

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&echo_probe)) {
		printk("ERROR: echo probe GPIO is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&echo_probe, GPIO_INPUT);
	if (ret != 0) {
		printk("ERROR: echo probe configuration failed: %d\n", ret);
		return ret;
	}

	gpio_init_callback(&echo_callback, echo_isr, BIT(echo_probe.pin));
	ret = gpio_add_callback(echo_probe.port, &echo_callback);
	if (ret != 0) {
		printk("ERROR: echo callback setup failed: %d\n", ret);
		return ret;
	}

	ret = gpio_pin_interrupt_configure_dt(&echo_probe, GPIO_INT_EDGE_BOTH);
	if (ret != 0) {
		printk("ERROR: echo interrupt setup failed: %d\n", ret);
		return ret;
	}

	printk("OnOffSonic logic probe ready\n");
	printk("Connect DUT D4 (HC-SR04 ECHO) to this board D4 and share GND.\n");

	return 0;
}