/**
 * @file http_get.h
 * @author The Authors
 * @brief
 * @version 0.1
 * @date 2024-12-14
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "common.h"

#ifndef __HTTP_GET_H
#define __HTTP_GET_H

void http_get_task(void *pvParameters);
void http_get_init(void(*light_animate_and_set_cb_remote)(const int temp_min, const int temp_now, const int temp_max));

#endif // __HTTP_GET_H