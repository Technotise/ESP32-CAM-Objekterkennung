#include "wifi_manager.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "wifi_credentials.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include <cstring>

// Funktion zur Behandlung des IP-Ereignisses
static void got_ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info((esp_netif_t*)arg, &ip_info);
    char ipStr[64];
    esp_ip4addr_ntoa(&ip_info.ip, ipStr, sizeof(ipStr));
    ESP_LOGI("Wifi", "IP-Adresse erhalten: %s", ipStr);
}

// Funktion zur Behandlung eines Verbindungsverlusts
static void wifi_disconnected_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    ESP_LOGI("Wifi","WLAN-Verbindung verloren, versuche erneut...");
    esp_wifi_connect();
}

// Initialisiert die Wi-Fi-Verbindung
void init_wifi() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t* netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {};
    strcpy((char*)wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char*)wifi_config.sta.password, WIFI_PASSWORD);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI("Wifi","WLAN wird verbunden...");
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &got_ip_event_handler, netif));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_disconnected_handler, nullptr));
    ESP_ERROR_CHECK(esp_wifi_connect());
}
