#include "api.h"
#include "device.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/version.h"
#include "pico/unique_id.h"
#include "led-task.h"
#include "button-task.h"
#include "pi-task.h"
#include "profiling.h"

#define DEFAULT_PI_TERMS 1000000

/* ---------- тип записи таблицы ---------- */

typedef void (*api_callback_t)(const command_t *);

typedef struct
{
    const char *name;
    const char *help;
    api_callback_t callback;
} api_command_t;

/* ---------- разбор числа ---------- */

static bool parse_u32(const char *text, uint32_t *out)
{
    if (text == NULL || *text == '\0') return false;
    if (!isdigit((unsigned char)text[0])) return false;

    errno = 0;
    char *end = NULL;
    unsigned long v = strtoul(text, &end, 10);

    if (errno != 0) return false;
    if (end == text || *end != '\0') return false;
    if (v > 0xFFFFFFFFul) return false;

    *out = (uint32_t)v;
    return true;
}

/* ---------- обработчики светодиода ---------- */

static void print_led_state(void)
{
    const char *state =
        led_task_get_state() == LED_STATE_OFF   ? "off"   :
        led_task_get_state() == LED_STATE_ON    ? "on"    :
                                                  "blink";

    printf("led: %s, period %u ms\n",
           state, (unsigned)led_task_get_period_ms());
}

static void cmd_led_enable(const command_t *c)
{
    (void)c;
    led_task_set_state(LED_STATE_ON);
    print_led_state();
}

static void cmd_led_disable(const command_t *c)
{
    (void)c;
    led_task_set_state(LED_STATE_OFF);
    print_led_state();
}

static void cmd_led_blink(const command_t *c)
{
    (void)c;
    led_task_set_state(LED_STATE_BLINK);
    print_led_state();
}

static void cmd_led_period(const command_t *c)
{
    uint32_t period;

    if (c->argc != 1 || !parse_u32(c->argv[0], &period))
    {
        printf("error: usage led_period <period_ms>\n");
        return;
    }

    if (!led_task_set_period_ms(period))
    {
        printf("error: period_ms must be greater than 0\n");
        return;
    }

    print_led_state();
}

static void cmd_button(const command_t *c)
{
    (void)c;
    printf("button: %s, presses %u\n",
           button_task_is_pressed() ? "pressed" : "released",
           (unsigned)button_task_get_press_count());
}

static void cmd_pi_start(const command_t *c)
{
    if (c->argc > 1)
    {
        printf("error: usage pi_start [<terms>]\n");
        return;
    }

    uint32_t terms = DEFAULT_PI_TERMS;
    if (c->argc == 1)
    {
        if (!parse_u32(c->argv[0], &terms))
        {
            printf("error: usage pi_start [<terms>]\n");
            return;
        }
    }

    if (terms == 0)
    {
        printf("error: terms must be greater than 0\n");
        return;
    }

    if (!pi_task_start(terms))
    {
        printf("error: pi is busy\n");
        return;
    }

    printf("pi: started, %u terms\n", (unsigned)terms);
}

static void cmd_pi(const command_t *c)
{
    (void)c;

    switch (pi_task_get_state())
    {
    case PI_STATE_NOT_STARTED:
        printf("pi: not started\n");
        break;

    case PI_STATE_RUNNING:
        printf("pi: running\n");
        break;

    case PI_STATE_READY:
    {
        double value;
        uint32_t time_ms;
        pi_task_get_result(&value, &time_ms);
        printf("pi: %.9f in %u ms\n", value, (unsigned)time_ms);
        break;
    }
    }
}

static void cmd_profiling(const command_t *c)
{
    (void)c;

    for (uint32_t i = 0; i < PROFILING_MAX_STOPWATCHES; i++)
    {
        profiling_result_t r;
        if (!profiling_get(i, &r)) continue;

        if (r.count == 0)
        {
            printf("%-10s count 0\n", r.name);
            continue;
        }

        double mean = (double)r.sum_us / (double)r.count;
        printf("%-10s count %u  mean %.2f us  max %u us\n",
               r.name,
               (unsigned)r.count,
               mean,
               (unsigned)r.max_us);
    }
}

static void cmd_profiling_reset(const command_t *c)
{
    (void)c;
    profiling_reset();
    printf("profiling reset\n");
}

/* ---------- остальные обработчики ---------- */

static void cmd_info(const command_t *c)
{
    (void)c;

    device_info_t info;
    device_get_info(&info);

    printf("name: %s\n",     DEVICE_NAME);
    printf("version: %s\n",  FIRMWARE_VERSION);
    printf("project: %s\n",  DEVICE_PROJECT);
    printf("repo: %s\n",     DEVICE_REPO);
    printf("board: %s\n",    info.board);
    printf("serial: %s\n",   info.serial);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n",
           info.manufacturer, info.part, info.revision);
    printf("pico-sdk: %s\n", info.sdk_version);
}

static void cmd_uptime(const command_t *c)
{
    (void)c;
    printf("uptime: %llu ms\n",
           (unsigned long long)(time_us_64() / 1000));
}


/* ---------- таблица ---------- */

static const api_command_t commands[] =
{
    { "info",            "device passport",                            cmd_info },
    { "uptime",          "time since reset",                           cmd_uptime },
    { "led_enable",      "turn led on",                                cmd_led_enable },
    { "led_disable",     "turn led off",                               cmd_led_disable },
    { "led_blink",       "blink led",                                  cmd_led_blink },
    { "led_period",      "set blink period: led_period <period_ms>",   cmd_led_period },
    { "button", "button state and press count", cmd_button },
    { "pi_start", "start pi calculation: pi_start [terms]", cmd_pi_start },
    { "pi",       "pi result or state",                    cmd_pi       },
    { "profiling",       "stopwatch results",  cmd_profiling       },
    { "profiling_reset", "reset stopwatches",  cmd_profiling_reset },
};

#define COMMAND_COUNT (sizeof(commands) / sizeof(commands[0]))

/* ---------- точка входа ---------- */

void api_handle(const command_t *command)
{
    if (command == NULL) return;

    if (command->truncated)
    {
        printf("error: command longer than 63 characters or 4 arguments\n");
        return;
    }

    if (strcmp(command->name, "help") == 0)
    {
        printf("%-16s %s\n", "help", "list of commands");
        for (uint32_t i = 0; i < COMMAND_COUNT; i++)
        {
            printf("%-16s %s\n", commands[i].name, commands[i].help);
        }
        return;
    }

    for (uint32_t i = 0; i < COMMAND_COUNT; i++)
    {
        if (strcmp(command->name, commands[i].name) == 0)
        {
            commands[i].callback(command);
            return;
        }
    }

    printf("error: unknown command '%s', try help\n", command->name);
}
