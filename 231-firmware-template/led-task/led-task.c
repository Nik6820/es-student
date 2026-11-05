#include "led-task.h"
#include "systime/systime.h"
#include "pico/stdlib.h"

static led_state_t led_state;
static uint32_t period_ms;
static uint64_t last_toggle_us;

static void led_set(bool on)
{
    gpio_put(PICO_DEFAULT_LED_PIN, on);
}

/* period_elapsed УДАЛИТЬ — теперь в systime */

void led_task_init(void) { /* без изменений */ }
void led_task_set_state(led_state_t state) { /* без изменений */ }
led_state_t led_task_get_state(void) { /* без изменений */ }
bool led_task_set_period_ms(uint32_t p) { /* без изменений */ }
uint32_t led_task_get_period_ms(void) { /* без изменений */ }

void led_task_handle(void)
{
    switch (led_state) {
    case LED_STATE_OFF:
    case LED_STATE_ON:
        break;
    case LED_STATE_BLINK:
        if (systime_period_elapsed(&last_toggle_us, (uint64_t)period_ms * 1000 / 2)) {
            led_set(!gpio_get(PICO_DEFAULT_LED_PIN));
        }
        break;
    }
}

void led_task_next_state(void) { /* без изменений */ }