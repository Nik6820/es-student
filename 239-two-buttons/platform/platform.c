#include "platform/platform.h"

#include "pico/stdlib.h"

#include "systime/systime.h"

#ifndef PLATFORM_LED_PIN
#define PLATFORM_LED_PIN PICO_DEFAULT_LED_PIN
#endif

/* Выводы кнопок: GP15 и GP14 — соседние */
static const uint button_pins[PLATFORM_BUTTON_COUNT] = {
    [PLATFORM_BUTTON_1] = 15,
    [PLATFORM_BUTTON_2] = 14,
};

void platform_init(void)
{
    stdio_init_all();

    gpio_init(PLATFORM_LED_PIN);
    gpio_set_dir(PLATFORM_LED_PIN, GPIO_OUT);

    for (uint i = 0; i < PLATFORM_BUTTON_COUNT; i++)
    {
        gpio_init(button_pins[i]);
        gpio_set_dir(button_pins[i], GPIO_IN);
        gpio_pull_up(button_pins[i]);
    }
}

void platform_led_set(bool on)
{
    gpio_put(PLATFORM_LED_PIN, on);
}

bool platform_button_read(platform_button_t button)
{
    if (button >= PLATFORM_BUTTON_COUNT) return false;
    return gpio_get(button_pins[button]) == 0;
}

uint64_t platform_time_us(void)
{
    return systime_us();
}