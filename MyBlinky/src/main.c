#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* size of stack area used by each thread */
#define STACKSIZE 1024
/* scheduling priority used by each thread */
#define PRIORITY 7

#define SLEEP_SLOW_MS   6000   /* green and red */
#define SLEEP_FAST_MS   200   /* yellow faster */

/* The devicetree node identifier for the "led0", "led1", "led2" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

/*The 3 colored leds(green, yellow, red)*/
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);/*green led*/
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);/*yellow led*/
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);/*red led*/

/*atribui viteza de clipire verde*/
void blink_green(void)
{
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
        while (1) {
        gpio_pin_toggle_dt(&led0);
        k_msleep(SLEEP_FAST_MS);  /*faster!*/
    }/*.........*/
}
/*atribui viteza de clipire galben*/
void blink_yellow(void)
{
  
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    while (1) {
        gpio_pin_toggle_dt(&led1);
        k_msleep(SLEEP_SLOW_MS);
              }/*- - - - - - - - - -*/
       
}
/*atribui viteza de clipire rosu*/
void blink_red(void)
{
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    while (1) {
        gpio_pin_toggle_dt(&led2);
        k_msleep(SLEEP_FAST_MS);  /*faster!*/
    }/*.........*/
}

K_THREAD_DEFINE(green_id,  STACKSIZE, blink_green,  NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(yellow_id, STACKSIZE, blink_yellow, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(red_id,    STACKSIZE, blink_red,    NULL, NULL, NULL, PRIORITY, 0, 0);

int main(void)
{

    if (!gpio_is_ready_dt(&led0) ||
        !gpio_is_ready_dt(&led1) ||
        !gpio_is_ready_dt(&led2)
        )
    {
        return 0;
    }

    
    return 0;
}
