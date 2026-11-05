#pragma once

#include <stdbool.h>
#include <stdint.h>

void platform_init(void);
void platform_led_set(bool on);
bool platform_button_read(void);
uint64_t platform_time_us(void);      // пригодится pi-task