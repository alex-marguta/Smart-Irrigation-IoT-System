#include "buttons.h"
#include "config.h"

#include "pico/stdlib.h"

void buttons_init(void)
{
    gpio_init(BUTTON_MENU);
    gpio_set_dir(BUTTON_MENU, GPIO_IN);
    gpio_pull_up(BUTTON_MENU);

    gpio_init(BUTTON_WATER);
    gpio_set_dir(BUTTON_WATER, GPIO_IN);
    gpio_pull_up(BUTTON_WATER);
}

bool menu_button_pressed(void)
{
    return !gpio_get(BUTTON_MENU);
}

bool water_button_pressed(void)
{
    return !gpio_get(BUTTON_WATER);
}