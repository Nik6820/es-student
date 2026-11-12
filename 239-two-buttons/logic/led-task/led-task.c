#include "led-task/led-task.h"      // свой заголовок первым (П10)
#include "platform/platform.h"      // единственный, кто знает про железо
#include "systime/systime.h"        // это не SDK, лежит в libs/ — можно

static led_state_t led_state;
static uint32_t    period_ms;
static uint64_t    last_toggle_us;
static bool        led_lit;         // ← замена gpio_get(PICO_DEFAULT_LED_PIN)

// единственное место модуля, где трогаем светодиод
static void led_set(bool on)
{
    led_lit = on;
    platform_led_set(on);
}

void led_task_init(void)
{
    period_ms = 1000;
    led_set(false);                 // сброс в известное состояние
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
            led_set(!led_lit);      // было: led_set(!gpio_get(PICO_DEFAULT_LED_PIN));
        }
        break;
    }
}

void led_task_next_state(void)
{
    led_task_set_state((led_state_t)((led_state + 1) % 3));
}

static const uint32_t PERIOD_CYCLE[] = { 250, 500, 1000 };
#define PERIOD_CYCLE_COUNT (sizeof(PERIOD_CYCLE) / sizeof(PERIOD_CYCLE[0]))

void led_task_next_period(void)
{
    for (uint32_t i = 0; i < PERIOD_CYCLE_COUNT; i++)
    {
        if (PERIOD_CYCLE[i] == period_ms)
        {
            period_ms = PERIOD_CYCLE[(i + 1) % PERIOD_CYCLE_COUNT];
            return;
        }
    }

    /* период вне круга — начинаем с первого */
    period_ms = PERIOD_CYCLE[0];
}

void led_task_blink(void)
{
    led_task_set_state(LED_STATE_BLINK);
}