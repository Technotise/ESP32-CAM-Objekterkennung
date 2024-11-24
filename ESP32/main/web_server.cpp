#include "web_server.h"
#include "camera_utils.h"
#include "esp_log.h"
#include "sensor.h"
#include "model_load.h"

httpd_handle_t server = nullptr;
esp_err_t run_model_handler(httpd_req_t* req);
bool video_running = false;
extern const char* mount_point;

esp_err_t static_file_handler(httpd_req_t* req) {
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s%s", mount_point, req->uri);

    ESP_LOGI("STATIC_FILE", "Requested file: %s", filepath);
    ESP_LOGI("DEBUG", "Filepath: %s", filepath);

    // Öffnen der Datei
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        ESP_LOGW("STATIC_FILE", "File not found: %s", filepath);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
        return ESP_FAIL;
    }

    // MIME-Type basierend auf der Dateiendung setzen
    if (strstr(req->uri, ".css")) {
        httpd_resp_set_type(req, "text/css");
    } else if (strstr(req->uri, ".js")) {
        httpd_resp_set_type(req, "application/javascript");
    } else if (strstr(req->uri, ".jpg") || strstr(req->uri, ".jpeg")) {
        httpd_resp_set_type(req, "image/jpeg");
    } else if (strstr(req->uri, ".png")) {
        httpd_resp_set_type(req, "image/png");
    } else {
        httpd_resp_set_type(req, "text/plain");
    }

    // Datei senden
    char buffer[512];
    size_t read_bytes;
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        httpd_resp_send_chunk(req, buffer, read_bytes);
    }
    fclose(file);

    // Abschließen der Antwort
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

// Initialisiert den Webserver
void init_webserver() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 11;

    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE("WEB_SERVER", "Failed to start the HTTP server");
        return;
    }

    ESP_LOGI("WEB_SERVER", "HTTP server started");

    // Definition der URI-Handler
    const httpd_uri_t uris[] = {
        { .uri = "/", .method = HTTP_GET, .handler = root_handler, .user_ctx = nullptr },
        { .uri = "/toggle_video", .method = HTTP_GET, .handler = toggle_video_handler, .user_ctx = nullptr },
        { .uri = "/snapshot", .method = HTTP_GET, .handler = snapshot_handler, .user_ctx = nullptr },
        { .uri = "/photo", .method = HTTP_GET, .handler = snapshot_handler, .user_ctx = nullptr },
        { .uri = "/video", .method = HTTP_GET, .handler = video_stream_handler, .user_ctx = nullptr },
        { .uri = "/run_model", .method = HTTP_GET, .handler = run_model_handler, .user_ctx = nullptr },
        {.uri = "/camera_off.jpg", .method = HTTP_GET, .handler = static_file_handler, .user_ctx = NULL}, 
        {.uri = "/PHOTO.JPG", .method = HTTP_GET, .handler = static_file_handler, .user_ctx = NULL}, 
        {.uri = "/style.css", .method = HTTP_GET, .handler = static_file_handler, .user_ctx = NULL},
        {.uri = "/script.js", .method = HTTP_GET, .handler = static_file_handler, .user_ctx = NULL},
        {.uri = "/result.txt", .method = HTTP_GET, .handler = static_file_handler, .user_ctx = NULL}, 
    };

    // URI-Handler registrieren
    for (size_t i = 0; i < sizeof(uris) / sizeof(uris[0]); i++) {
        if (httpd_register_uri_handler(server, &uris[i]) != ESP_OK) {
            ESP_LOGW("WEB_SERVER", "Failed to register URI: %s", uris[i].uri);
        } else {
            ESP_LOGI("WEB_SERVER", "Registered URI: %s", uris[i].uri);
        }
    }
}

// Toggle-Video
esp_err_t toggle_video_handler(httpd_req_t* req) {
    if (video_running) {
        stop_camera(); // Speicher freigeben
        video_running = false;
        httpd_resp_send(req, "Video gestoppt.", HTTPD_RESP_USE_STRLEN);
    } else {
        if (start_camera(PIXFORMAT_JPEG) == ESP_OK) {
            video_running = true;
            httpd_resp_send(req, "Video gestartet.", HTTPD_RESP_USE_STRLEN);
        } else {
            httpd_resp_send(req, "Fehler beim Starten des Videos.", HTTPD_RESP_USE_STRLEN);
        }
    }
    return ESP_OK;
}


