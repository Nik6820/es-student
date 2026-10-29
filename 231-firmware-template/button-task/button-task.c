#include "button-task.h"

#include "pico/stdlib.h"
#include "led-task.h"

#define BUTTON_PIN          15
#define DEBOUNCE_US         20000

typedef enum {
    BUTTON_STATE_RELEASED,       /* отпущена */
    BUTTON_STATE_PRESS_PENDING,  /* нажатие? */
    BUTTON_STATE_PRESSED,        /* нажата */
    BUTTON_STATE_RELEASE_PENDING /* отпускание? */
} button_state_t;

static button_state_t button_state;
static uint64_t transition_us;
static uint32_t press_count;

static bool button_level_pressed(void)
{
    /* нажатая кнопка замыкает вывод на землю и даёт ноль */
    return !gpio_get(BUTTON_PIN);
}

void button_task_init(void)
{
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    button_state = BUTTON_STATE_RELEASED;
    transition_us = 0;
    press_count = 0;
}

void button_task_handle(void)
{
    bool pressed = button_level_pressed();
    uint64_t now_us = time_us_64();

    switch (button_state)
    {
    case BUTTON_STATE_RELEASED:
        if (pressed)
        {
            transition_us = now_us;
            button_state = BUTTON_STATE_PRESS_PENDING;
        }
        break;

    case BUTTON_STATE_PRESS_PENDING:
        if (!pressed)
        {
            /* вернулись в отпущено раньше 20 мс — это дребезг */
            button_state = BUTTON_STATE_RELEASED;
        }
        else if (now_us - transition_us >= DEBOUNCE_US)
        {
            /* уровень продержался 20 мс — нажатие состоялось */
            press_count++;
            led_task_next_state();
            button_state = BUTTON_STATE_PRESSED;
        }
        break;

    case BUTTON_STATE_PRESSED:
        if (!pressed)
        {
            transition_us = now_us;
            button_state = BUTTON_STATE_RELEASE_PENDING;
        }
        break;

    case BUTTON_STATE_RELEASE_PENDING:
        if (pressed)
        {
            /* вернулись в нажато раньше 20 мс — дребезг */
            button_state = BUTTON_STATE_PRESSED;
        }
        else if (now_us - transition_us >= DEBOUNCE_US)
        {
            button_state = BUTTON_STATE_RELEASED;
        }
        break;
    }
}

bool button_task_is_pressed(void)
{
    /* нажата и в «нажата», и в «отпускание?» */
    return button_state == BUTTON_STATE_PRESSED
        || button_state == BUTTON_STATE_RELEASE_PENDING;
}

uint32_t button_task_get_press_count(void)
{
    return press_count;
}