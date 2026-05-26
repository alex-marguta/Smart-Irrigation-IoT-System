#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "hardware/i2c.h"

#include "lwip/netif.h"
#include "lwip/ip4_addr.h"

#include "config.h"
#include "pump.h"
#include "soil.h"
#include "buttons.h"
#include "oled.h"
#include "web_server.h"

bool auto_watering = false;

absolute_time_t auto_watering_stop;
absolute_time_t last_auto_water;

int oled_page = 0;

void save_last_water_time(void)
{
    strcpy(last_water_time, current_browser_time);
}

int main()
{
    stdio_init_all();

    sleep_ms(8000);

    printf("BOOT OK\n");

    // ================= OLED INIT =================

    i2c_init(I2C_PORT, 100000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    oled_init_display();

    oled_clear();
    draw_rect(0, 0, 128, 64);
    draw_text_center(20, "BOOT OK");
    draw_text_center(40, "STARTING");
    oled_show();

    // ================= HARDWARE INIT =================

    buttons_init();

    pump_init();

    soil_init();

    // ================= WIFI INIT =================

    if (cyw43_arch_init()) {

        oled_clear();
        draw_rect(0, 0, 128, 64);
        draw_text_center(20, "CYW43 ERROR");
        oled_show();

        printf("CYW43 INIT FAILED\n");

        while (true) {
            sleep_ms(1000);
        }
    }

    pump_set(false);

    cyw43_arch_enable_sta_mode();

    oled_clear();
    draw_rect(0, 0, 128, 64);
    draw_text_center(20, "CONNECTING");
    draw_text_center(40, "TO WIFI");
    oled_show();

    printf("CONNECTING TO WIFI: %s\n", WIFI_SSID);

    int result = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID,
        WIFI_PASS,
        CYW43_AUTH_WPA2_AES_PSK,
        30000
    );

    if (result != 0) {

        oled_clear();
        draw_rect(0, 0, 128, 64);
        draw_text_center(16, "WIFI FAILED");
        draw_text_center(36, "CHECK PASS");
        oled_show();

        printf("WIFI CONNECT FAILED: %d\n", result);

        while (true) {
            cyw43_arch_poll();
            sleep_ms(100);
        }
    }

    // ================= GET IP =================

    struct netif *netif = netif_default;
    const ip4_addr_t *ip = netif_ip4_addr(netif);

    char ip_text[32];

    snprintf(ip_text, sizeof(ip_text), "%s", ip4addr_ntoa(ip));

    printf("IP: %s\n", ip_text);

    // ================= SERVER =================

    start_server();

    // ================= MAIN LOOP =================

    absolute_time_t last_oled_update = get_absolute_time();
    absolute_time_t last_menu_press = get_absolute_time();

    bool last_menu_pressed = false;
    bool last_manual_pressed = false;

    last_auto_water = get_absolute_time();

    while (true)
    {
        cyw43_arch_poll();

        int percent = soil_get_percent();

        bool menu_pressed = menu_button_pressed();
        bool manual_pressed = water_button_pressed();

        // ================= MENU BUTTON =================

        if (!last_menu_pressed && menu_pressed) {

            if (absolute_time_diff_us(last_menu_press, get_absolute_time()) > 250000) {

                oled_page++;

                if (oled_page > 3) {
                    oled_page = 0;
                }

                last_menu_press = get_absolute_time();
            }
        }

        last_menu_pressed = menu_pressed;

        // ================= MANUAL BUTTON =================

        if (!last_manual_pressed && manual_pressed) {
            save_last_water_time();
        }

        last_manual_pressed = manual_pressed;

        // ================= WEB WATER =================

        if (web_watering) {

            pump_set(true);

            if (time_reached(web_watering_stop)) {
                web_watering = false;
                pump_set(false);
            }
        }

        // ================= MANUAL WATER =================

        else if (manual_pressed) {

            pump_set(true);
        }

        // ================= AUTO WATER =================

        else {

            int64_t cooldown_ms =
                absolute_time_diff_us(last_auto_water, get_absolute_time()) / 1000;

            if (auto_watering) {

                pump_set(true);

                if (time_reached(auto_watering_stop)) {
                    auto_watering = false;
                    pump_set(false);
                }
            }
            else if (percent <= auto_water_percent && cooldown_ms > AUTO_COOLDOWN_MS) {

                auto_watering = true;

                auto_watering_stop = make_timeout_time_ms(WATER_TIME_MS);

                last_auto_water = get_absolute_time();

                save_last_water_time();

                pump_set(true);
            }
            else {
                pump_set(false);
            }
        }

        // ================= OLED UPDATE =================

        if (absolute_time_diff_us(last_oled_update, get_absolute_time()) > 500000) {

            show_oled_status(
                ip_text,
                manual_pressed,
                oled_page,
                last_water_time,
                percent,
                auto_water_percent,
                pump_is_on()
            );

            last_oled_update = get_absolute_time();
        }

        sleep_ms(10);
    }
}