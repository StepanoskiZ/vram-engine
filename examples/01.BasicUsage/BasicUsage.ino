/*
  =========================================================================
  S-OS V-RAM Engine™ — Basic Usage Example
  =========================================================================
  Demonstracija osnovnog rada sa virtuelnom memorijom na ESP32:
  1. Pokretanje V-RAM Engine-a
  2. Upis i čitanje primitivnih tipova (float, uint32_t) sa vram.put() / vram.get()
  3. Čuvanje i čitanje kompleksnih C++ struktura (Struct) na virtuelnim adresama
  4. Prikaz statistike kompresije

  Brend: Syntetika Universe
  Autor: Zoran Stepanoski
  Sajt: https://zoranstepanoski-prof-website.fly.dev/
  =========================================================================
*/

#include <Arduino.h>
#include <LittleFS.h>
#include <VirtualMemoryEngine.h>

// Inicijalizacija V-RAM Engine-a (16KB blokovi, 128KB RAM pool limit)
VirtualMemoryEngine vram(16384, 131072, 32);

// Struktura za demonstraciju čuvanja kompleksnih podataka
struct TelemetryData {
    uint32_t timestamp;
    float temperature;
    float humidity;
    char deviceName[16];
    bool isStatusOk;
};

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n========================================================");
    Serial.println("⚡ Syntetika Universe — V-RAM Engine Basic Usage Demo");
    Serial.println("========================================================\n");

    // 1. Pokretanje V-RAM Engine-a
    if (!vram.begin()) {
        Serial.println("❌ KRAH: V-RAM Engine se nije uspešno pokrenuo!");
        return;
    }

    // Formatiramo Flash radi čistog početnog stanja u demo-u
    LittleFS.format();
    Serial.println("✓ Flash memorija je uspešno inicijalizovana i čista.");

    // ====================================================================
    // PRIMER 1: Rad sa bazičnim tipovima podataka (float, int, uint32_t)
    // ====================================================================
    Serial.println("\n▶ PRIMER 1: Upisivanje primitivnih tipova u V-RAM...");

    uint32_t myAddr = 10000; // Virtuelna adresa od 10 KB
    float myTemp = 36.6f;
    uint32_t myCounter = 12345678;

    // Upisujemo vrednosti na definisanu virtuelnu adresu
    vram.put<float>(myAddr, myTemp);
    vram.put<uint32_t>(myAddr + sizeof(float), myCounter);

    // Čitamo vrednosti sa iste virtuelne adrese
    float readTemp = vram.get<float>(myAddr);
    uint32_t readCounter = vram.get<uint32_t>(myAddr + sizeof(float));

    Serial.printf("   • [Adresa %u] Pročitana Temperatura: %.1f °C\n", myAddr, readTemp);
    Serial.printf("   • [Adresa %u] Pročitani Brojač:     %u\n", myAddr + sizeof(float), readCounter);

    // ====================================================================
    // PRIMER 2: Rad sa kompleksnim C++ strukturama (Struct)
    // ====================================================================
    Serial.println("\n▶ PRIMER 2: Čuvanje kompleksne strukture u V-RAM-u...");

    uint32_t structAddr = 250000; // Virtuelna adresa od 250 KB mark!

    TelemetryData sensor1;
    sensor1.timestamp = millis();
    sensor1.temperature = 25.8f;
    sensor1.humidity = 58.4f;
    strcpy(sensor1.deviceName, "Senzor_Node_01");
    sensor1.isStatusOk = true;

    // Upisujemo celu strukturu u V-RAM u jednoj jedinoj liniji!
    vram.put<TelemetryData>(structAddr, sensor1);
    Serial.printf("   ✓ Struktura '%s' uspešno sačuvana na virtuelnoj adresi %u!\n", sensor1.deviceName, structAddr);

    // Čitamo celu strukturu iz V-RAM-a u jednoj liniji!
    TelemetryData readSensor = vram.get<TelemetryData>(structAddr);

    Serial.println("   ✓ Pročitani podaci iz V-RAM-a:");
    Serial.printf("      • Ime Uređaja: %s\n", readSensor.deviceName);
    Serial.printf("      • Vreme (ms):   %u\n", readSensor.timestamp);
    Serial.printf("      • Temperatura:  %.1f °C\n", readSensor.temperature);
    Serial.printf("      • Vlažnost:     %.1f %%\n", readSensor.humidity);
    Serial.printf("      • Status:       %s\n", readSensor.isStatusOk ? "OK" : "GREŠKA");

    // ====================================================================
    // PRIMER 3: Provera statistike V-RAM kompresije
    // ====================================================================
    Serial.println("\n▶ PRIMER 3: Statistika V-RAM kompresije:");
    size_t compressedBytes = vram.getCompressedSize();
    float ratio = vram.getCompressionRatio(300000); // Za ukupno zauzetih 300KB prostora

    Serial.printf("   • Stvarno komprimovano u memoriji: %u bajtova\n", compressedBytes);
    Serial.printf("   • Faktor kompresije: %.2fx\n", ratio);

    Serial.println("\n========================================================");
    Serial.println("🏁 BASIC USAGE DEMO USPEŠNO ZAVRŠEN!");
    Serial.println("========================================================\n");
}

void loop() {
    delay(1000);
}
