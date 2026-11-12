#pragma once

#include <stdbool.h>
#include <stdint.h>

void pi_task_init(void);
void pi_task_handle(void);

/* false — расчёт уже идёт, второй запуск не принят */
bool pi_task_start(uint32_t terms);

/* Готов ли результат. Если да — записать значение и время, вернуть true */
bool pi_task_get_result(double *value, uint32_t *time_ms);

/* Состояние для команды pi: not started / running / ready */
typedef enum {
    PI_STATE_NOT_STARTED,
    PI_STATE_RUNNING,
    PI_STATE_READY,
} pi_state_t;

pi_state_t pi_task_get_state(void);