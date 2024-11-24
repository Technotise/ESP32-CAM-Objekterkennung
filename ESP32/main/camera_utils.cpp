#include "esp_camera.h"
#include "esp_log.h"
#include "driver/i2c.h"

static const char* TAG = "CAMERA_UTILS";
bool is_camera_initialized = false;

esp_err_t start_camera(pixformat_t pixel_format) {
    // Kamera deinitialisieren, falls bereits initialisiert
    if (is_camera_initialized) {
        ESP_LOGI(TAG, "Kamera wird neu gestartet, um das Format zu ändern.");
        esp_camera_deinit();
        is_camera_initialized = false;
    }

    // Kamera-Konfiguration
    camera_config_t config = {
        .pin_pwdn = 32,
        .pin_reset = -1,
        .pin_xclk = 0,
        .pin_sscb_sda = 26,
        .pin_sscb_scl = 27,
        .pin_d7 = 35,
        .pin_d6 = 34,
        .pin_d5 = 39,
        .pin_d4 = 36,
        .pin_d3 = 21,
        .pin_d2 = 19,
        .pin_d1 = 18,
        .pin_d0 = 5,
        .pin_vsync = 25,
        .pin_href = 23,
        .pin_pclk = 22,
        .xclk_freq_hz = 20000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
        .pixel_format = pixel_format, // Dynamisches Format
        .frame_size = FRAMESIZE_240X240,
        .jpeg_quality = 12,
        .fb_count = 2
    };

    // Kamera initialisieren
    esp_err_t err = esp_camera_init(&config);
    if (err == ESP_OK) {
        is_camera_initialized = true;
        ESP_LOGI(TAG, "Kamera erfolgreich mit Pixel-Format %d gestartet", pixel_format);
    } else {
        ESP_LOGE(TAG, "Fehler bei der Kamera-Initialisierung: %s", esp_err_to_name(err));
    }
    return err;
}

void stop_camera() {
    if (is_camera_initialized) {
        esp_camera_deinit();
        is_camera_initialized = false;
        ESP_LOGI("CAMERA_UTILS", "Kamera gestoppt");
    }
}
