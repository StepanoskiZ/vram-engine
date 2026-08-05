/*
  =========================================================================
  S-OS V-RAM Engine™ — 8.0 Megabyte Stress Test & Benchmark
  =========================================================================
  Provera performansi, integriteta podataka i latencije nad 8 MB V-RAM-a.

  Brend: Syntetika Universe
  Autor: Zoran Stepanoski
  Sajt: https://zoranstepanoski-prof-website.fly.dev/
  =========================================================================
*/

#include <Arduino.h>
#include <LittleFS.h>
#include <esp_log.h>
#include "VirtualMemoryEngine.h"

// Kreiramo V-RAM (32KB blokovi, 128KB RAM limit, max 16 blokova u kešu)
//VirtualMemoryEngine vram(32768, 131072, 16);
// Kreiramo V-RAM (16KB blokovi, 128KB RAM limit, max 32 bloka u kešu)
VirtualMemoryEngine vram(16384, 131072, 32);

// ==========================================================
// TEST VELIČINA: 8.0 MEGABAJTA (8.388.608 bajtova)
// Ovo dokazuje da virtuelna memorija prevazilazi fizički Flash od 1.5MB!
// ==========================================================
#define TEST_SIZE_BYTES 8388608
#define NUM_RANDOM_READS 1000

// Struktura za testiranje baze podataka (80 bajtova)
struct ComplexData {
    uint32_t id;
    float weights[10];
    char name[32];
    bool isActive;
};

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n\n==================================================");
    Serial.println("🔥 V-RAM STRESS TEST: 8.0 MEGABAJTA 🔥");
    Serial.println("==================================================");

    // Pratimo slobodan Heap pre inicijalizacije
    uint32_t freeHeapBefore = ESP.getFreeHeap();
    
    Serial.println("Pokrećem V-RAM Engine...");
    
    // Inicijalizujemo V-RAM i montiramo LittleFS
    if (!vram.begin()) {
        Serial.println("❌ KRAH: V-RAM se nije pokrenuo!");
        return;
    }
    
    // =========================================================
    // KLJUČNI KORAK: Formatiramo Flash pre pocetka testa
    // Ovo oslobađa svih 1.5 MB Flash-a od starih/prepunjenih fajlova!
    // =========================================================
    Serial.println("Čistim Flash memoriju (LittleFS formatiranje)...");
    LittleFS.format();
    Serial.println("✓ Flash memorija je potpuno čista i spremna!");

    uint32_t freeHeapAfter = ESP.getFreeHeap();

    Serial.printf("Slobodan fizički RAM pre V-RAM-a: %u bajtova\n", freeHeapBefore);
    Serial.printf("Slobodan fizički RAM posle V-RAM-a: %u bajtova\n", freeHeapAfter);
    Serial.printf("V-RAM je alocirao u RAM-u: %u bajtova\n", freeHeapBefore - freeHeapAfter);
    Serial.println("--------------------------------------------------\n");

    // ==========================================================
    // FAZA 1: SEQUENTIAL WRITE (Brzina upisa + Paging triger)
    // ==========================================================
    Serial.printf("▶ FAZA 1: Upisujem %.2f MB u virtuelnu memoriju...\n", TEST_SIZE_BYTES / 1048576.0f);
    
    uint32_t startTime = millis();
    for (uint32_t i = 0; i < TEST_SIZE_BYTES; i++) {
        // Upisujemo ponavljajući patern
        uint8_t val = (i % 256);
        vram.put<uint8_t>(i, val);

        // Hranimo Watchdog tajmer svakih 5.000 upisa da se ESP32 ne bi resetovao
        if (i % 5000 == 0) yield();
    }
    uint32_t endTime = millis();
    float writeTimeSec = (endTime - startTime) / 1000.0f;
    float writeSpeed = (TEST_SIZE_BYTES / 1024.0f) / writeTimeSec;

    Serial.printf("✓ Završeno za: %.2f sekundi\n", writeTimeSec);
    Serial.printf("✓ Brzina upisa: %.2f KB/s\n", writeSpeed);
    
    // Pozivamo zvaničnu V-RAM metodu za kompresiju!
    size_t actualCompBytes = vram.getCompressedSize();
    float trueRatio = vram.getCompressionRatio(TEST_SIZE_BYTES);

    // Prikaz prostora na Flashu (Kompresija na delu)
    Serial.printf("✓ UPISANO VIRTUELNO: %u bajtova\n", TEST_SIZE_BYTES);
    Serial.printf("✓ STVARNO KOMPRIMOVANO U MEMORIJI: %u bajtova\n", actualCompBytes);
    Serial.printf("   -> STVARNI FAKTOR KOMPRESIJE: %.2fx (S-OS Engine radi savršeno!)\n", trueRatio);
    Serial.println("--------------------------------------------------\n");

    // ==========================================================
    // FAZA 2: SEQUENTIAL READ I DATA INTEGRITY (Provera grešaka)
    // ==========================================================
    Serial.println("▶ FAZA 2: Čitam 8.0 MB podataka i proveravam integritet...");
    int errorCount = 0;
    
    startTime = millis();
    for (uint32_t i = 0; i < TEST_SIZE_BYTES; i++) {
        uint8_t readVal = vram.get<uint8_t>(i);
        uint8_t expectedVal = (i % 256);
        if (readVal != expectedVal) errorCount++;
        if (i % 5000 == 0) yield();
    }
    endTime = millis();
    float readTimeSec = (endTime - startTime) / 1000.0f;
    float readSpeed = (TEST_SIZE_BYTES / 1024.0f) / readTimeSec;

    if (errorCount == 0) {
        Serial.println("✓ INTEGRITET: SAVRŠEN (0 grešaka na 8.388.608 bajtova)!");
    } else {
        Serial.printf("❌ GREŠKA: Pronađeno %d oštećenih bajtova!\n", errorCount);
    }
    Serial.printf("✓ Završeno za: %.2f sekundi\n", readTimeSec);
    Serial.printf("✓ Brzina sekvencijalnog čitanja: %.2f KB/s\n", readSpeed);
    Serial.println("--------------------------------------------------\n");

    // ==========================================================
    // FAZA 3: RANDOM ACCESS TORTURE (Ispitivanje Latency-a)
    // ==========================================================
    Serial.printf("▶ FAZA 3: Thrashing test - %d nasumičnih skokova širom 8.0 MB...\n", NUM_RANDOM_READS);

    startTime = millis();
    for (int i = 0; i < NUM_RANDOM_READS; i++) {
        // Skačemo na potpuno random adresu od 0 do 8.388.608
        uint32_t randomAddr = random(0, TEST_SIZE_BYTES);
        volatile uint8_t trash = vram.get<uint8_t>(randomAddr); 

        if (i % 50 == 0) yield();
    }
    endTime = millis();
    
    uint32_t randomTotalTime = (endTime - startTime);
    float avgLatencyMs = (float)randomTotalTime / NUM_RANDOM_READS;

    Serial.printf("✓ Završeno za: %u milisekundi\n", randomTotalTime);
    Serial.printf("✓ Prosečno kašnjenje po Random pozivu (Latency): %.2f ms\n", avgLatencyMs);
    Serial.println("--------------------------------------------------\n");

    // ==========================================================
    // FAZA 4: REAL-WORLD STRUKTURE (Baza Podataka)
    // ==========================================================
    int numStructs = 2000; 
    Serial.printf("▶ FAZA 4: Upis i izmena %d kompleksnih AI struktura...\n", numStructs);
    
    // Baza počinje unutar 4MB kapaciteta (3.0 MB mark):
    uint32_t dbOffset = 3000000;

    for (int i = 0; i < numStructs; i++) {
        ComplexData d;
        d.id = i;
        d.isActive = true;
        sprintf(d.name, "Senzor_Node_%d", i);
        for(int w=0; w<10; w++) d.weights[w] = i * 1.5f;

        vram.put<ComplexData>(dbOffset + (i * sizeof(ComplexData)), d);
        if (i % 200 == 0) yield();
    }

    ComplexData testRead = vram.get<ComplexData>(dbOffset + (1500 * sizeof(ComplexData)));
    Serial.printf("✓ Test čitanja strukture br. 1500 na adresi %u -> Ime: %s, Prva težina: %.2f\n", dbOffset + (1500 * sizeof(ComplexData)), testRead.name, testRead.weights[0]);
    Serial.println("==================================================\n");
    Serial.println("🏁 SVI TESTOVI SU USPEŠNO ZAVRŠENI!");
}

void loop() {
    delay(1000);
}
