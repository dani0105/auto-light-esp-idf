#define __SNTP_CONTROL_H__

#include "esp_sntp.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif_sntp.h"

#define SNTP_CONTROL_TAG "Sntp"

#define SNTP_CONTROL_TIMEZONE "CST6"
#define SNTP_CONTROL_SERVER "pool.ntp.org"

#define SNTP_SYNC_BIT BIT0

struct tm get_current_time();
void init_sntp(void);
void start_sntp();
void stop_sntp();
bool is_sntp_sincronized();


int get_current_hour();
int get_current_minute();