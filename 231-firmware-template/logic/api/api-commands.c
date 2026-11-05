#include "api/api-callbacks.h"   // если есть; иначе сразу api/api.h
#include "api/api-commands.h"

#include "led-task/led-task.h"
#include "pi-task/pi-task.h"

static void cmd_led(const char *args)    { /* ... */ }
static void cmd_pi(const char *args)     { /* ... */ }
static void cmd_version(const char *args){ /* ... */ }

// таблица лежит рядом с обработчиками → они остаются static
const api_command_t api_commands[] = {
    { "led",     cmd_led     },
    { "pi",      cmd_pi      },
    { "version", cmd_version },
};
const uint32_t api_command_count = sizeof(api_commands) / sizeof(api_commands[0]);