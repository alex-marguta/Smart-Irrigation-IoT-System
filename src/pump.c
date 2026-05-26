#include "pump.h"
#include "config.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

static bool pump_state = false;

void pump_init(void)
{
    gpio_init(PUMP_PIN);
    gpio_set_dir(PUMP_PIN, GPIO_OUT);

    pump_state = false;
    gpio_put(PUMP_PIN, 0);
}

void pump_set(bool state)
{
    pump_state = state;

    gpio_put(PUMP_PIN, state ? 1 : 0);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, state ? 1 : 0);
}

bool pump_is_on(void)
{
    return pump_state;
}