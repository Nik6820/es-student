#include "pi-task.h"

#include "pico/stdlib.h"

/* За член ряда: 4 операции с double, ~585 тактов при 125 МГц = 4,7 мкс.
   Порция 100 членов — 470 мкс, укладывается в 0,5 мс (Н1). */
#define PI_TERMS_PER_CALL 100

/* ---------- состояние задачи ---------- */

static pi_state_t pi_state;
static uint32_t pi_k;          /* текущий член */
static uint32_t pi_n;          /* сколько всего членов */
static double pi_sum;
static double pi_sign;

static double pi_value;        /* готовый результат */
static uint32_t pi_time_ms;    /* сколько считалось */
static uint64_t pi_start_us;   /* метка старта — для измерения времени */

void pi_task_init(void)
{
    pi_state = PI_STATE_NOT_STARTED;
    pi_k = 0;
    pi_n = 0;
    pi_sum = 0.0;
    pi_sign = 1.0;
    pi_value = 0.0;
    pi_time_ms = 0;
    pi_start_us = 0;
}

bool pi_task_start(uint32_t terms)
{
    if (pi_state == PI_STATE_RUNNING)
    {
        return false;
    }

    pi_n = terms;
    pi_k = 0;
    pi_sum = 0.0;
    pi_sign = 1.0;
    pi_start_us = time_us_64();
    pi_state = PI_STATE_RUNNING;
    return true;
}

void pi_task_handle(void)
{
    if (pi_state != PI_STATE_RUNNING)
    {
        return;
    }

    /* Конец порции. Возле 4 294 967 295 сумма переполнится, поэтому
       проверяем, не оказался ли конец меньше начала. */
    uint32_t end = pi_k + PI_TERMS_PER_CALL;
    if (end < pi_k || end > pi_n)
    {
        end = pi_n;
    }

    for (; pi_k < end; pi_k++)
    {
        pi_sum += pi_sign / (2.0 * (double)pi_k + 1.0);
        pi_sign = -pi_sign;
    }

    if (pi_k >= pi_n)
    {
        pi_value = 4.0 * pi_sum;
        pi_time_ms = (uint32_t)((time_us_64() - pi_start_us) / 1000);
        pi_state = PI_STATE_READY;
    }
}

bool pi_task_get_result(double *value, uint32_t *time_ms)
{
    if (pi_state != PI_STATE_READY)
    {
        return false;
    }
    if (value   != NULL) *value   = pi_value;
    if (time_ms != NULL) *time_ms = pi_time_ms;
    return true;
}

pi_state_t pi_task_get_state(void)
{
    return pi_state;
}