//PHOTO
esp_err_t snapshot_handler(httpd_req_t* req) {
    // Kamera mit JPEG-Format starten
    if (start_camera(PIXFORMAT_JPEG) != ESP_OK) {
        httpd_resp_send(req, "Fehler beim Starten der Kamera im JPEG-Format.", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    // Foto aufnehmen (JPEG)
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb || fb->format != PIXFORMAT_JPEG) {
        stop_camera();
        httpd_resp_send(req, "Fehler beim Aufnehmen des JPEG-Bildes.", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    // JPEG speichern
    char jpeg_path[128];
    snprintf(jpeg_path, sizeof(jpeg_path), "%s/photo.jpg", mount_point);
    FILE* jpeg_file = fopen(jpeg_path, "wb");
    if (!jpeg_file) {
        esp_camera_fb_return(fb);
        stop_camera();
        httpd_resp_send(req, "Fehler beim Speichern des JPEG-Bildes.", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }
    fwrite(fb->buf, 1, fb->len, jpeg_file);
    fclose(jpeg_file);
    ESP_LOGI("CAMERA", "JPEG-Bild gespeichert: %s", jpeg_path);
    esp_camera_fb_return(fb);
    stop_camera();

    // Kamera neu starten mit YUV422-Format
    if (start_camera(PIXFORMAT_YUV422) != ESP_OK) {
        httpd_resp_send(req, "Fehler beim Starten der Kamera im YUV422-Format.", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    // Foto aufnehmen (YUV422)
    fb = esp_camera_fb_get();
    if (!fb || fb->format != PIXFORMAT_YUV422) {
        stop_camera();
        httpd_resp_send(req, "Fehler beim Aufnehmen des YUV422-Bildes.", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    // YUV422 speichern
    char yuv_path[128];
    snprintf(yuv_path, sizeof(yuv_path), "%s/photo.yuv", mount_point);
    FILE* yuv_file = fopen(yuv_path, "wb");
    if (!yuv_file) {
        esp_camera_fb_return(fb);
        stop_camera();
        httpd_resp_send(req, "Fehler beim Speichern des YUV422-Bildes.", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }
    fwrite(fb->buf, 1, fb->len, yuv_file);
    fclose(yuv_file);
    ESP_LOGI("CAMERA", "YUV422-Bild gespeichert: %s", yuv_path);

    // Speicher freigeben und Kamera stoppen
    esp_camera_fb_return(fb);
    stop_camera();

    // Erfolgsantwort an den Browser
    const char* resp = "Foto aufgenommen.";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

//STREAM
esp_err_t video_stream_handler(httpd_req_t* req) {
    if (!video_running) {
        httpd_resp_send(req, "Video ist nicht aktiv. Bitte aktivieren Sie das Video zuerst.", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    if (!is_camera_initialized) {
        if (start_camera(PIXFORMAT_JPEG) != ESP_OK) {
            httpd_resp_send(req, "Fehler bei der Kamera-Initialisierung.", HTTPD_RESP_USE_STRLEN);
            return ESP_FAIL;
        }
    }

    // Hole ein einzelnes Frame von der Kamera
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE("VIDEO_STREAM", "Fehler beim Abrufen eines Frames");
        httpd_resp_send(req, "Fehler beim Abrufen eines Frames.", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    // Setze den Content-Type als JPEG-Bild
    httpd_resp_set_type(req, "image/jpeg");
    esp_err_t res = httpd_resp_send(req, (const char*)fb->buf, fb->len);

    // Frame zurückgeben
    esp_camera_fb_return(fb);

    // Fehlerbehandlung
    if (res != ESP_OK) {
        ESP_LOGE("VIDEO_STREAM", "Fehler beim Senden des Frames.");
        return res;
    }

    return ESP_OK;
}

//KI
void classify_image_task(void* params) {
    if (classify_image()) {
        ESP_LOGI("KI", "Modell-Eingabe-Daten erfolgreich vorbereitet");
    } else {
        ESP_LOGE("KI", "Modell-Eingabe-Daten fehlgeschlagen");
    }
    run_model();

    vTaskDelete(NULL);
}

esp_err_t run_model_handler(httpd_req_t* req) {
    ESP_LOGI("Handler", "Modell-Klassifizierung wird asynchron gestartet");

    // Task erstellen
    if (xTaskCreatePinnedToCore(classify_image_task, "classify_task", 16 * 1024, NULL, 5, NULL, 1) != pdPASS) {
        ESP_LOGE("Handler", "Fehler beim Erstellen des Modell-Eingabe-Daten-Tasks");
        const char* response = "Task-Erstellung fehlgeschlagen";
        httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    const char* resp = "Klassifikation beendet.";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Webseite
esp_err_t root_handler(httpd_req_t* req) {
    const char* resp = R"rawliteral(
    <!DOCTYPE html>
    <html>
        <head>
            <title>ESP32-CAM Steuerung</title>
            <link rel="stylesheet" href="/style.css">
            <script src="/script.js"></script>
        </head>
        <body>
            <h1>ESP32-CAM Steuerung</h1>
            <table>
            <tr>
            <td><button onclick="toggleVideo()">Stream Start/Stop</button></td>
            <td><button onclick="fetch('/snapshot').then(response => response.text()).then(alert)">Take Picture</button></td>
            <td><button onclick="fetch('/run_model').then(response => response.text()).then(alert)">Check Picture</button></td>
            </tr>
            <tr>
                <td><h2>Stream</h2></td>
                <td><h2>Letztes Foto</h2></td>
                <td><h2>Letztes Ergebnis<h2></td>
            </tr>
            <tr>
                <td><img id="stream" src="/camera_off.jpg" alt="Stream" width="240" height="240"></td>
                <td><img id="latestPhoto" src="/PHOTO.JPG" width="240" height="240"></td>
                <td><pre id="result">Lade Ergebnisse...</pre></td>
            </tr>
            </table>
        </body>
    </html>
    )rawliteral";

    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
