#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/image/processing.hpp"
#include "sdkconfig.h"
#include "esp_idf_version.h"
#include "yuv.h"

static const char *TAG = "KIModel";
extern const char* mount_point;

//LOAD
bool classify_image() {
    // Datei öffnen
    ESP_LOGI(TAG, "Öffne Datei.");
    const char *image_name = "photo.yuv";
    char image_path[1024];
    snprintf(image_path, sizeof(image_path), "%s/%s", mount_point, image_name);
    FILE *file = fopen(image_path, "rb");
    if (!file) {
        ESP_LOGE(TAG, "Fehler beim Öffnen der YUV-Datei: %s", image_path);
        return false;
    }

    // Datei in YUV422 lesen
    ESP_LOGI(TAG, "Lese YUV422 Datei.");
    size_t pixel_count = 3 * 240 * 240;
    size_t yuv_size = pixel_count * 2;
    uint8_t *yuv422_image = (uint8_t *)heap_caps_malloc(yuv_size, MALLOC_CAP_SPIRAM);
    if (!yuv422_image) {
        ESP_LOGE(TAG, "Speicher für YUV422 konnte nicht allokiert werden.");
        fclose(file);
        return false;
    }
    fread(yuv422_image, 1, yuv_size, file);
    fclose(file);

    //YUV422 zu RGB888 konvertieren
    uint8_t *rgb888_image = (uint8_t *)heap_caps_malloc(240 * 240 * 3, MALLOC_CAP_SPIRAM);
    if (!rgb888_image) {
        ESP_LOGE(TAG, "Speicher für RGB888 konnte nicht allokiert werden.");
        heap_caps_free(yuv422_image);
        yuv422_image = NULL;
        return false;
    }

    ESP_LOGI(TAG, "Konvertiere YUV422 zu RGB888.");
    // Konvertiere YUV422 zu RGB888
    for (size_t i = 0, j = 0; i < yuv_size; i += 4, j += 6) {
        uint8_t y0 = yuv422_image[i];
        uint8_t u  = yuv422_image[i + 1];
        uint8_t y1 = yuv422_image[i + 2];
        uint8_t v  = yuv422_image[i + 3];

        yuv2rgb(y0, u, v, &rgb888_image[j], &rgb888_image[j + 1], &rgb888_image[j + 2]);
        yuv2rgb(y1, u, v, &rgb888_image[j + 3], &rgb888_image[j + 4], &rgb888_image[j + 5]);
    }

    heap_caps_free(yuv422_image);
    yuv422_image= NULL;

    uint8_t *resized_image = (uint8_t *)heap_caps_malloc(96 * 96 * 3, MALLOC_CAP_SPIRAM);
    if (!resized_image) {
        ESP_LOGE(TAG, "Speicher für resized_image konnte nicht allokiert werden.");
        heap_caps_free(rgb888_image);
        rgb888_image = NULL;
        return false;
    }

    ESP_LOGI(TAG, "Resize von 240x240 auf 96x96.");
    //Resize von 240x240 auf 96x96
    int res_resize = ei::image::processing::resize_image(
        rgb888_image,   // Eingangsbild
        240,            // Eingangsbreite
        240,            // Eingangshöhe
        resized_image,  // Ausgangspuffer
        96,             // Zielbreite
        96,             // Zielhöhe
        3               // Pixelgröße (RGB888 = 3)
    );

    ESP_LOGI(TAG, "Resize abgeschlossen mit Code: %d", res_resize);

    /*
    //Bug mit Resize, daher Speicher hier nicht leeren
    if (res_resize != 0) {
    ESP_LOGE(TAG, "Fehler beim Resizing: %d", res_resize);
    heap_caps_free(rgb888_image);
    heap_caps_free(resized_image);
    return false;
    }

    heap_caps_free(rgb888_image);
    rgb888_image = NULL;
    */

    ESP_LOGI(TAG, "raw_features.txt erstellen.");
    // Datei öffnen, um die Ergebnisse zu speichern
    const char *result_name = "raw_features.txt";
    char result_path[1024];
    snprintf(result_path, sizeof(result_path), "%s/%s", mount_point, result_name);
    FILE* result_file = fopen(result_path, "w");
    if (!result_file) {
        ESP_LOGE("RawFeatures", "Fehler beim Öffnen der Datei.");
        heap_caps_free(resized_image);
        resized_image = NULL;
        return false;
    } else {
        ESP_LOGI("RawFeatures", "Datei geöffnet.");
    }

    // Hier einfügen
    for (int row = 0; row < 96; ++row) {
        for (int col = 0; col < 96; ++col) {
            // Offset berechnen
            int offset = (row * 96 + col) * 3;

            // R, G, B-Werte auslesen
            uint8_t blue   = resized_image[offset];
            uint8_t green = resized_image[offset + 1];
            uint8_t red  = resized_image[offset + 2];

            // HEX-Wert in die Datei schreiben
            fprintf(result_file, "0x%02X%02X%02X", blue, green, red);

            // Komma und Leerzeichen hinzufügen, außer beim letzten Pixel
            if (!(row == 95 && col == 95)) {
                fprintf(result_file, ", ");
            }
        }
    }

    fclose(result_file);
    ESP_LOGI(TAG, "Raw Features erfolgreich in raw_features.txt geschrieben.");

    heap_caps_free(resized_image);
    resized_image = NULL;
    return true;
}

//RUN

float features_run[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = {0};    

// Funktion, um die Features zu setzen
void set_features(const float* input_features, size_t length) {
    if (length != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        ESP_LOGE(TAG,"Die Länge der Eingabedaten (%zu) stimmt nicht mit der erwarteten Größe (%d) überein.\n", 
               length, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
        return;
    }
    memcpy(features_run, input_features, length * sizeof(float));
}

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features_run + offset, length * sizeof(float));
    return 0;
}

