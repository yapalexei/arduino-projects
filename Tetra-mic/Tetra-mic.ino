#include "driver/i2s.h"
#include <WiFi.h>

// I2S Configuration
#define SAMPLE_RATE 44100
#define SAMPLE_BITS 32
#define BUFFER_SIZE 1024

// I2S Port definitions
#define I2S_PORT_0 I2S_NUM_0
#define I2S_PORT_1 I2S_NUM_1

// I2S0 pins
#define I2S0_SCK_PIN 26
#define I2S0_WS_PIN 25
#define I2S0_SD_PIN 22

// I2S1 pins  
#define I2S1_SCK_PIN 14
#define I2S1_WS_PIN 15
#define I2S1_SD_PIN 32

// Data buffers
int32_t i2s0_buffer[BUFFER_SIZE];
int32_t i2s1_buffer[BUFFER_SIZE];

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Quad INMP441 Microphone Test");
  
  // Disable WiFi to reduce interference
  WiFi.mode(WIFI_OFF);
  
  // Configure I2S0
  i2s_config_t i2s0_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = BUFFER_SIZE / 2,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  
  i2s_pin_config_t i2s0_pins = {
    .bck_io_num = I2S0_SCK_PIN,
    .ws_io_num = I2S0_WS_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S0_SD_PIN
  };
  
  // Configure I2S1  
  i2s_config_t i2s1_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = BUFFER_SIZE / 2,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  
  i2s_pin_config_t i2s1_pins = {
    .bck_io_num = I2S1_SCK_PIN,
    .ws_io_num = I2S1_WS_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S1_SD_PIN
  };
  
  // Install and start I2S drivers
  esp_err_t result0 = i2s_driver_install(I2S_PORT_0, &i2s0_config, 0, NULL);
  esp_err_t result1 = i2s_driver_install(I2S_PORT_1, &i2s1_config, 0, NULL);
  
  if (result0 != ESP_OK) {
    Serial.printf("Error installing I2S0 driver: %d\n", result0);
    return;
  }
  
  if (result1 != ESP_OK) {
    Serial.printf("Error installing I2S1 driver: %d\n", result1);
    return;
  }
  
  // Set pin configurations
  i2s_set_pin(I2S_PORT_0, &i2s0_pins);
  i2s_set_pin(I2S_PORT_1, &i2s1_pins);
  
  // Start I2S
  i2s_start(I2S_PORT_0);
  i2s_start(I2S_PORT_1);
  
  Serial.println("I2S initialized successfully!");
  Serial.println("Microphone mapping:");
  Serial.println("Mic 1: I2S0 Left Channel");
  Serial.println("Mic 2: I2S0 Right Channel"); 
  Serial.println("Mic 3: I2S1 Left Channel");
  Serial.println("Mic 4: I2S1 Right Channel");
  Serial.println("\nStarting audio level monitoring...");
  
  delay(1000);
}

void loop() {
  size_t bytes_read0, bytes_read1;
  
  // Read from both I2S ports
  esp_err_t result0 = i2s_read(I2S_PORT_0, i2s0_buffer, sizeof(i2s0_buffer), &bytes_read0, portMAX_DELAY);
  esp_err_t result1 = i2s_read(I2S_PORT_1, i2s1_buffer, sizeof(i2s1_buffer), &bytes_read1, portMAX_DELAY);
  
  if (result0 == ESP_OK && result1 == ESP_OK) {
    // Calculate RMS levels for each channel
    float rms_levels[4] = {0, 0, 0, 0};
    int samples_per_channel = (bytes_read0 / 4) / 2; // 32-bit samples, 2 channels
    
    // Process I2S0 data (Mic 1 & 2)
    for (int i = 0; i < samples_per_channel; i++) {
      // Left channel (Mic 1)
      int32_t sample_left = i2s0_buffer[i * 2];
      rms_levels[0] += (float)(sample_left >> 8) * (sample_left >> 8); // Convert to 24-bit
      
      // Right channel (Mic 2)  
      int32_t sample_right = i2s0_buffer[i * 2 + 1];
      rms_levels[1] += (float)(sample_right >> 8) * (sample_right >> 8);
    }
    
    // Process I2S1 data (Mic 3 & 4)
    samples_per_channel = (bytes_read1 / 4) / 2;
    for (int i = 0; i < samples_per_channel; i++) {
      // Left channel (Mic 3)
      int32_t sample_left = i2s1_buffer[i * 2];
      rms_levels[2] += (float)(sample_left >> 8) * (sample_left >> 8);
      
      // Right channel (Mic 4)
      int32_t sample_right = i2s1_buffer[i * 2 + 1];
      rms_levels[3] += (float)(sample_right >> 8) * (sample_right >> 8);
    }
    
    // Calculate final RMS values
    for (int i = 0; i < 4; i++) {
      rms_levels[i] = sqrt(rms_levels[i] / samples_per_channel);
    }
    
    // Display levels every 100ms worth of data
    static unsigned long last_display = 0;
    if (millis() - last_display > 100) {
      Serial.printf("Mic Levels - 1: %6.0f  2: %6.0f  3: %6.0f  4: %6.0f\n", 
                    rms_levels[0], rms_levels[1], rms_levels[2], rms_levels[3]);
      last_display = millis();
    }
  }
}

// Optional: Function to test individual microphones
void testIndividualMics() {
  Serial.println("\n=== Individual Microphone Test ===");
  Serial.println("Speak near each microphone when prompted...\n");
  
  for (int mic = 0; mic < 4; mic++) {
    Serial.printf("Testing Microphone %d - Speak now!\n", mic + 1);
    
    unsigned long test_start = millis();
    float max_level = 0;
    
    while (millis() - test_start < 3000) { // 3 second test
      size_t bytes_read0, bytes_read1;
      i2s_read(I2S_PORT_0, i2s0_buffer, sizeof(i2s0_buffer), &bytes_read0, portMAX_DELAY);
      i2s_read(I2S_PORT_1, i2s1_buffer, sizeof(i2s1_buffer), &bytes_read1, portMAX_DELAY);
      
      float level = 0;
      int samples = 0;
      
      // Extract data for specific microphone
      if (mic < 2) { // I2S0 mics
        samples = (bytes_read0 / 4) / 2;
        for (int i = 0; i < samples; i++) {
          int32_t sample = (mic == 0) ? i2s0_buffer[i * 2] : i2s0_buffer[i * 2 + 1];
          level += abs(sample >> 8);
        }
      } else { // I2S1 mics
        samples = (bytes_read1 / 4) / 2;
        for (int i = 0; i < samples; i++) {
          int32_t sample = (mic == 2) ? i2s1_buffer[i * 2] : i2s1_buffer[i * 2 + 1];
          level += abs(sample >> 8);
        }
      }
      
      level = level / samples;
      if (level > max_level) max_level = level;
    }
    
    Serial.printf("Mic %d max level: %.0f %s\n", mic + 1, max_level, 
                  (max_level > 1000) ? "✓ GOOD" : "✗ LOW/NO SIGNAL");
    delay(1000);
  }
}