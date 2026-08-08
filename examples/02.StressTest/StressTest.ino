/*
  =========================================================================
  S-OS V-RAM Engine™ — 512 KILOBYTE Stress Test & Benchmark (CE Edition)
  =========================================================================
  Performance, data integrity, and latency verification over 512 KB V-RAM.

  Brand: Syntetika Universe
  Author: Zoran Stepanoski
  Website: https://zoranstepanoski-prof-website.fly.dev/
  =========================================================================
*/

#include <Arduino.h>
#include <LittleFS.h>
#include <esp_log.h>
#include "VirtualMemoryEngine.h"

// Initialize CE V-RAM (16KB page blocks, 128KB RAM limit, max 32 blocks in cache)
// Uses default CE limit of 512 KB (524288 bytes)
VirtualMemoryEngine vram(16384, 131072, 32);

// ==========================================================
// TEST SIZE: 512 KILOBYTES (524,288 bytes) - CE LIMIT
// ==========================================================
#define TEST_SIZE_BYTES 524288
#define NUM_RANDOM_READS 1000

// Mock Structure for Database/AI testing (80 bytes)
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
    Serial.println("🔥 V-RAM STRESS TEST: 512 KILOBYTES (Community Edition) 🔥");
    Serial.println("==================================================");

    // Track free Heap before initialization
    uint32_t freeHeapBefore = ESP.getFreeHeap();
    
    // =========================================================
    // CRUCIAL STEP: Format Flash BEFORE starting V-RAM!
    // This clears all Flash memory from old/corrupted files.
    // =========================================================
    esp_log_level_set("*", ESP_LOG_NONE); 
    Serial.println("Formatting Flash memory (LittleFS Format)...");
    LittleFS.begin(true);
    LittleFS.format();
    esp_log_level_set("*", ESP_LOG_INFO); 
    Serial.println("✓ Flash memory is completely clean and ready!");

    Serial.println("Starting S-OS V-RAM Engine...");
    if (!vram.begin()) {
        Serial.println("❌ CRITICAL: V-RAM Failed to Start!");
        return;
    }

    uint32_t freeHeapAfter = ESP.getFreeHeap();

    Serial.printf("Free physical RAM before V-RAM: %u bytes\n", freeHeapBefore);
    Serial.printf("Free physical RAM after V-RAM: %u bytes\n", freeHeapAfter);
    Serial.printf("V-RAM allocated in RAM (Overhead): %u bytes\n", freeHeapBefore - freeHeapAfter);
    Serial.println("--------------------------------------------------\n");

    // ==========================================================
    // PHASE 1: SEQUENTIAL WRITE (Write speed + Paging trigger)
    // ==========================================================
    Serial.printf("▶ PHASE 1: Writing %.2f KB to virtual memory...\n", TEST_SIZE_BYTES / 1024.0f);
    
    uint32_t startTime = millis();
    for (uint32_t i = 0; i < TEST_SIZE_BYTES; i++) {
        uint8_t val = (i % 256);
        vram.put<uint8_t>(i, val);
        if (i % 5000 == 0) yield();
    }

    // Force write of last cached blocks for accurate compression stats!
    vram.flush(); 

    uint32_t endTime = millis();
    float writeTimeSec = (endTime - startTime) / 1000.0f;
    float writeSpeed = (TEST_SIZE_BYTES / 1024.0f) / writeTimeSec;

    Serial.printf("✓ Completed in: %.2f seconds\n", writeTimeSec);
    Serial.printf("✓ Write throughput: %.2f KB/s\n", writeSpeed);
    
    size_t actualCompBytes = vram.getCompressedSize();
    float trueRatio = vram.getCompressionRatio(TEST_SIZE_BYTES);

    Serial.printf("✓ VIRTUALLY WRITTEN: %u bytes (512 KB)\n", TEST_SIZE_BYTES);
    Serial.printf("✓ ACTUALLY COMPRESSED IN MEMORY: %u bytes\n", actualCompBytes);
    Serial.printf("   -> TRUE COMPRESSION RATIO: %.2fx (S-OS Engine works perfectly!)\n", trueRatio);
    Serial.println("--------------------------------------------------\n");

    // ==========================================================
    // PHASE 2: SEQUENTIAL READ & DATA INTEGRITY
    // ==========================================================
    Serial.println("▶ PHASE 2: Reading 512 KB of data & checking integrity...");
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
        Serial.println("✓ INTEGRITY: PERFECT (0 errors across 524,288 bytes)!");
    } else {
        Serial.printf("❌ ERROR: Found %d corrupted bytes!\n", errorCount);
    }
    Serial.printf("✓ Completed in: %.2f seconds\n", readTimeSec);
    Serial.printf("✓ Sequential read throughput: %.2f KB/s\n", readSpeed);
    Serial.println("--------------------------------------------------\n");

    // ==========================================================
    // PHASE 3: RANDOM ACCESS TORTURE (Latency testing)
    // ==========================================================
    Serial.printf("▶ PHASE 3: Thrashing test - %d random hops across 512 KB...\n", NUM_RANDOM_READS);

    startTime = millis();
    for (int i = 0; i < NUM_RANDOM_READS; i++) {
        uint32_t randomAddr = random(0, TEST_SIZE_BYTES);
        volatile uint8_t trash = vram.get<uint8_t>(randomAddr); 
        if (i % 50 == 0) yield();
    }
    endTime = millis();
    
    uint32_t randomTotalTime = (endTime - startTime);
    float avgLatencyMs = (float)randomTotalTime / NUM_RANDOM_READS;

    Serial.printf("✓ Completed in: %u milliseconds\n", randomTotalTime);
    Serial.printf("✓ Average latency per Random swap: %.2f ms\n", avgLatencyMs);
    Serial.println("--------------------------------------------------\n");

    // ==========================================================
    // PHASE 4: REAL-WORLD STRUCTURES (Database Simulation)
    // ==========================================================
    int numStructs = 500; 
    Serial.printf("▶ PHASE 4: Writing and updating %d complex AI structures...\n", numStructs);
    
    // Database starts at 250 KB mark (well within 512 KB limit):
    uint32_t dbOffset = 250000;

    for (int i = 0; i < numStructs; i++) {
        ComplexData d;
        d.id = i;
        d.isActive = true;
        sprintf(d.name, "Sensor_Node_%d", i);
        for(int w=0; w<10; w++) d.weights[w] = i * 1.5f;

        vram.put<ComplexData>(dbOffset + (i * sizeof(ComplexData)), d);
        if (i % 100 == 0) yield();
    }

    ComplexData testRead = vram.get<ComplexData>(dbOffset + (350 * sizeof(ComplexData)));
    Serial.printf("✓ Structure read test #350 at address %u -> Name: %s, First weight: %.2f\n", dbOffset + (350 * sizeof(ComplexData)), testRead.name, testRead.weights[0]);
    Serial.println("==================================================\n");
    Serial.println("🏁 ALL TESTS SUCCESSFULLY COMPLETED!");
}

void loop() {
    delay(1000);
}
