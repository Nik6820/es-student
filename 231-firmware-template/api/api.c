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

/* ---------- остальные обработчики ---------- */

static void cmd_info(const command_t *c)
{
    (void)c;

    printf("name: %s\n", DEVICE_NAME);
    printf("version: %s\n", FIRMWARE_VERSION);

    device_info();

    char serial[17];
    pico_get_unique_board_id_string(serial, sizeof(serial));
    printf("serial: %s\n", serial);

    uint32_t chip_id = *(volatile uint32_t *)0x40000000u;
    unsigned manufacturer =  chip_id         & 0xFFFu;
    unsigned part         = (chip_id >> 12)  & 0xFFFFu;
    unsigned revision     = (chip_id >> 28)  & 0xFu;
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n",
           manufacturer, part, revision);

    printf("pico-sdk: %s\n", PICO_SDK_VERSION_STRING);
}

static void cmd_uptime(const command_t *c)
{
    (void)c;
    printf("uptime: %llu ms\n",
           (unsigned long long)(time_us_64() / 1000));
}

static void cmd_calc_pi(const command_t *c)
{
    if (c->argc > 1)
    {
        printf("error: usage calc_pi [terms]\n");
        return;
    }

    uint32_t terms = 1000000;
    if (c->argc == 1)
    {
        if (!parse_u32(c->argv[0], &terms))
        {
            printf("error: usage calc_pi [terms]\n");
            return;
        }
    }

    if (terms == 0)
    {
        printf("error: terms must be greater than 0\n");
        return;
    }

    uint64_t t0 = time_us_64();

    double pi = 0.0;
    double sign = 1.0;
    for (uint32_t i = 0; i < terms; i++)
    {
        pi += sign / (2.0 * i + 1.0);
        sign = -sign;
    }
    pi *= 4.0;

    uint64_t dt = time_us_64() - t0;

    printf("pi: %.9f\n", pi);
    printf("time: %llu ms\n", (unsigned long long)(dt / 1000));
}

static void cmd_main_time_exec(const command_t *c)
{
    (void)c;
    printf("main_time_exec: not implemented\n");
}

static void cmd_main_time_reset(const command_t *c)
{
    (void)c;
    printf("main_time_reset: not implemented\n");
}

/* ---------- таблица ---------- */

static const api_command_t commands[] =
{
    { "info",            "device passport",                            cmd_info },
    { "uptime",          "time since reset",                           cmd_uptime },
    { "calc_pi",         "calculate pi: calc_pi [terms]",              cmd_calc_pi },
    { "main_time_exec",  "superloop iteration: average and maximum",   cmd_main_time_exec },
    { "main_time_reset", "reset maximum iteration",                    cmd_main_time_reset },
    { "led_enable",      "turn led on",                                cmd_led_enable },
    { "led_disable",     "turn led off",                               cmd_led_disable },
    { "led_blink",       "blink led",                                  cmd_led_blink },
    { "led_period",      "set blink period: led_period <period_ms>",   cmd_led_period },
    { "button", "button state and press count", cmd_button },
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
