#include "oled.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"

static uint8_t buffer[OLED_BUF_SIZE];

// =====================================================
// ================= OLED LOW LEVEL ====================
// =====================================================

void oled_cmd(uint8_t cmd)
{
    uint8_t data[2] = {0x00, cmd};

    i2c_write_blocking(
        I2C_PORT,
        OLED_ADDR,
        data,
        2,
        false
    );
}

void oled_data(uint8_t *data, size_t len)
{
    uint8_t temp[OLED_BUF_SIZE + 1];

    temp[0] = 0x40;

    memcpy(&temp[1], data, len);

    i2c_write_blocking(
        I2C_PORT,
        OLED_ADDR,
        temp,
        len + 1,
        false
    );
}

void oled_init_display(void)
{
    sleep_ms(100);

    uint8_t cmds[] = {
        0xAE,
        0x20,
        0x00,
        0xB0,
        0xC8,
        0x00,
        0x10,
        0x40,
        0x81,
        0x7F,
        0xA1,
        0xA6,
        0xA8,
        0x3F,
        0xA4,
        0xD3,
        0x00,
        0xD5,
        0x80,
        0xD9,
        0xF1,
        0xDA,
        0x12,
        0xDB,
        0x40,
        0x8D,
        0x14,
        0xAF
    };

    for (int i = 0; i < sizeof(cmds); i++) {
        oled_cmd(cmds[i]);
    }
}

void oled_clear(void)
{
    memset(buffer, 0, OLED_BUF_SIZE);
}

void oled_show(void)
{
    oled_cmd(0x21);
    oled_cmd(0);
    oled_cmd(127);

    oled_cmd(0x22);
    oled_cmd(0);
    oled_cmd(7);

    oled_data(buffer, OLED_BUF_SIZE);
}

// =====================================================
// ================= OLED DRAW =========================
// =====================================================

void draw_pixel(int x, int y)
{
    if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT)
        return;

    buffer[x + (y / 8) * OLED_WIDTH] |= (1 << (y % 8));
}

void draw_rect(int x, int y, int w, int h)
{
    for (int i = x; i < x + w; i++) {

        draw_pixel(i, y);

        draw_pixel(i, y + h - 1);
    }

    for (int j = y; j < y + h; j++) {

        draw_pixel(x, j);

        draw_pixel(x + w - 1, j);
    }
}

// =====================================================
// ================= FONT ==============================
// =====================================================

const uint8_t font5x7[][5] = {

    {0x7E,0x11,0x11,0x11,0x7E},
    {0x7F,0x49,0x49,0x49,0x36},
    {0x3E,0x41,0x41,0x41,0x22},
    {0x7F,0x41,0x41,0x22,0x1C},
    {0x7F,0x49,0x49,0x49,0x41},
    {0x7F,0x09,0x09,0x09,0x01},
    {0x3E,0x41,0x49,0x49,0x7A},
    {0x7F,0x08,0x08,0x08,0x7F},
    {0x00,0x41,0x7F,0x41,0x00},
    {0x20,0x40,0x41,0x3F,0x01},
    {0x7F,0x08,0x14,0x22,0x41},
    {0x7F,0x40,0x40,0x40,0x40},
    {0x7F,0x02,0x04,0x02,0x7F},
    {0x7F,0x04,0x08,0x10,0x7F},
    {0x3E,0x41,0x41,0x41,0x3E},
    {0x7F,0x09,0x09,0x09,0x06},
    {0x3E,0x41,0x51,0x21,0x5E},
    {0x7F,0x09,0x19,0x29,0x46},
    {0x46,0x49,0x49,0x49,0x31},
    {0x01,0x01,0x7F,0x01,0x01},
    {0x3F,0x40,0x40,0x40,0x3F},
    {0x1F,0x20,0x40,0x20,0x1F},
    {0x7F,0x20,0x18,0x20,0x7F},
    {0x63,0x14,0x08,0x14,0x63},
    {0x03,0x04,0x78,0x04,0x03},
    {0x61,0x51,0x49,0x45,0x43},

    {0x3E,0x51,0x49,0x45,0x3E},
    {0x00,0x42,0x7F,0x40,0x00},
    {0x42,0x61,0x51,0x49,0x46},
    {0x21,0x41,0x45,0x4B,0x31},
    {0x18,0x14,0x12,0x7F,0x10},
    {0x27,0x45,0x45,0x45,0x39},
    {0x3C,0x4A,0x49,0x49,0x30},
    {0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36},
    {0x06,0x49,0x49,0x29,0x1E},

    {0x00,0x00,0x00,0x00,0x00},
    {0x00,0x60,0x60,0x00,0x00},
    {0x08,0x08,0x08,0x08,0x08},
    {0x00,0x36,0x36,0x00,0x00},
    {0x20,0x10,0x08,0x04,0x02},
    {0x00,0x40,0x30,0x00,0x00},
    {0x23,0x13,0x08,0x64,0x62}
};

