#include "profiling.h"

#include "pico/stdlib.h"

typedef struct
{
    const char *name;
    uint64_t    start_us;
    uint32_t    count;
    uint32_t    sum_us;
    uint32_t    max_us;
    bool        in_use;
    bool        running;
} stopwatch_t;

static stopwatch_t watches[PROFILING_MAX_STOPWATCHES];

void profiling_stopwatch_init(uint32_t id, const char *name)
{
    if (id >= PROFILING_MAX_STOPWATCHES) return;

    watches[id].name     = name;
    watches[id].start_us = 0;
    watches[id].count    = 0;
    watches[id].sum_us   = 0;
    watches[id].max_us   = 0;
    watches[id].in_use   = true;
    watches[id].running  = false;
}

void profiling_start(uint32_t id)
{
    if (id >= PROFILING_MAX_STOPWATCHES) return;
    if (!watches[id].in_use) return;

    watches[id].start_us = time_us_64();
    watches[id].running  = true;
}

void profiling_stop(uint32_t id)
{
    if (id >= PROFILING_MAX_STOPWATCHES) return;
    if (!watches[id].in_use) return;
    if (!watches[id].running) return;

    uint32_t elapsed = (uint32_t)(time_us_64() - watches[id].start_us);

    watches[id].count++;
    watches[id].sum_us += elapsed;
    if (elapsed > watches[id].max_us) {
        watches[id].max_us = elapsed;
    }
    watches[id].running = false;
}

bool profiling_get(uint32_t id, profiling_result_t *result)
{
    if (id >= PROFILING_MAX_STOPWATCHES) return false;
    if (!watches[id].in_use) return false;
    if (result == NULL) return false;

    result->name   = watches[id].name;
    result->count  = watches[id].count;
    result->sum_us = watches[id].sum_us;
    result->max_us = watches[id].max_us;
    return true;
}

void profiling_reset(void)
{
    for (uint32_t i = 0; i < PROFILING_MAX_STOPWATCHES; i++)
    {
        watches[i].count  = 0;
        watches[i].sum_us = 0;
        watches[i].max_us = 0;
    }
}