#include "wifi.h"
#include "sntp_control.h"

#include "esp_sleep.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 3        /* Time ESP32 will go to sleep (in seconds) */

#define RELAY_GPIO GPIO_NUM_34
#define LED_GPIO GPIO_NUM_18

static const char *TAG_MAIN = "Main";

int start_hour = 7; // 7 AM
int end_hour = 15;  // 3 PM

void hibernate(int seconds)
{
    uint64_t timer = seconds * 1000000LL;
    ESP_LOGI(TAG_MAIN, "Entering deep sleep for %d seconds", seconds);
    esp_deep_sleep(timer);
}

int calculate_hours_left(int current, int target)
{
    if (current >= target)
    {
        return 24 - current + target;
    }
    else
    {
        return target - current;
    }
}

void init_gpio()
{
    ESP_LOGI(TAG_MAIN, "Init GPIO");
    gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(RELAY_GPIO, 1);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 1);

    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG_MAIN, "Testing Leds");
    gpio_set_level(RELAY_GPIO, 0);
    gpio_set_level(LED_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(200));
    gpio_set_level(LED_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(200));
    gpio_set_level(LED_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(200));
    gpio_set_level(LED_GPIO, 1);
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    init_gpio();
    init_wifi();
    if (!is_wifi_connected())
    {
        ESP_LOGI(TAG_MAIN, "Reboting on 10 seconds because is not connected to a wifi");
        vTaskDelay(pdMS_TO_TICKS(10000));
        esp_restart();
    }

    init_sntp();

    if (!is_sntp_sincronized())
    {
        ESP_LOGI(TAG_MAIN, "Reboting on 10 seconds because is not syncronized with the internet time");
        vTaskDelay(pdMS_TO_TICKS(10000));
        esp_restart();
    }

    stop_wifi(); // WiFi is not more require

    int current_hour = get_current_hour();

    stop_sntp(); // sntp is not more require

    if (current_hour < start_hour || current_hour >= end_hour)
    {
        int hibernation_time_seconds = (60 - get_current_minute()) * 60;
        hibernation_time_seconds += (calculate_hours_left(current_hour, start_hour) - 1) * 3600;
        hibernate(hibernation_time_seconds);
        return;
    }

    // turn on the light
    ESP_LOGI(TAG_MAIN, "Turning On Light");

    gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(RELAY_GPIO, 1);

    int suspension_time_seconds = (60 - get_current_minute()) * 60;
    suspension_time_seconds += (calculate_hours_left(current_hour, end_hour) - 1) * 3600;
    ESP_LOGI(TAG_MAIN, "Suspend for %d seconds", suspension_time_seconds);
    vTaskDelay(pdMS_TO_TICKS(suspension_time_seconds * 1000));
    ESP_LOGI(TAG_MAIN, "Reboting");
    esp_restart();
}