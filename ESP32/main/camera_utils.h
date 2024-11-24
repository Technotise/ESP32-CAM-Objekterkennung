#ifndef CAMERA_UTILS_H
#define CAMERA_UTILS_H

#include "esp_err.h"

extern bool is_camera_initialized;

esp_err_t start_camera(pixformat_t pixel_format);
void stop_camera();

#endif // CAMERA_UTILS_H
