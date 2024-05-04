#include "sntp_control.h"

static const char *TAG_SNTP = SNTP_CONTROL_TAG;
static EventGroupHandle_t sntp_event_group;

bool sincronized = false;

struct tm get_current_time()
{
    time_t now = 0;
    struct tm timeinfo = {0};

    time(&now);
    localtime_r(&now, &timeinfo);
    return timeinfo;
}

int get_current_hour()
{
    struct tm time_info = get_current_time();
    return time_info.tm_hour;
}

int get_current_minute()
{
    struct tm time_info = get_current_time();
    return time_info.tm_min;
}

void stop_sntp()
{
    ESP_LOGI(TAG_SNTP, "Closing Sntp");
    esp_sntp_stop();
}

void start_sntp()
{
    ESP_LOGI(TAG_SNTP, "Starting Sntp");
    esp_sntp_restart();
}

bool is_sntp_sincronized()
{
    return sincronized;
}

void callback(struct timeval *tv)
{
    ESP_LOGI(TAG_SNTP, "Sincronized SNTP");
    struct tm time_info = get_current_time();
    ESP_LOGI(TAG_SNTP, "Sincronized SNTP");
    sincronized = true;
    ESP_LOGI(TAG_SNTP, "Current time: %02d:%02d:%02d\n", time_info.tm_hour, time_info.tm_min, time_info.tm_sec);
    xEventGroupSetBits(sntp_event_group, SNTP_SYNC_BIT);
}

void init_sntp(void)
{
    ESP_LOGI(TAG_SNTP, "Initializing SNTP");
    sntp_event_group = xEventGroupCreate();
    setenv("TZ", SNTP_CONTROL_TIMEZONE, 1); // Establece la zona horaria (UTC por ejemplo)
    tzset();
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, SNTP_CONTROL_SERVER);
    sntp_set_time_sync_notification_cb(callback);
    esp_sntp_init();

    EventBits_t bits = xEventGroupWaitBits(sntp_event_group,
                                           SNTP_SYNC_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & SNTP_SYNC_BIT)
    {
        sincronized = true;
    }
    else
    {
        sincronized = false;
    }
}