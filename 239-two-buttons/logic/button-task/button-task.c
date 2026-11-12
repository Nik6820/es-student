#include "button-task/button-task.h"
#include "platform/platform.h"
#include "systime/systime.h"

#include <stddef.h>

#define DEBOUNCE_US 20000

typedef enum {
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESS_PENDING,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASE_PENDING
} button_state_t;

typedef struct
{
    button_task_callback_t on_press;
    button_state_t         state;
    uint64_t               transition_us;
    uint32_t               press_count;
    bool                   in_use;
} button_machine_t;

static button_machine_t buttons[PLATFORM_BUTTON_COUNT];

void button_task_init(platform_button_t button, button_task_callback_t on_press)
{
    if (button >= PLATFORM_BUTTON_COUNT) return;

    buttons[button].on_press      = on_press;
    buttons[button].state         = BUTTON_STATE_RELEASED;
    buttons[button].transition_us = 0;
    buttons[button].press_count   = 0;
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
            m->press_count++;
            if (m->on_press != NULL) {
                m->on_press();
            }
            m->state = BUTTON_STATE_PRESSED;
        }
        break;

    case BUTTON_STATE_PRESSED:
        if (!pressed) {
            m->transition_us = now_us;
            m->state         = BUTTON_STATE_RELEASE_PENDING;
        }
        break;

    case BUTTON_STATE_RELEASE_PENDING:
        if (pressed) {
            m->state = BUTTON_STATE_PRESSED;
        } else if (now_us - m->transition_us >= DEBOUNCE_US) {
            m->state = BUTTON_STATE_RELEASED;
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