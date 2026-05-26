#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <stdbool.h>
#include "pico/stdlib.h"

extern bool web_watering;

extern absolute_time_t web_watering_stop;

extern char last_water_time[64];
extern char current_browser_time[64];

extern int auto_water_percent;

void start_server(void);
void build_html(void);

#endif