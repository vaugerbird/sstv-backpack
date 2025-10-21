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
/*  
  // **Optimized camera settings - DAYLIGHT**
  s->set_quality(s, 10);      // jpeg quality, 0-63 (10 to prevent out of buffer hang)
  s->set_gain_ctrl(s, 1);     // sensor automatic gain control 1.on 0.off
  //s->set_agc_gain(s, 0);      // manual agc (if sensor automatic gain control is off) values are 0-30
  //s->set_gainceiling(s, (gainceiling_t)32); // play with the maximum roof that agc can reach (0 to 64) 
  s->set_exposure_ctrl(s, 1); // auto exposure control 1.on 0.off
  s->set_aec2(s ,1);          // increased auto exposure algo 1.on 0.off
  s->set_ae_level(s, 2);      // exposure compensation -2/+2
  //s->set_aec_value(s, 0);     // if set_exposure_ctrl is 0 use this parameter for manual exposure (0-1200)
  //************************
  s->set_whitebal(s, 1);      // enable automatic white balance 1.on 0.off       
  s->set_awb_gain(s, 1);      // automatic white balance gain 1.on 0.off                
  s->set_wb_mode(s, 0);       // manual white balance, modes are 0.auto,1.sunny,2.cloudy,3.office,4.home
  //************************
  s->set_denoise(s, 0);       // denoise filter 1.on 0.off (warning, loss of details in low light)                
  s->set_lenc(s, 1);          // lens correction 1.on 0.off
  s->set_raw_gma(s, 0);       // gamma correction over raw image 1.on 0.off
  s->set_wpc(s, 1);           // wide pixel correction 1.on           
  //************************
  s->set_brightness(s, 0);    // range -2 / 2
  s->set_contrast(s, 2);      // range -2 / 2
  s->set_saturation(s, -2);   // range -2 / 2
  //************************
  s->set_vflip(s, 0);         // image vertical flip 1.on 0.off
  s->set_hmirror(s, 0);       // image horizontal mirror 1.om 0.off
  //s->set_special_effect(s, 6); //2.black&white 1.negative 6.sepia
*/
  // **Optimized camera settings - HOME**
  s->set_gain_ctrl(s, 1);       // Mantiene il controllo automatico del guadagno (AGC)
  s->set_exposure_ctrl(s, 1);   // Mantiene il controllo automatico dell'esposizione (AEC)
  s->set_aec2(s, 1);            // Mantiene l'algoritmo AEC avanzato
  s->set_ae_level(s, 2);        // Aumenta l'esposizione al massimo (+2) per schiarire l'immagine
  s->set_gainceiling(s, (gainceiling_t)64); // **Nuovo:** Aumenta il tetto massimo del guadagno automatico a 64x (valore massimo)
  
  // **Impostazioni di qualità e rumore bilanciate**
  s->set_quality(s, 15);        // **Modificato:** Abbassa leggermente la qualità per ridurre il carico sul processore, utile per la stabilità in bassa luce.
  s->set_denoise(s, 1);         // **Modificato:** Abilita la riduzione del rumore per le basse luci.
  
  // **Impostazioni di bilanciamento del bianco**
  s->set_whitebal(s, 1);        // Mantiene il bilanciamento automatico del bianco (AWB)
  s->set_awb_gain(s, 1);        // Mantiene il guadagno automatico dell'AWB
  s->set_wb_mode(s, 0);         // Modalità automatica per il bilanciamento del bianco
  
  // **Altri parametri**
  s->set_lenc(s, 1);            // Correzione lente attiva
  s->set_raw_gma(s, 0);         // Gamma correction disattivata (per evitare rumore aggiuntivo)
  s->set_wpc(s, 1);             // Wide pixel correction attiva
  
  s->set_brightness(s, 1);      // Mantiene un leggero aumento di luminosità
  s->set_contrast(s, 1);        // Mantiene un leggero aumento di contrasto
  s->set_saturation(s, -1);     // Mantiene una leggera riduzione della saturazione
  
  s->set_vflip(s, 0);           // Flip verticale disattivo
  s->set_hmirror(s, 0);         // Mirror orizzontale disattivo


  
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
