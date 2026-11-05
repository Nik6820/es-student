#include "led-task.h"
#include "systime/systime.h"
#include "pico/stdlib.h"

static led_state_t led_state;
static uint32_t period_ms;
static uint64_t last_toggle_us;

static void led_set(bool on)
{
    gpio_put(PICO_DEFAULT_LED_PIN, on);
}

void led_task_init(void)
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    period_ms = 1000;
    led_task_set_state(LED_STATE_BLINK);
}

void led_task_set_state(led_state_t state)
{
    led_state = state;

    switch (led_state) {
    case LED_STATE_OFF:
        led_set(false);
        break;
    case LED_STATE_ON:
        led_set(true);
        break;
    case LED_STATE_BLINK:
        last_toggle_us = 0;
        break;
    }
}

led_state_t led_task_get_state(void)
{
    return led_state;
}

bool led_task_set_period_ms(uint32_t p)
{
    if (p == 0) {
        return false;
    }
    period_ms = p;
    return true;
}

uint32_t led_task_get_period_ms(void)
{
    return period_ms;
}

void led_task_handle(void)
{
    switch (led_state) {
    case LED_STATE_OFF:
    case LED_STATE_ON:
        break;
    case LED_STATE_BLINK:
        if (systime_period_elapsed(&last_toggle_us, (uint64_t)period_ms * 1000 / 2)) {
            led_set(!gpio_get(PICO_DEFAULT_LED_PIN));
        }
        break;
    }
}

void led_task_next_state(void)
{
    led_task_set_state((led_state_t)((led_state + 1) % 3));
}