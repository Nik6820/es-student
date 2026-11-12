#include "api/api-commands.h"

#include "firmware.h"
#include "device/device.h"
#include "systime/systime.h"
#include "profiling/profiling.h"
#include "led-task/led-task.h"
#include "button-task/button-task.h"
#include "pi-task/pi-task.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>

#define DEFAULT_PI_TERMS 1000000

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

static void print_led_state(void)
{
    const char *state =
        led_task_get_state() == LED_STATE_OFF   ? "off"   :
        led_task_get_state() == LED_STATE_ON    ? "on"    :
                                                  "blink";

    printf("led: %s, period %u ms\n",
           state, (unsigned)led_task_get_period_ms());
}

static void cmd_led_enable(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_ON);
    print_led_state();
}

static void cmd_led_disable(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_OFF);
    print_led_state();
}

static void cmd_led_blink(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_BLINK);
    print_led_state();
}

static void cmd_led_period(const command_t *command)
{
    uint32_t period;

    if (command->argc != 1 || !parse_u32(command->argv[0], &period))
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

static void cmd_pi_start(const command_t *command)
{
    if (command->argc > 1)
    {
        printf("error: usage pi_start [<terms>]\n");
        return;
    }

    uint32_t terms = DEFAULT_PI_TERMS;
    if (command->argc == 1)
    {
        if (!parse_u32(command->argv[0], &terms))
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

static void cmd_pi(const command_t *command)
{
    (void)command;

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

static void cmd_profiling(const command_t *command)
{
    (void)command;

#if PROFILING_ENABLED
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
#else
    printf("profiling is disabled in this build\n");
#endif
}

static void cmd_profiling_reset(const command_t *command)
{
    (void)command;
    profiling_reset();
    printf("profiling reset\n");
}

static void cmd_button(const command_t *command)
{
    (void)command;

    for (uint32_t i = 0; i < PLATFORM_BUTTON_COUNT; i++)
    {
        platform_button_t b = (platform_button_t)i;
        printf("button %u: %s, presses %u\n",
               (unsigned)(i + 1),
               button_task_is_pressed(b) ? "pressed" : "released",
               (unsigned)button_task_get_press_count(b));
    }
}

static void cmd_info(const command_t *command)
{
    (void)command;

    device_info_t info;
    device_get_info(&info);

    printf("name: %s\n",     FIRMWARE_NAME);
    printf("version: %s\n",  FIRMWARE_VERSION);
    printf("project: %s\n",  FIRMWARE_PROJECT);
    printf("repo: %s\n",     FIRMWARE_REPO);
    printf("board: %s\n",    info.board);
    printf("serial: %s\n",   info.serial);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n",
           info.manufacturer, info.part, info.revision);
    printf("pico-sdk: %s\n", info.sdk_version);
}

static void cmd_uptime(const command_t *command)
{
    (void)command;
    printf("uptime: %llu ms\n", (unsigned long long)systime_ms());
}

const api_command_t api_commands[] =
{
    { "info",            "device passport",                            cmd_info },
    { "uptime",          "time since reset",                           cmd_uptime },
    { "led_enable",      "turn led on",                                cmd_led_enable },
    { "led_disable",     "turn led off",                               cmd_led_disable },
    { "led_blink",       "blink led",                                  cmd_led_blink },
    { "led_period",      "set blink period: led_period <period_ms>",   cmd_led_period },
    { "button",          "button state and press count",               cmd_button },
    { "pi_start",        "start pi calculation: pi_start [terms]",     cmd_pi_start },
    { "pi",              "pi result or state",                         cmd_pi },
    { "profiling",       "stopwatch results",                          cmd_profiling },
    { "profiling_reset", "reset stopwatches",                          cmd_profiling_reset },
};

const uint32_t api_command_count =
    sizeof(api_commands) / sizeof(api_commands[0]);