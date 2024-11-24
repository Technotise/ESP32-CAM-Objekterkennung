#include "wifi_manager.h"
#include "web_server.h"
#include "sd_card_mount.h"
#include "camera_utils.h"
#include "model_load.h"

const char* mount_point = "/sdcard";

extern "C" void app_main() {
    mount_sd_card(mount_point);
    list_files(mount_point);
    init_wifi();
    init_webserver();
}
