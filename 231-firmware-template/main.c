#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include "log.h"
#include "device.h"
#include "profiling.h"
#include "api.h"
#include "stdio-text-protocol.h"
#include "led-task.h"

#define BUTTON_PIN  15
#define DEBOUNCE_MS 20

static bool get_button_debounce(uint pin)
{
    bool state = gpio_get(pin);
    sleep_ms(DEBOUNCE_MS);
    return state && gpio_get(pin);
}

int main(void)
{
    stdio_init_all();

    /* Кнопка */
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    profiling_init();
    stdio_text_protocol_init();
    led_task_init();

    bool previous = true;   /* подтяжка к питанию: не нажата = true */

    while (1)
    {
        profiling_iteration();

        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL)
        {
            api_handle(command);
        }

        /* Кнопка: нажатие — фронт true → false */
        bool current = get_button_debounce(BUTTON_PIN);
        if (previous == true && current == false)
        {
            led_state_t next = (led_state_t)((led_task_get_state() + 1) % 3);
            led_task_set_state(next);
        }
        previous = current;

        led_task_handle();
    }
}