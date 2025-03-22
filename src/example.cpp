/*
#include "Adafruit_TinyUSB.h"
#include "tusb.h"

// Zwei unabhängige MIDI-Schnittstellen
Adafruit_USBD_MIDI midi1;
Adafruit_USBD_MIDI midi2;

void setup() {
    Serial.begin(115200);

    // MIDI 1 initialisieren
    midi1.setStringDescriptor("MIDI Port A");
    midi1.begin();

    // MIDI 2 initialisieren
    midi2.setStringDescriptor("MIDI Port B");
    midi2.begin();

    // TinyUSB starten
    TinyUSBDevice.begin();
    Serial.println("MIDI-Interfaces bereit!");
}

void loop() {
    tud_task(); // USB Stack verwalten

    // MIDI-Daten von MIDI 1 empfangen und an Serial ausgeben
    uint8_t packet1[4];
    while (midi1.available()) {
        midi1.read(packet1, sizeof(packet1));
        Serial.printf("[MIDI 1] Note: %d, Velocity: %d\n", packet1[2], packet1[3]);
    }

    // MIDI-Daten von MIDI 2 empfangen und an Serial ausgeben
    uint8_t packet2[4];
    while (midi2.available()) {
        midi2.read(packet2, sizeof(packet2));
        Serial.printf("[MIDI 2] Note: %d, Velocity: %d\n", packet2[2], packet2[3]);
    }

    // Testweise MIDI-Noten senden
    sendTestNotes();
    delay(1000);
}

void sendTestNotes() {
    uint8_t noteOn1[4] = { 0x09, 0x90, 60, 127 };  // Note-On für MIDI 1 (C4)
    uint8_t noteOn2[4] = { 0x09, 0x90, 64, 127 };  // Note-On für MIDI 2 (E4)
    uint8_t noteOff1[4] = { 0x08, 0x80, 60, 0 };   // Note-Off für MIDI 1
    uint8_t noteOff2[4] = { 0x08, 0x80, 64, 0 };   // Note-Off für MIDI 2

    midi1.write(noteOn1, 4);
    midi2.write(noteOn2, 4);
    delay(500);
    midi1.write(noteOff1, 4);
    midi2.write(noteOff2, 4);
}
*/