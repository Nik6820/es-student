#include "button-task/button-task.h"
#include "platform/platform.h"
#include "systime/systime.h"

#include <stddef.h>     /* NULL */

#define DEBOUNCE_US 20000

typedef enum {
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESS_PENDING,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASE_PENDING
} button_state_t;

static button_task_callback_t on_press_cb;
static button_state_t button_state;
static uint64_t transition_us;
static uint32_t press_count;

void button_task_init(button_task_callback_t on_press)
{
    on_press_cb   = on_press;
    button_state  = BUTTON_STATE_RELEASED;
    transition_us = 0;
    press_count   = 0;
}

void button_task_handle(void)
{
    bool pressed    = platform_button_read();
    uint64_t now_us = systime_us();

    switch (button_state)
    {
    case BUTTON_STATE_RELEASED:
        if (pressed) {
            transition_us = now_us;
            button_state  = BUTTON_STATE_PRESS_PENDING;
        }
        break;

    case BUTTON_STATE_PRESS_PENDING:
        if (!pressed) {
            button_state = BUTTON_STATE_RELEASED;
        } else if (now_us - transition_us >= DEBOUNCE_US) {
            press_count++;
            if (on_press_cb != NULL) {
                on_press_cb();
            }
            button_state = BUTTON_STATE_PRESSED;
        }
        break;

    case BUTTON_STATE_PRESSED:
        if (!pressed) {
            transition_us = now_us;
            button_state  = BUTTON_STATE_RELEASE_PENDING;
        }
        break;

    case BUTTON_STATE_RELEASE_PENDING:
        if (pressed) {
            button_state = BUTTON_STATE_PRESSED;
        } else if (now_us - transition_us >= DEBOUNCE_US) {
            button_state = BUTTON_STATE_RELEASED;
        }
        break;
    }
}

bool button_task_is_pressed(void)
{
    return button_state == BUTTON_STATE_PRESSED
        || button_state == BUTTON_STATE_RELEASE_PENDING;
}

uint32_t button_task_get_press_count(void)
{
    return press_count;
}