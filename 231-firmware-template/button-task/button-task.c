#include "button-task.h"
#include "systime/systime.h"
#include "pico/stdlib.h"
#include "led-task.h"

/* ... */

void button_task_handle(void)
{
    bool pressed = button_level_pressed();
    uint64_t now_us = systime_us();
    /* ... остальное без изменений ... */
}