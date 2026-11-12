#include "button-task/button-task.h"
#include "platform/platform.h"
#include "systime/systime.h"

#include <stddef.h>

#define DEBOUNCE_US   20000
#define LONG_PRESS_US 1000000 

typedef enum {
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESS_PENDING,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASE_PENDING
} button_state_t;

typedef struct
{
    button_task_callback_t on_short;
    button_task_callback_t on_long;
    button_state_t         state;
    uint64_t               transition_us;   /* для DEBOUNCE */
    uint64_t               press_start_us;  /* для LONG_PRESS */
    uint32_t               press_count;
    bool                   long_reported;   /* о долгом уже сообщили в этом цикле */
    bool                   in_use;
} button_machine_t;

static button_machine_t buttons[PLATFORM_BUTTON_COUNT];
void button_task_init(platform_button_t button,
                      button_task_callback_t on_short,
                      button_task_callback_t on_long)
{
    if (button >= PLATFORM_BUTTON_COUNT) return;

    buttons[button].on_short      = on_short;
    buttons[button].on_long       = on_long;
    buttons[button].state         = BUTTON_STATE_RELEASED;
    buttons[button].transition_us = 0;
    buttons[button].press_start_us = 0;
    buttons[button].press_count   = 0;
    buttons[button].long_reported = false;
    buttons[button].in_use        = true;
}

static void handle_one(platform_button_t button)
{
    button_machine_t *m = &buttons[button];
    if (!m->in_use) return;

    bool pressed    = platform_button_read(button);
    uint64_t now_us = systime_us();

    switch (m->state)
    {
    case BUTTON_STATE_RELEASED:
        if (pressed) {
            m->transition_us = now_us;
            m->state         = BUTTON_STATE_PRESS_PENDING;
        }
        break;

    case BUTTON_STATE_PRESS_PENDING:
        if (!pressed) {
            m->state = BUTTON_STATE_RELEASED;
        } else if (now_us - m->transition_us >= DEBOUNCE_US) {
            /* Уровень продержался 20 мс — нажатие подтверждено */
            m->press_count++;
            m->press_start_us = now_us;
            m->long_reported  = false;
            m->state          = BUTTON_STATE_PRESSED;

            /* Если у кнопки нет долгого действия — короткое сообщаем сразу */
            if (m->on_long == NULL && m->on_short != NULL) {
                m->on_short();
            }
        }
        break;

    case BUTTON_STATE_PRESSED:
        /* Проверяем долгое нажатие ДО перехода к отпусканию:
           если сейчас уже 1 с, то это долгое, а не короткое */
        if (m->on_long != NULL && !m->long_reported &&
            now_us - m->press_start_us >= LONG_PRESS_US) {
            m->on_long();
            m->long_reported = true;
        }

        if (!pressed) {
            m->transition_us = now_us;
            m->state         = BUTTON_STATE_RELEASE_PENDING;
        }
        break;

    case BUTTON_STATE_RELEASE_PENDING:
        if (pressed) {
            m->state = BUTTON_STATE_PRESSED;   /* дребезг, ждём снова */
        } else if (now_us - m->transition_us >= DEBOUNCE_US) {
            m->state = BUTTON_STATE_RELEASED;

            /* Отпускание подтверждено. Если у кнопки есть долгое действие,
               а о долгом не сообщали — это было короткое нажатие */
            if (m->on_long != NULL && !m->long_reported && m->on_short != NULL) {
                m->on_short();
            }
        }
        break;
    }
}

void button_task_handle(void)
{
    for (uint32_t i = 0; i < PLATFORM_BUTTON_COUNT; i++)
    {
        handle_one((platform_button_t)i);
    }
}

bool button_task_is_pressed(platform_button_t button)
{
    if (button >= PLATFORM_BUTTON_COUNT) return false;
    return buttons[button].state == BUTTON_STATE_PRESSED
        || buttons[button].state == BUTTON_STATE_RELEASE_PENDING;
}

uint32_t button_task_get_press_count(platform_button_t button)
{
    if (button >= PLATFORM_BUTTON_COUNT) return 0;
    return buttons[button].press_count;
}