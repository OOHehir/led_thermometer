/* HTTP GET Example using plain POSIX sockets

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
//#include "nvs_flash.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "sdkconfig.h"
#include "http_get.h"

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "eu-api.openweathermap.org"
#define WEB_PORT "80"

// Current weather
// #define WEB_PATH "/data/2.5/weather?q="OPENWEATHERMAP_LOCATION"&appid="OPENWEATHERMAP_API_KEY"&units=metric"

// 5 day forecast - 3 hour intervals. Change cnt=4 to cnt=8 for 24 hour intervals
#define WEB_PATH "/data/2.5/forecast?q="OPENWEATHERMAP_LOCATION"&cnt=4&appid="OPENWEATHERMAP_API_KEY"&units=metric"

#ifndef OPENWEATHERMAP_API_KEY
    #error "OPENWEATHERMAP_API_KEY is not defined"
#endif

static const char *TAG = __FILE__;

static const char *REQUEST = "GET "WEB_PATH" HTTP/1.0\r\n"
    "Host: api.openweathermap.org\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";

light_animate_and_set_cb_t light_animate_and_set_cb;

typedef void (*light_animate_and_set_wrapper_t)(const int, const int, const int);

/**
 * @brief Wrapper to access light_animate_and_set function in light_driver.c
 */
void light_animate_and_set_wrapper(const int temp_min, const int temp_now, const int temp_max) {
  if (0) {
    ESP_LOGI(TAG, "%s, temp_min=%d, temp_now=%d, temp_max=%d", __func__, temp_min, temp_now, temp_max);
  }
  light_animate_and_set_cb(temp_min, temp_now, temp_max);
}

void http_get_task(void *pvParameters)
{
    light_animate_and_set_wrapper_t light_animateSet = (light_animate_and_set_wrapper_t)pvParameters;

    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[64];
    int temp_min = 0;
    int temp_now = 0;
    int temp_max = 0;

    while (1) {
        if (esp_wifi_connect() != ESP_OK) {
            ESP_LOGE(TAG, "Failed to connect to wifi");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

        if (err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed of %s:%s err=%d res=%p", WEB_SERVER, WEB_PORT, err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.
        Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if (s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if (connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        ESP_LOGI(TAG, "Request: %s", REQUEST);
        freeaddrinfo(res);

        if (write(s, REQUEST, strlen(REQUEST)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");
            ESP_ERROR_CHECK(close(s));
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            ESP_ERROR_CHECK(close(s));
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... set socket receiving timeout success");

        /* Read HTTP response */
        do {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
            for (int i = 0; i < r; i++) {
                putchar(recv_buf[i]);
            }
        } while (r > 0);
        putchar('\n');
        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
        ESP_ERROR_CHECK(close(s));

        // Push results to led driver
        light_animateSet(temp_min, temp_now, temp_max);

        for (int countdown = 60; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
    }
}

void http_get_init(void(*light_animate_and_set_cb_remote)(const int temp_min, const int temp_now, const int temp_max)){
    light_animate_and_set_cb = light_animate_and_set_cb_remote;
    xTaskCreate(&http_get_task, "http_get_task", 4096, (void *)&light_animate_and_set_wrapper, 5, NULL);
}