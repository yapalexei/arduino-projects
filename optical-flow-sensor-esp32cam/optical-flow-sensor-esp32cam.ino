#if !defined ESP32
  #error This sketch is only for an ESP32Cam module
#endif

#include "esp_camera.h"
#include "soc/soc.h"                       
#include "soc/rtc_cntl_reg.h"      

#define CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32      // power to camera (on/off)
#define RESET_GPIO_NUM    -1      // -1 = not used
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26      // i2c sda
#define SIOC_GPIO_NUM     27      // i2c scl
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25      // vsync_pin
#define HREF_GPIO_NUM     23      // href_pin
#define PCLK_GPIO_NUM     22      // pixel_clock_pin

#define WIDTH 96                 // image size should match FRAMESIZE_96X96
#define HEIGHT 96                // image size should match FRAMESIZE_96X96

const int serialSpeed = 115200;                        // Serial data speed to use
int cameraImageExposure = 0;                         // Camera exposure (0 - 1200)   If gain and exposure both set to zero then auto adjust is enabled
int cameraImageGain = 0;                             // Image gain (0 - 30)

bool initCamera();
bool cameraImageSettings();

void setup() {
  Serial.begin(serialSpeed);
  Serial.println("disabling brownout detector");
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.println("initializing camera...");
  if (initCamera()) {
    Serial.println("OK");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  capture_frame();
  delay(100);
}

bool initCamera() {
  
    camera_config_t config;
  
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;               // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    config.pixel_format = PIXFORMAT_GRAYSCALE;              // Options =  YUV422, GRAYSCALE, RGB565, JPEG, RGB888
    config.frame_size = FRAMESIZE_96X96;          // Image sizes: 160x120 (QQVGA), 128x160 (QQVGA2), 176x144 (QCIF), 240x176 (HQVGA), 320x240 (QVGA), 
                                                  //              400x296 (CIF), 640x480 (VGA, default), 800x600 (SVGA), 1024x768 (XGA), 1280x1024 (SXGA), 
                                                  //              1600x1200 (UXGA)
    config.jpeg_quality = 63;                     // 0-63 lower number means higher quality
    config.fb_count = 2;                          // if more than one, i2s runs in continuous mode. Use only with JPEG

    // check the esp32cam board has a psram chip installed (extra memory used for storing captured images)
    //    Note: if not using "AI thinker esp32 cam" in the Arduino IDE, PSRAM must be enabled
    if (!psramFound()) {
      Serial.println("Warning: No PSRam found so defaulting to image size 'CIF'");
      config.frame_size = FRAMESIZE_CIF;
    }
  
    //#if defined(CAMERA_MODEL_ESP_EYE)
    //  pinMode(13, INPUT_PULLUP);
    //  pinMode(14, INPUT_PULLUP);
    //#endif  
  
    esp_err_t camerr = esp_camera_init(&config);  // initialise the camera
    if (camerr != ESP_OK) {
      Serial.printf("ERROR: Camera init failed with error 0x%x", camerr);
    }

    cameraImageSettings();                        // apply custom camera settings  
    
    return (camerr == ESP_OK);                    // return boolean result of camera initialisation
}

bool cameraImageSettings() { 
   
    sensor_t *s = esp_camera_sensor_get();    
    // something to try?:     if (s->id.PID == OV3660_PID) 
    if (s == NULL) {
      Serial.println("Error: problem reading camera sensor settings");
      return 0;
    } 

    // if both set to zero enable auto adjust
    if (cameraImageExposure == 0 && cameraImageGain == 0) {              
      // enable auto adjust
        s->set_gain_ctrl(s, 1);                       // auto gain on 
        s->set_exposure_ctrl(s, 1);                   // auto exposure on 
        s->set_awb_gain(s, 1);                        // Auto White Balance enable (0 or 1)
    } else {
      // Apply manual settings
        s->set_gain_ctrl(s, 0);                       // auto gain off 
        s->set_awb_gain(s, 1);                        // Auto White Balance enable (0 or 1)
        s->set_exposure_ctrl(s, 0);                   // auto exposure off 
        s->set_agc_gain(s, cameraImageGain);          // set gain manually (0 - 30)
        s->set_aec_value(s, cameraImageExposure);     // set exposure manually  (0-1200)
    }

    return 1;
}
String preMsg = F("Should be capturing frame");
String postMsg = F("captured frame!");
String out = "";
uint8_t DIV = 4;
uint8_t ROWS = WIDTH / DIV;
const int len = 576; //ROWS*ROWS;
uint8_t * outb[len] = {};
bool capture_frame() {
  Serial.println(preMsg);
  camera_fb_t *frame = esp_camera_fb_get();
  if (!frame) return false;
  
  out = "";
  for (size_t i = 0; i < frame->len; i++) {
    uint8_t pixel = frame->buf[i];                     // pixel value
    int x = i % WIDTH;                   // x position in image
    // int y = floor(i / WIDTH);            // y position in image
    int SUB = i % DIV;
    int ii = i / DIV;

    if (SUB == 0) {
      if (ii > 0) {
        outb[ii-1] = outb[ii-1] / (uint8_t*)DIV;
        out += String(outb[ii-1]);
      }
      outb[ii] = pixel;
    } else {
      outb[ii] += pixel;
    }
    if (i % ROWS == 0 && i > 0) {
      out += "\n";
    }


    // if (ii % ROWS == 0) {
    //   out += "\n";
    // }
    

    // // show data
    // if (x==0) out += "\n";               // new line
    // if (pixel < 100) {
    //   out += "0";                 // print byte as a string
    // }
    // out += String(pixel/10);                 // print byte as a string
    // out += "`";
  }

  Serial.println(out);
  esp_camera_fb_return(frame);                      // return storage space
  return true;
}
