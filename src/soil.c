#include "soil.h"
#include "config.h"

#include "hardware/adc.h"

void soil_init(void)
{
    adc_init();

    adc_gpio_init(SOIL_ADC_PIN);

    adc_select_input(SOIL_ADC_INPUT);
}

uint16_t soil_read_adc(void)
{
    adc_select_input(SOIL_ADC_INPUT);

    return adc_read();
}

int soil_percent(uint16_t adc)
{
    if (adc <= SOIL_WET_ADC) return 100;

    if (adc >= SOIL_DRY_ADC) return 0;

    int percent =
        (SOIL_DRY_ADC - adc) * 100 /
        (SOIL_DRY_ADC - SOIL_WET_ADC);

    if (percent < 0) percent = 0;

    if (percent > 100) percent = 100;

    return percent;
}

int soil_get_percent(void)
{
    return soil_percent(soil_read_adc());
}