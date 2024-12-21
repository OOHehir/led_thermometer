/**
 * @file common.h
 * @author The Authors
 * @brief
 * @version 0.1
 * @date 2024-12-18
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#define CONFIG_ONBOARD_LED_GPIO   8     // ESP32 C3 Super mini https://www.espboards.dev/esp32/esp32-c3-super-mini/

typedef void (*light_animate_and_set_cb_t)(const int temp_min, const int temp_now, const int temp_max);

#define OPENWEATHERMAP_LOCATION "Dublin,IE"
