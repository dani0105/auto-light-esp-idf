#include "wifi.h"

static const char *TAG_WIFI = WIFI_TAG;
static bool initialized = false;
static EventGroupHandle_t wifi_event_group;

const char *SSID = WIFI_SSID;
const char *PASSWORD = WIFI_PASSWORD;
const short MAX_RETRY = WIFI_MAX_RETRY;

bool connected = false;
bool close_action = false;
short retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED && !close_action)
    {
        ESP_LOGI(TAG_WIFI, "connect to the AP fail");
        connected = false;

        if (retry_num < MAX_RETRY)
        {
            ESP_LOGI(TAG_WIFI, "retry to connect to the AP");
            esp_wifi_connect();
            retry_num++;
            return;
        }
        xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
        ESP_LOGI(TAG_WIFI, "Failed to connect to SSID:%s, password:%s", SSID, PASSWORD);
        return;
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG_WIFI, "connected to ap SSID:%s password:%s", SSID, PASSWORD);
        ESP_LOGI(TAG_WIFI, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        connected = true;
        return;
    }
}

void stop_wifi()
{
    close_action = true;
    ESP_LOGI(TAG_WIFI, "Closing WiFi");
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(esp_wifi_stop());
}

void start_wifi()
{
    close_action = false;
    connected = false;
    ESP_LOGI(TAG_WIFI, "Starting WiFi");
    ESP_ERROR_CHECK(esp_wifi_start());
}

bool is_wifi_connected()
{
    return connected;
}

void init_wifi(void)
{
    ESP_LOGI(TAG_WIFI, "initialise_wifi started.");
    wifi_event_group = xEventGroupCreate();
    esp_log_level_set("wifi", ESP_LOG_WARN);

    if (initialized)
    {
        return;
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        }};
    strcpy((char *)wifi_config.sta.ssid, SSID);
    strcpy((char *)wifi_config.sta.password, PASSWORD);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    initialized = true;
    ESP_LOGI(TAG_WIFI, "initialise_wifi finished.");

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        connected = true;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        connected = false;
    }
}