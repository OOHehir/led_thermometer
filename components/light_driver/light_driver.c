/*
 * SPDX-FileCopyrightText: 2021-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: LicenseRef-Included
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Espressif Systems
 *    integrated circuit in a product or a software update for such product,
 *    must reproduce the above copyright notice, this list of conditions and
 *    the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * 4. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "esp_log.h"
#include "led_strip.h"
#include "light_driver.h"

static const char *TAG = __FILE__;

static led_strip_handle_t s_led_strip;
static uint8_t s_red = 255, s_green = 255, s_blue = 255;

void light_driver_set_power(bool power) {
    ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, 0, s_red * power, s_green * power, s_blue * power));
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
}

void light_driver_set_color(uint32_t red, uint32_t green, uint32_t blue) {
    s_red = red;
    s_green = green;
    s_blue = blue;
    ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, 0, s_red, s_green, s_blue));
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
}

void light_driver_init(bool power) {
    led_strip_config_t led_strip_conf = {
        .max_leds = CONFIG_EXAMPLE_STRIP_LED_NUMBER,
        .strip_gpio_num = CONFIG_EXAMPLE_STRIP_LED_GPIO,
        .led_model = LED_MODEL_WS2812,           // LED strip model
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,    // The color order of the strip: GRB
        .flags = {
            .invert_out = false,                 // don't invert the output signal
        }
    };
    led_strip_rmt_config_t rmt_conf = {
        .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
        .resolution_hz = 10 * 1000 * 1000,      // 10MHz
        .mem_block_symbols = 64,               // the memory size of each RMT channel, in words (4 bytes)
        .flags = {
            .with_dma = false,                  // DMA feature is available on chips like ESP32-S3/P4
        }
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&led_strip_conf, &rmt_conf, &s_led_strip));
    light_driver_set_power(power);
}

void light_animate_and_set(const int temp_min, const int temp_now, const int temp_max) {
    ESP_LOGI(TAG, "light_animate_and_set: temp_min=%d, temp_now=%d, temp_max=%d", temp_min, temp_now, temp_max);
    // First animate
    for (int index = 0; index < CONFIG_EXAMPLE_STRIP_LED_NUMBER; index++) {
        ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, index, 5, 5, 5));
        ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
        vTaskDelay(pdMS_TO_TICKS(50));
        ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, index, 0, 0, 0));
    }
    for (int index = CONFIG_EXAMPLE_STRIP_LED_NUMBER - 1; index > 0; index--) {
        ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, index, 5, 5, 5));
        ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
        vTaskDelay(pdMS_TO_TICKS(50));
        ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, index, 0, 0, 0));
    }
    // Second set values & blink min & max
    int animate_time = 10;
    bool led_on_off = false;
    while (animate_time > 0) {
        if (led_on_off) {
            ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, ZERO_CELSIUS_POSITION + temp_min, 5, 5, 5));
            ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, ZERO_CELSIUS_POSITION + temp_now, 5, 5, 5));
            ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, ZERO_CELSIUS_POSITION + temp_max, 5, 5, 5));
        } else {
            /* Turn off min & max but leave on temp_now */
            ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, ZERO_CELSIUS_POSITION + temp_min, 0, 0, 0));
            ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, ZERO_CELSIUS_POSITION + temp_max, 0, 0, 0));
        }
        /* Refresh the strip to send data */
        ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
        led_on_off = !led_on_off;
        vTaskDelay(pdMS_TO_TICKS(500));
        --animate_time;
    }
    // Third turn all off
    ESP_ERROR_CHECK(led_strip_clear(s_led_strip));
}
