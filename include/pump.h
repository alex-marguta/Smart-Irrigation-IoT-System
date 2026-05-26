#ifndef PUMP_H
#define PUMP_H

#include <stdbool.h>

void pump_init(void);
void pump_set(bool state);
bool pump_is_on(void);

#endif