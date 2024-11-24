#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "esp_http_server.h"
#include "esp_camera.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_system.h"

extern const char* mount_point;
extern const char* model_name;

// Webserver-Funktionen
void init_webserver();

// Handler-Funktionen
esp_err_t root_handler(httpd_req_t* req);
esp_err_t toggle_video_handler(httpd_req_t* req);
esp_err_t snapshot_handler(httpd_req_t* req);
esp_err_t video_stream_handler(httpd_req_t* req);

#endif // WEB_SERVER_H
