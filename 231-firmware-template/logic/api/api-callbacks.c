#include "api/api-commands.h"    // свой «главный» заголовок первым

#include "led-task/led-task.h"
#include "pi-task/pi-task.h"

// ← переносите сюда реальные обработчики из старого api.c
static void cmd_led(const command_t *command)
{
    (void)command;
    // ваш код
}

static void cmd_pi(const command_t *command)
{
    (void)command;
    // ваш код
}

static void cmd_version(const command_t *command)
{
    (void)command;
    // ваш код
}

// таблица лежит рядом с обработчиками → они остаются static
const api_command_t api_commands[] = {
    { "led",     cmd_led     },
    { "pi",      cmd_pi      },
    { "version", cmd_version },
};

const uint32_t api_command_count =
    sizeof(api_commands) / sizeof(api_commands[0]);