#pragma once

#include <stdbool.h>
#include <stdint.h>

#define PROFILING_MAX_STOPWATCHES 8

/*
 * Суммы замеров хранятся в 32 битах (uint32_t).
 * Сумма замеров одного секундомера переполнится
 * через 71 минуту непрерывной работы при среднем 1 мкс.
 * Для сеансов работы прибора этого достаточно.
 */

typedef struct
{
    const char *name;
    uint32_t    count;
    uint32_t    sum_us;
    uint32_t    max_us;
} profiling_result_t;

void profiling_stopwatch_init(uint32_t id, const char *name);

#ifndef PROFILING_ENABLED
#define PROFILING_ENABLED 1
#endif

#if PROFILING_ENABLED
void profiling_start(uint32_t id);
void profiling_stop(uint32_t id);
#else
/* профилирование выключено: вызовы ничего не делают */
#define profiling_start(id) ((void)0)
#define profiling_stop(id)  ((void)0)
#endif

/* false — секундомер с таким номером не заведён */
bool profiling_get(uint32_t id, profiling_result_t *result);

void profiling_reset(void);