void print_inference_result(ei_impulse_result_t result, FILE* output_file) {

    // Print how long it took to perform inference
    ESP_LOGI(TAG,"Timing: DSP %d ms, inference %d ms, anomaly %d ms\r\n",
            result.timing.dsp,
            result.timing.classification,
            result.timing.anomaly);
    if (output_file) {
    fprintf(output_file, "Timing: DSP %d ms, inference %d ms, anomaly %d ms\n",
            result.timing.dsp,
            result.timing.classification,
            result.timing.anomaly);
    }

    // Print the prediction results (object detection)
#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    ESP_LOGI(TAG,"Object detection bounding boxes:");
    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
        if (bb.value == 0) {
            continue;
        }
        ESP_LOGI(TAG,"  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                bb.label,
                bb.value,
                bb.x,
                bb.y,
                bb.width,
                bb.height);
        if (output_file) {
            fprintf(output_file, "  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\n",
                    bb.label,
                    bb.value,
                    bb.x,
                    bb.y,
                    bb.width,
                    bb.height);
        }
    }

    // Print the prediction results (classification)
#else
    ESP_LOGI(TAG,"Predictions:");
    if (output_file) {
        fprintf(output_file, "Predictions:\n");
    }
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        ESP_LOGI(TAG,"  %s: ", ei_classifier_inferencing_categories[i]);
        ESP_LOGI(TAG,"%.5f\r\n", result.classification[i].value);
        if (output_file) {
            fprintf(output_file, "  %s: %.5f\n", ei_classifier_inferencing_categories[i], result.classification[i].value);
        }
    }
#endif

    // Print anomaly result (if it exists)
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ESP_LOGI(TAG,"Anomaly prediction: %.3f\r\n", result.anomaly);
    if (output_file) {
        fprintf(output_file, "Anomaly prediction: %.3f\n", result.anomaly);
    }
#endif

}

// Einmalige Ausführung des Modells
extern "C" int run_model() {
    ESP_LOGI(TAG,"Model wird gestartet");

    // Datei raw_features.txt auslesen
    char file_path[1024];
    const char *feature_name = "raw_features.txt";
    snprintf(file_path, sizeof(file_path), "%s/%s", mount_point, feature_name);

    ESP_LOGI(TAG,"Features laden");
    // Features laden
    float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
    FILE *file = fopen(file_path, "r");
    if (!file) {
        ESP_LOGE(TAG,"Fehler beim Öffnen der Datei: %s", file_path);
        return 1;
    }

    ESP_LOGI(TAG,"Datei-Größe ermitteln");
    // Datei-Größe ermitteln
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    ESP_LOGI(TAG,"Speicher für den Dateiinhalt reservieren");
    // Speicher für den Dateiinhalt reservieren
    char *buffer = (char *)malloc(file_size + 1);
    if (!buffer) {
        ESP_LOGE(TAG,"Speicherallokierung fehlgeschlagen.");
        fclose(file);
        return 1;
    }

    ESP_LOGI(TAG,"Datei in den Puffer lesen");
    // Datei in den Puffer lesen
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0'; // Nullterminierung hinzufügen
    fclose(file);

    ESP_LOGI(TAG,"Hex-Werte aus dem Puffer in das Features-Array parsen");
    // Hex-Werte aus dem Puffer in das Features-Array parsen
    char *token = strtok(buffer, ",");
    size_t index = 0;
    while (token && index < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        features_run[index++] = (float)strtol(token, NULL, 16); // Hex-Wert in float umwandeln
        token = strtok(NULL, ",");
    }

    free(buffer); // Speicher freigeben

    ESP_LOGI(TAG,"Überprüfen, ob die richtige Anzahl an Features geladen wurde");
    // Überprüfen, ob die richtige Anzahl an Features geladen wurde
    if (index != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        ESP_LOGE(TAG,"Fehler: Erwartete %d Features, aber nur %zu geladen.",
               EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, index);
        return 1;
    }

    ESP_LOGI(TAG,"Signal vorbereiten");
    // Signal vorbereiten
    ei_impulse_result_t result = {nullptr};
    signal_t features_signal;
    features_signal.total_length = sizeof(features_run) / sizeof(features_run[0]);
    features_signal.get_data = &raw_feature_get_data;

    ESP_LOGI(TAG,"Klassifikator ausführen");
    // Klassifikator ausführen
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
    if (res != EI_IMPULSE_OK) {
        ESP_LOGE(TAG, "Klassifikator konnte nicht ausgeführt werden. Fehlercode: %d", res);
        return res;
    }

    ESP_LOGI(TAG,"Ergebnisse");

    // Datei result.txt schreiben
    char kiresult_path[1024];
    const char *kiresult_name = "result.txt";
    snprintf(kiresult_path, sizeof(kiresult_path), "%s/%s", mount_point, kiresult_name);

    ESP_LOGI(TAG, "Schreibe Ergebnisse");
    FILE *kiresult_file = fopen(kiresult_path, "w");
    if (!kiresult_file) {
        ESP_LOGE(TAG, "Fehler beim Öffnen der Datei: %s", kiresult_path);
        return 1;
    }

    // Ergebnis in Datei schreiben
    ESP_LOGI(TAG, "Ergebnisse in Datei schreiben...");
    print_inference_result(result, kiresult_file);

    fclose(kiresult_file);
    ESP_LOGI(TAG, "Ergebnisse erfolgreich gespeichert.");

    ESP_LOGI(TAG,"Klassifikator beendet");
    return 0;
}
