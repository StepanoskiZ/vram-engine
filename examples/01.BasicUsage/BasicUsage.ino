/*
  =========================================================================
  S-OS V-RAM Engine™ — Basic Usage Example (CE Edition)
  =========================================================================
  Demonstration of basic Virtual Memory usage on ESP32:
  1. Starting the V-RAM Engine
  2. Writing and reading primitive types (float, uint32_t) using vram.put() / vram.get()
  3. Storing and retrieving complex C++ Structures at virtual addresses
  4. Displaying compression statistics

  Brand: Syntetika Universe
  Author: Zoran Stepanoski
  Website: https://zoranstepanoski-prof-website.fly.dev/
  =========================================================================
*/

#include <Arduino.h>
#include <LittleFS.h>
#include <VirtualMemoryEngine.h>

// Initialize V-RAM Engine (16KB blocks, 128KB RAM pool limit)
// Default CE limit is 512 KB (524288 bytes)
VirtualMemoryEngine vram(16384, 131072, 32);

// Complex C++ structure for demonstration
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
    Serial.println("⚡ Syntetika Universe — V-RAM Engine Basic Usage Demo (CE)");
    Serial.println("========================================================\n");


    // 1. ALWAYS FORMAT LITTLEFS FIRST!
    esp_log_level_set("*", ESP_LOG_NONE); 
    Serial.println("Formatting Flash memory (LittleFS Format)...");
    LittleFS.begin(true);
    LittleFS.format();
    esp_log_level_set("*", ESP_LOG_INFO); 
    Serial.println("✓ Flash memory is completely clean and ready!");

    // 2. THEN START THE V-RAM ENGINE!
    if (!vram.begin()) {
        Serial.println("❌ CRITICAL: V-RAM Engine failed to start!");
        return;
    }

    // ====================================================================
    // EXAMPLE 1: Primitive data types (float, int, uint32_t)
    // ====================================================================
    Serial.println("\n▶ EXAMPLE 1: Writing primitive types to V-RAM...");

    uint32_t myAddr = 10000; // Virtual address at 10 KB mark
    float myTemp = 36.6f;
    uint32_t myCounter = 12345678;

    // Write values to the specified virtual address
    vram.put<float>(myAddr, myTemp);
    vram.put<uint32_t>(myAddr + sizeof(float), myCounter);

    // Read values back from the same virtual address
    float readTemp = vram.get<float>(myAddr);
    uint32_t readCounter = vram.get<uint32_t>(myAddr + sizeof(float));

    Serial.printf("   • [Address %u] Read Temperature: %.1f °C\n", myAddr, readTemp);
    Serial.printf("   • [Address %u] Read Counter:     %u\n", myAddr + sizeof(float), readCounter);

    // ====================================================================
    // EXAMPLE 2: Complex C++ Structures (Structs)
    // ====================================================================
    Serial.println("\n▶ EXAMPLE 2: Storing complex C++ structure in V-RAM...");

    uint32_t structAddr = 250000; // Virtual address at 250 KB mark!

    TelemetryData sensor1;
    sensor1.timestamp = millis();
    sensor1.temperature = 25.8f;
    sensor1.humidity = 58.4f;
    strcpy(sensor1.deviceName, "Sensor_Node_01");
    sensor1.isStatusOk = true;

    // Write the entire structure to V-RAM in a single line!
    vram.put<TelemetryData>(structAddr, sensor1);
    Serial.printf("   ✓ Structure '%s' successfully saved to virtual address %u!\n", sensor1.deviceName, structAddr);

    // Read the entire structure from V-RAM in a single line!
    TelemetryData readSensor = vram.get<TelemetryData>(structAddr);

    Serial.println("   ✓ Retrieved data from V-RAM:");
    Serial.printf("      • Device Name: %s\n", readSensor.deviceName);
    Serial.printf("      • Time (ms):    %u\n", readSensor.timestamp);
    Serial.printf("      • Temperature:  %.1f °C\n", readSensor.temperature);
    Serial.printf("      • Humidity:     %.1f %%\n", readSensor.humidity);
    Serial.printf("      • Status:       %s\n", readSensor.isStatusOk ? "OK" : "ERROR");

    // ====================================================================
    // EXAMPLE 3: V-RAM Compression statistics
    // ====================================================================
    Serial.println("\n▶ EXAMPLE 3: V-RAM Compression Statistics:");
    vram.flush(); // Force write last cached blocks to get exact flash usage
    size_t compressedBytes = vram.getCompressedSize();
    float ratio = vram.getCompressionRatio(300000); // For 300KB virtual footprint

    Serial.printf("   • Actually compressed in flash memory: %u bytes\n", compressedBytes);
    Serial.printf("   • Compression ratio: %.2fx\n", ratio);

    Serial.println("\n========================================================");
    Serial.println("🏁 BASIC USAGE DEMO SUCCESSFULLY COMPLETED!");
    Serial.println("========================================================\n");
}

void loop() {
    delay(1000);
}
