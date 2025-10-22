#ifndef __CAMERA_H
#define __CAMERA_H

#include "esp_camera.h"

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22



void setupCamera()
{
  esp_camera_deinit();

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
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //config.pixel_format = PIXFORMAT_RGB565;
  //config.pixel_format = PIXFORMAT_RGB888;
  //config.pixel_format = PIXFORMAT_GRAYSCALE;
  //init with high specs to pre-allocate larger buffers
  //config.frame_size = FRAMESIZE_QQVGA;
  config.frame_size = FRAMESIZE_QVGA;//320x240
  config.fb_count = 1;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10; // 10.best 63.worst
  config.fb_count = 1;
/*
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  }
  */
  /*
    if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
    //Serial.println("Psram");
    } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    }
  */

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  sensor_t * s = esp_camera_sensor_get();
  
  // General settings *STANDARD*
  s->set_quality(s, 10);          // 0-63, lower is higher quality. Start at 16 for a balance of file size and quality.
  s->set_contrast(s, 0);          // -2 to 2, 0 is default
  s->set_brightness(s, 0);        // -2 to 2, 0 is default
  s->set_saturation(s, 0);        // -2 to 2, 0 is default
  s->set_special_effect(s, 0);    // 0 = No Effect
  
  // White balance
  s->set_whitebal(s, 1);          // 1 = enable AWB
  s->set_awb_gain(s, 1);          // 1 = enable AWB gain control
  s->set_wb_mode(s, 0);           // 0 = Auto white balance
  
  // Exposure and gain control
  s->set_exposure_ctrl(s, 1);     // 1 = enable auto exposure control (AEC)
  s->set_aec2(s, 0);              // 0 = disable AEC2 (reduces banding)
  s->set_ae_level(s, 0);          // -2 to 2, 0 is default
  s->set_aec_value(s, 300);       // Auto exposure value. Can be set manually if AEC is off.
  s->set_gain_ctrl(s, 1);         // 1 = enable auto gain control (AGC)
  s->set_agc_gain(s, 0);          // 0 = auto gain value. Can be set manually if AGC is off.
  s->set_gainceiling(s, (gainceiling_t)4); // 4 = 16x gain. Adjust as needed.
  
  // Other adjustments
  s->set_lenc(s, 1);              // 1 = enable lens correction (LENC)
  s->set_bpc(s, 1);               // 1 = enable black pixel correction (BPC)
  s->set_wpc(s, 1);               // 1 = enable white pixel correction (WPC)
  s->set_raw_gma(s, 1);           // 1 = enable gamma correction
  
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
    s->set_saturation(s, -2);//lower the saturation
  }
  //drop down frame size for higher initial frame rate
  //  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

}
#endif
