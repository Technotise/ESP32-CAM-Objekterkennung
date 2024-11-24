// script.js
let streamInterval = null;

function startStream() {
    const img = document.getElementById("stream");
    if (streamInterval) clearInterval(streamInterval);

    streamInterval = setInterval(() => {
        img.src = `/video?time=${new Date().getTime()}`;
    }, 100);
}

function stopStream() {
    const img = document.getElementById("stream");
    if (streamInterval) clearInterval(streamInterval);
    streamInterval = null;
    img.src = "/camera_off.jpg";
}

function toggleVideo() {
    fetch('/toggle_video')
        .then(response => response.text())
        .then(data => {
            alert(data);
            if (data.includes("gestartet")) {
                startStream();
            } else {
                stopStream();
            }
        });
}

function takePicture() {
    fetch('/snapshot')
        .then(response => {
            if (response.ok) {
                // Erfolgreiche Aufnahme, Seite neu laden
                window.location.reload();
            } else {
                // Fehler vom Server anzeigen
                response.text().then(text => alert('Fehler: ' + text));
            }
        })
        .catch(err => {
            // Netzwerkfehler oder andere Probleme
            alert('Fehler bei der Verbindung: ' + err);
        });
}

function runModel() {
    fetch('/run_model')
        .then(response => {
            if (response.ok) {
                // Erfolgreiche Aufnahme, Seite neu laden
                window.location.reload();
            } else {
                // Fehler vom Server anzeigen
                response.text().then(text => alert('Fehler: ' + text));
            }
        })
        .catch(err => {
            // Netzwerkfehler oder andere Probleme
            alert('Fehler bei der Verbindung: ' + err);
        });
}

function fetchResult() {
    fetch('/result.txt')
        .then(response => {
            if (response.ok) {
                return response.text();
            } else {
                throw new Error('Datei konnte nicht geladen werden.');
            }
        })
        .then(data => {
            document.getElementById('result').textContent = data;
        })
        .catch(err => {
            document.getElementById('result').textContent = 'Fehler: ' + err.message;
        });
}

// Automatisches Laden des Ergebnisses beim Öffnen der Seite
window.onload = fetchResult;
