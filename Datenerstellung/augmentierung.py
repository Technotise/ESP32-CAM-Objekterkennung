# -*- coding: utf-8 -*-
import os
from PIL import Image, ImageEnhance
import random

# Input- und Output-Ordner
INPUT_DIR = "G:\dataset2\c"  # Ordner mit den Originalbildern
OUTPUT_DIR = "G:\dataset2\c2"  # Ordner für die augmentierten Bilder
NUM_AUGMENTATIONS = 10  # Anzahl der Augmentierungen pro Bild

# Sicherstellen, dass der Ausgabeordner existiert
os.makedirs(OUTPUT_DIR, exist_ok=True)

def augment_image(image):
    """Fuehrt eine zufaellige Augmentierung auf einem Bild durch."""
    # Zufällige Transformationen anwenden
    if random.random() > 0.5:
        image = image.rotate(random.uniform(-15, 15))  # Drehung um -15° bis 15°
    if random.random() > 0.5:
        image = image.transpose(Image.FLIP_LEFT_RIGHT)  # Horizontale Spiegelung
    if random.random() > 0.5:
        enhancer = ImageEnhance.Brightness(image)
        image = enhancer.enhance(random.uniform(0.8, 1.2))  # Helligkeit
    if random.random() > 0.5:
        enhancer = ImageEnhance.Contrast(image)
        image = enhancer.enhance(random.uniform(0.8, 1.2))  # Kontrast
    if random.random() > 0.5:
        enhancer = ImageEnhance.Color(image)
        image = enhancer.enhance(random.uniform(0.8, 1.2))  # Farbsättigung

    # Zufälliges Zuschneiden und Reskalieren
    if random.random() > 0.5:
        width, height = image.size
        left = random.uniform(0, width * 0.1)
        top = random.uniform(0, height * 0.1)
        right = width - random.uniform(0, width * 0.1)
        bottom = height - random.uniform(0, height * 0.1)
        image = image.crop((left, top, right, bottom))
        image = image.resize((width, height))  # Zurück auf Originalgröße skalieren

    return image

# Alle Bilder aus dem Input-Ordner augmentieren
for filename in os.listdir(INPUT_DIR):
    if filename.lower().endswith((".png", ".jpg", ".jpeg")):
        # Originalbild laden
        img_path = os.path.join(INPUT_DIR, filename)
        image = Image.open(img_path)

        # Augmentierungen durchführen
        base_name, ext = os.path.splitext(filename)
        for i in range(NUM_AUGMENTATIONS):
            augmented_image = augment_image(image)
            augmented_filename = f"{base_name}_aug_{i}{ext}"
            augmented_image.save(os.path.join(OUTPUT_DIR, augmented_filename))

print(f"Augmentierung abgeschlossen. Bilder wurden in {OUTPUT_DIR} gespeichert.")
