#ifndef OLED_H
#define OLED_H

#include <stdbool.h>

void oled_init_display(void);
void oled_clear(void);
void oled_show(void);

void draw_pixel(int x, int y);
void draw_rect(int x, int y, int w, int h);
void draw_text(int x, int y, const char *text);
void draw_text_center(int y, const char *text);

void split_last_water(
    const char *last_water_time,
    char *date_part,
    char *time_part
);

void show_oled_status(
    const char *ip_text,
    bool manual_button,
    int oled_page,
    const char *last_water_time,
    int soil_percent,
    int auto_water_percent,
    bool pump_on
);

#endif