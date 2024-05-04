#define __WIFI_H__

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include <freertos/FreeRTOS.h>
#include <stdio.h>
#include <string.h>
#include "freertos/event_groups.h"

#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "Password"

#define WIFI_TAG "Wifi"

#define WIFI_MAX_RETRY 5

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

void init_wifi(void);
void stop_wifi(void);
void start_wifi(void);
bool is_wifi_connected();