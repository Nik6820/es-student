#include "api/api-commands.h"       // свой «главный» заголовок первым (П10)
#include "api/api.h"

#include <stdio.h>
#include <stdlib.h>                 // strtoul
#include <string.h>

#include "firmware.h"

#include "led-task/led-task.h"
#include "button-task/button-task.h"
#include "pi-task/pi-task.h"
#include "device/device.h"
#include "profiling/profiling.h"

// ---------- led ----------
static void cmd_led_enable(const command_t *c)
{
    (void)c;
    led_task_set_state(LED_STATE_ON);
}

static void cmd_led_disable(const command_t *c)
{
    (void)c;
    led_task_set_state(LED_STATE_OFF);
}

static void cmd_led_blink(const command_t *c)
{
    (void)c;
    led_task_set_state(LED_STATE_BLINK);
}

static void cmd_led_period(const command_t *c)
{
    if (c->argc < 1) {
        printf("error: usage led_period <period_ms>\n");
        return;
    }
    char *end = NULL;
    unsigned long ms = strtoul(c->argv[0], &end, 10);        // ← strtoul (p2.3.1)
    if (end == c->argv[0] || *end != '\0' ||
        !led_task_set_period_ms((uint32_t)ms)) {
        printf("error: usage led_period <period_ms>\n");
    }
}

// ---------- button ----------
static void cmd_button(const command_t *c)
{
    (void)c;
    printf("presses: %lu\n",
           (unsigned long)button_task_get_press_count());
}

// ---------- pi ----------
static const char *pi_state_name(pi_state_t s)
{
    switch (s) {
    case PI_STATE_NOT_STARTED: return "not started";
    case PI_STATE_RUNNING:     return "running";
    case PI_STATE_READY:       return "ready";
    }
    return "?";
}

static void cmd_pi(const command_t *c)
{
    (void)c;
    pi_state_t s = pi_task_get_state();
    if (s == PI_STATE_READY) {
        double  v  = 0.0;
        uint32_t t = 0;
        if (pi_task_get_result(&v, &t)) {
            printf("pi: %.6f, %lu ms\n", v, (unsigned long)t);
            return;
        }
    }
    printf("pi: %s\n", pi_state_name(s));
}

static void cmd_pi_start(const command_t *c)
{
    uint32_t terms = 100000;            // значение по умолчанию
    if (c->argc >= 1) {
        char *end = NULL;
        unsigned long n = strtoul(c->argv[0], &end, 10);      // ← strtoul (p2.3.1)
        if (end != c->argv[0] && *end == '\0' && n > 0) {
            terms = (uint32_t)n;
        }
    }
    if (!pi_task_start(terms)) {
        printf("error: pi is busy\n");
    }
}

// ---------- info ----------
static void cmd_info(const command_t *c)
{
    (void)c;
    device_info_t info;
    device_get_info(&info);             // ← курс ищет этот вызов (p2.3.5)
    /* поля структуры сверьте с вашим device.h */
    printf("board:   %s\n", info.board);
    printf("chip:    %s\n", info.chip);
    printf("sdk:     %s\n", info.sdk_version);
}

// ---------- profiling ----------
static void cmd_profiling(const command_t *c)
{
    (void)c;
#if !PROFILING_ENABLED
    printf("profiling is disabled in this build\n");
#else
    profiling_print_table();            /* ← сверьте с вашим profiling.h */
#endif
}

static void cmd_profiling_reset(const command_t *c)
{
    (void)c;
#if !PROFILING_ENABLED
    printf("profiling is disabled in this build\n");
#else
    profiling_reset();                  /* ← сверьте с вашим profiling.h */
#endif
}

// ---------- version ----------
static void cmd_version(const command_t *c)
{
    (void)c;
    printf("%s %s\n", FIRMWARE_NAME, FIRMWARE_VERSION);
}

// ---------- таблица ----------
const api_command_t api_commands[] = {
    { "led_enable",     "включить светодиод",           cmd_led_enable     },
    { "led_disable",    "выключить светодиод",          cmd_led_disable    },
    { "led_blink",      "мигать",                       cmd_led_blink      },
    { "led_period",     "led_period <ms> — период",     cmd_led_period     },
    { "button",         "число нажатий",                cmd_button         },
    { "pi",             "состояние/результат π",        cmd_pi             },
    { "pi_start",       "pi_start [terms] — запуск π",  cmd_pi_start       },
    { "info",           "сведения о плате",             cmd_info           },
    { "profiling",      "таблица секундомеров",         cmd_profiling      },
    { "profiling_reset","сброс секундомеров",           cmd_profiling_reset},
    { "version",        "имя и версия прошивки",        cmd_version        },
};

const uint32_t api_command_count =
    sizeof(api_commands) / sizeof(api_commands[0]);