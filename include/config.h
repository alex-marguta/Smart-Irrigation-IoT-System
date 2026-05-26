#ifndef CONFIG_H
#define CONFIG_H

#define WIFI_SSID "aln"
#define WIFI_PASS "cocacola"

#define I2C_PORT i2c0
#define I2C_SDA 16
#define I2C_SCL 17
#define OLED_ADDR 0x3C

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_BUF_SIZE (OLED_WIDTH * OLED_HEIGHT / 8)

#define BUTTON_MENU 14
#define BUTTON_WATER 15
#define PUMP_PIN 18

#define SOIL_ADC_PIN 26
#define SOIL_ADC_INPUT 0

#define SOIL_WET_ADC 1200
#define SOIL_DRY_ADC 3000

#define WATER_TIME_MS 3000
#define AUTO_COOLDOWN_MS 15000

#endif