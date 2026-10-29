#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include "log.h"
#include "device.h"
#include "profiling.h"
#include "api.h"
#include "stdio-text-protocol.h"
#include "led-task.h"
#include "button-task.h"    


int main(void)
{
    stdio_init_all();

    profiling_init();
    stdio_text_protocol_init();
    led_task_init();
    button_task_init();

    bool previous = true;   /* подтяжка к питанию: не нажата = true */

    while (1)
    {
        profiling_iteration();

        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL)
        {
            api_handle(command);
        }
        button_task_handle();
        led_task_handle();
    }
}