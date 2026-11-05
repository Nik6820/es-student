#include "platform/platform.h"

#include "pico/stdlib.h"

// номера выводов знает только этот файл (было в led-task.c и button-task.c)
#ifndef PLATFORM_LED_PIN
#define PLATFORM_LED_PIN PICO_DEFAULT_LED_PIN   // 25 на pico
#endif
#ifndef PLATFORM_BUTTON_PIN
#define PLATFORM_BUTTON_PIN 15                  // ← ваш пин кнопки
#endif

void platform_init(void)
{
    stdio_init_all();

    gpio_init(PLATFORM_LED_PIN);
    gpio_set_dir(PLATFORM_LED_PIN, GPIO_OUT);

    gpio_init(PLATFORM_BUTTON_PIN);
    gpio_set_dir(PLATFORM_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(PLATFORM_BUTTON_PIN);
}

void platform_led_set(bool on)
{
    gpio_put(PLATFORM_LED_PIN, on);
}

bool platform_button_read(void)
{
    return gpio_get(PLATFORM_BUTTON_PIN) == 0;   // нажата = 0
}

uint64_t platform_time_us(void)
{
    return time_us_64();
}