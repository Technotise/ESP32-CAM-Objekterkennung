# ESP32-CAM-Objekterkennung
Dieses Projekt verwendet das ESP32-CAM Modul zur Implementierung von Objekterkennung mithilfe von Edge-AI-Technologien.
Ziel ist es, eine energieeffiziente, datenschutzfreundliche und latenzarme Lösung zu schaffen.

## Features

- Einbindung der SD-Karte
- Einbindung der ESP32-Kamera
- Verbindung zu WiFi
- Erstellung eines Webservers (Nutzungsoberfläche für das gesamte Projekt)
- Schnittstelle zu Edge Impulse Exportdateien (Konvertierung von Bildern in das richtige Format, Ausführung des Modells)
- Objekterkennung

## Anforderungen

- ESP32-CAM + MB
- Edge Impulse Plattform (zur Modellerstellung und -export)
- Micro-USB-Kabel oder USB-C-Kabel
- Entwicklungsumgebung:
  - [VSCode](https://code.visualstudio.com/) mit ESP-IDF-Erweiterung
  - ESP-IDF Framework

## Projektaufbau

1. **Modellerstellung**: Das Modell wurde mithilfe von Edge Impulse erstellt und optimiert. Es basiert auf TensorFlow. Das Dataset wurde direkt mit der Kamera des ESP32s erstellt und mit einer Augmentierung mithilfe von Visual Studio und Python erweitert. Aufgrund von Lizenzgründen sind die exportierten Dateien nicht in diesem Repository enthalten. Bitte exportieren Sie Ihr Modell direkt von der [Edge Impulse Plattform](https://www.edgeimpulse.com/) -> Mein Testset von EDGE IMPULSE kann hier eingesehen werden [Edge Impulse Plattform](https://studio.edgeimpulse.com/public/564420/latest).

4. **ESP32-CAM Konfiguration**:

   - SD-Karte eingebunden
   - Kamera-Treiber integriert

5. **Code-Struktur**:

   - `ESP32/`: Dieser Ordner muss in VSCode mit ESP-IDF geöffnet werden.
   - `SD-Karte/`: Inhalte dieses Ordners müssen auf die SD-Karte kopiert werden, die mit dem ESP32-CAM verwendet wird.

## Installation

1. Klone dieses Repository:
   ```bash
   git clone https://github.com/Technotise/esp32-cam-projekt.git
   ```
2. Öffne das Projekt in VSCode mit der ESP-IDF-Erweiterung.
3. Konfiguriere die Umgebung für und in ESP-IDF.
4. Baue und flashe das Projekt:
   ```bash
   idf.py build
   idf.py flash
   ```
5. Überprüfe die serielle Ausgabe:
   ```bash
   idf.py monitor
   ```

## Hinweise

- **Lizenz**: Die Dateien, die aus Edge Impulse exportiert wurden, sind aus Lizenzgründen nicht in diesem Repository enthalten. Bitte exportieren Sie sie selbst.
- **Speicheroptimierung**: Das Projekt verwendet ein quantisiertes Modell, um den begrenzten Speicher des ESP32-CAM optimal zu nutzen.

## Lizenz

Dieses Projekt steht unter der [Apache 2.0 Lizenz](LICENSE).

## Autor

[Moritz Brühl](https://github.com/Technotise)

---

Bei Fragen oder Anregungen, eröffnen Sie bitte ein Issue oder kontaktieren Sie mich über GitHub.