int get_char_index(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';

    if (c >= '0' && c <= '9') return 26 + (c - '0');

    if (c == ' ') return 36;

    if (c == '.') return 37;

    if (c == '-') return 38;

    if (c == ':') return 39;

    if (c == '/') return 40;

    if (c == ',') return 41;

    if (c == '%') return 42;

    return 36;
}

void draw_char(int x, int y, char c)
{
    int index = get_char_index(c);

    for (int col = 0; col < 5; col++) {

        uint8_t line = font5x7[index][col];

        for (int row = 0; row < 7; row++) {

            if (line & (1 << row)) {

                draw_pixel(x + col, y + row);
            }
        }
    }
}

void draw_text(int x, int y, const char *text)
{
    while (*text) {

        draw_char(x, y, *text);

        x += 6;

        text++;
    }
}

void draw_text_center(int y, const char *text)
{
    int len = strlen(text);

    int x = (OLED_WIDTH - len * 6) / 2;

    if (x < 0) x = 0;

    draw_text(x, y, text);
}

// =====================================================
// ================= OLED STATUS =======================
// =====================================================

void split_last_water(
    const char *last_water_time,
    char *date_part,
    char *time_part
)
{
    strcpy(date_part, "--");
    strcpy(time_part, "--");

    if (strcmp(last_water_time, "NU A FOST UDATA") == 0) {
        return;
    }

    char *comma = strchr(last_water_time, ',');

    if (comma) {

        int date_len = comma - last_water_time;

        if (date_len > 15) {
            date_len = 15;
        }

        strncpy(date_part, last_water_time, date_len);

        date_part[date_len] = '\0';

        char *time_start = comma + 1;

        while (*time_start == ' ') {
            time_start++;
        }

        strncpy(time_part, time_start, 15);

        time_part[15] = '\0';
    }
    else {

        strncpy(date_part, last_water_time, 15);

        date_part[15] = '\0';
    }
}

void show_oled_status(
    const char *ip_text,
    bool manual_button,
    int oled_page,
    const char *last_water_time,
    int soil_percent_value,
    int auto_water_percent,
    bool pump_on
)
{
    char line[32];

    char date_part[20];

    char time_part[20];

    oled_clear();

    draw_rect(0, 0, 128, 64);

    if (manual_button) {

        draw_text_center(14, "UDARE MANUALA");

        draw_text_center(36, "PUMP ON");

        oled_show();

        return;
    }

    if (oled_page == 0) {

        draw_text_center(6, "IRIGARE SMART");

        draw_text_center(24, ip_text);

        draw_text_center(
            46,
            pump_on ? "PUMP ON" : "PUMP OFF"
        );
    }
    else if (oled_page == 1) {

        split_last_water(
            last_water_time,
            date_part,
            time_part
        );

        draw_text_center(6, "ULTIMA UDARE");

        draw_text_center(24, date_part);

        draw_text_center(42, time_part);
    }
    else if (oled_page == 2) {

        draw_text_center(6, "UMIDITATE");

        sprintf(line, "%d%%", soil_percent_value);

        draw_text_center(24, line);

        draw_text_center(
            46,
            pump_on ? "PUMP ON" : "PUMP OFF"
        );
    }
    else if (oled_page == 3) {

        draw_text_center(6, "UDARE AUTO");

        draw_text_center(22, "SUB:");

        sprintf(line, "%d%%", auto_water_percent);

        draw_text_center(38, line);

        draw_text_center(
            52,
            pump_on ? "PUMP ON" : "PUMP OFF"
        );
    }

    oled_show();
}