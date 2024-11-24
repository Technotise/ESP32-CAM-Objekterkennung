#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "esp_log.h"
#include <dirent.h>

static const char* TAG = "SD_CARD";

void mount_sd_card(const char* mount_point) {
    // Standardkonfiguration für SDMMC-Host
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // Mount-Konfiguration für das FAT-Dateisystem
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 10
    };

    sdmmc_card_t* card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SD-Karte konnte nicht gemountet werden: %s", esp_err_to_name(ret));
        return;
    }

    // Log Informationen über die SD-Karte
    sdmmc_card_print_info(stdout, card);
    ESP_LOGI(TAG, "SD-Karte erfolgreich gemountet bei: %s", mount_point);
}

void list_files(const char* mount_point) {
    DIR *dir = opendir(mount_point);
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open directory: %s", mount_point);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            ESP_LOGI(TAG, "File: %s", entry->d_name);
        } else if (entry->d_type == DT_DIR) {
            ESP_LOGI(TAG, "Directory: %s", entry->d_name);
        } else {
            ESP_LOGI(TAG, "Other: %s", entry->d_name);
        }
    }
    closedir(dir);
}
