#include "api.h"
#include "device.h"
#include "led.h"
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

/* ---------- обработчики ---------- */

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

/* main_time_exec / main_time_reset — перенеси обработчики из 221-command-time
   без изменений, добавь к ним help-строки. Ниже — заглушки, замени своими. */

static void cmd_main_time_exec(const command_t *c)
{
    (void)c;
    /* TODO: перенести из 221-command-time */
    printf("main_time_exec: not implemented\n");
}

static void cmd_main_time_reset(const command_t *c)
{
    (void)c;
    /* TODO: перенести из 221-command-time */
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
};

#define COMMAND_COUNT (sizeof(commands) / sizeof(commands[0]))

/* ---------- точка входа ---------- */

void api_handle(const command_t *command)
{
    if (command == NULL) return;

    /* 1. переполнение */
    if (command->truncated)
    {
        printf("error: command longer than 63 characters or 4 arguments\n");
        return;
    }

    /* 2. help — обрабатывается самой api */
    if (strcmp(command->name, "help") == 0)
    {
        printf("%-16s %s\n", "help", "list of commands");
        for (uint32_t i = 0; i < COMMAND_COUNT; i++)
        {
            printf("%-16s %s\n", commands[i].name, commands[i].help);
        }
        return;
    }

    /* 3. поиск в таблице */
    for (uint32_t i = 0; i < COMMAND_COUNT; i++)
    {
        if (strcmp(command->name, commands[i].name) == 0)
        {
            commands[i].callback(command);
            return;
        }
    }

    /* 4. неизвестная */
    printf("error: unknown command '%s', try help\n", command->name);
}