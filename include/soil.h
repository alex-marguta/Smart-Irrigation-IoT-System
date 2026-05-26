#ifndef SOIL_H
#define SOIL_H

#include <stdint.h>

void soil_init(void);

uint16_t soil_read_adc(void);

int soil_percent(uint16_t adc);

int soil_get_percent(void);

#endif