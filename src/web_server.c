#include "web_server.h"
#include "web_page.h"
#include "config.h"
#include "pump.h"
#include "soil.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "pico/stdlib.h"

#include "lwip/err.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

bool web_watering = false;

absolute_time_t web_watering_stop;

char html[8192];

char last_water_time[64] = "NU A FOST UDATA";
char current_browser_time[64] = "ORA NECUNOSCUTA";

int auto_water_percent = 35;

void build_html(void)
{
    int percent = soil_get_percent();

    sprintf(
        html,
        MAIN_PAGE,
        last_water_time,
        percent,
        auto_water_percent,
        auto_water_percent,
        pump_is_on() ? "on" : "off",
        pump_is_on() ? "ON" : "OFF"
    );
}

void decode_url_time(char *dest, const char *src, int max_len)
{
    int i = 0;
    int j = 0;

    while (src[i] != '\0' && src[i] != ' ' && src[i] != '&' && j < max_len - 1) {

        if (src[i] == '+') {
            dest[j++] = ' ';
            i++;
        }
        else if (src[i] == '%' && src[i + 1] && src[i + 2]) {
            char hex[3];

            hex[0] = src[i + 1];
            hex[1] = src[i + 2];
            hex[2] = '\0';

            int value = 0;
            sscanf(hex, "%x", &value);

            dest[j++] = (char)value;
            i += 3;
        }
        else {
            dest[j++] = src[i++];
        }
    }

    dest[j] = '\0';
}

static err_t server_callback(
    void *arg,
    struct tcp_pcb *tpcb,
    struct pbuf *p,
    err_t err
)
{
    if (p == NULL) {
        tcp_close(tpcb);
        return ERR_OK;
    }

    char request[512];

    int len = p->tot_len;

    if (len > 511) {
        len = 511;
    }

    pbuf_copy_partial(p, request, len, 0);

    request[len] = '\0';

    tcp_recved(tpcb, p->tot_len);

    printf("REQUEST:\n%s\n", request);

    if (strstr(request, "GET /status")) {

        int percent = soil_get_percent();

        char json[512];

        sprintf(
            json,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Connection: close\r\n"
            "\r\n"
            "{\"last\":\"%s\",\"soil\":%d,\"limit\":%d,\"pump\":\"%s\"}",
            last_water_time,
            percent,
            auto_water_percent,
            pump_is_on() ? "ON" : "OFF"
        );

        tcp_write(tpcb, json, strlen(json), TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);

        pbuf_free(p);
        tcp_close(tpcb);

        return ERR_OK;
    }

    if (strstr(request, "GET /setlimit")) {

        char *value_start = strstr(request, "value=");

        if (value_start) {
            value_start += 6;

            int value = atoi(value_start);

            if (value < 0) value = 0;
            if (value > 100) value = 100;

            auto_water_percent = value;

            printf("NEW AUTO LIMIT: %d%%\n", auto_water_percent);
        }

        const char *response =
            "HTTP/1.1 204 No Content\r\n"
            "Connection: close\r\n"
            "\r\n";

        tcp_write(tpcb, response, strlen(response), TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);

        pbuf_free(p);
        tcp_close(tpcb);

        return ERR_OK;
    }

    if (strstr(request, "GET /time")) {

        char *time_start = strstr(request, "now=");

        if (time_start) {
            time_start += 4;

            decode_url_time(
                current_browser_time,
                time_start,
                sizeof(current_browser_time)
            );
        }

        const char *response =
            "HTTP/1.1 204 No Content\r\n"
            "Connection: close\r\n"
            "\r\n";

        tcp_write(tpcb, response, strlen(response), TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);

        pbuf_free(p);
        tcp_close(tpcb);

        return ERR_OK;
    }

    if (strstr(request, "GET /water")) {

        printf("WEB UDA ACUM\n");

        char *time_start = strstr(request, "time=");

        if (time_start) {
            time_start += 5;

            decode_url_time(
                last_water_time,
                time_start,
                sizeof(last_water_time)
            );

            strcpy(current_browser_time, last_water_time);
        }

        web_watering = true;

        web_watering_stop = make_timeout_time_ms(WATER_TIME_MS);

        pump_set(true);

        const char *response =
            "HTTP/1.1 204 No Content\r\n"
            "Connection: close\r\n"
            "\r\n";

        tcp_write(tpcb, response, strlen(response), TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);

        pbuf_free(p);
        tcp_close(tpcb);

        return ERR_OK;
    }

    build_html();

    err_t write_err = tcp_write(
        tpcb,
        html,
        strlen(html),
        TCP_WRITE_FLAG_COPY
    );

    if (write_err != ERR_OK) {
        printf("TCP WRITE ERROR: %d\n", write_err);
    }

    tcp_output(tpcb);

    pbuf_free(p);
    tcp_close(tpcb);

    return ERR_OK;
}

static err_t server_accept(
    void *arg,
    struct tcp_pcb *newpcb,
    err_t err
)
{
    if (err != ERR_OK || newpcb == NULL) {
        return ERR_VAL;
    }

    tcp_recv(newpcb, server_callback);

    return ERR_OK;
}

void start_server(void)
{
    struct tcp_pcb *pcb = tcp_new();

    if (!pcb) {
        printf("TCP NEW FAILED\n");
        return;
    }

    err_t bind_err = tcp_bind(pcb, IP_ADDR_ANY, 80);

    if (bind_err != ERR_OK) {
        printf("TCP BIND FAILED: %d\n", bind_err);
        return;
    }

    pcb = tcp_listen_with_backlog(pcb, 4);

    if (!pcb) {
        printf("TCP LISTEN FAILED\n");
        return;
    }

    tcp_accept(pcb, server_accept);

    printf("HTTP SERVER STARTED\n");
}