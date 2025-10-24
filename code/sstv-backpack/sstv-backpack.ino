#include <driver/ledc.h>
#include "camera.h"
#include "sin256.h"
#include "esp_task_wdt.h"

#define BELL202_BAUD 1200
#define F_SAMPLE ((BELL202_BAUD * 32) * 0.93) // ≈ 35712 Hz
#define FTOFTW (4294967295 / F_SAMPLE)
#define TIME_PER_SAMPLE (1000.0/F_SAMPLE)
#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP  5

#define LED_FLASH 4
#define LED_RED 33
#define SPEAKER_OUT 14
#define PTT_PIN 15      // Pin for PTT (HIGH = active)
#define BUZZER_PIN 13   // Pin for indication buzzer
#define CAPT_BTN 12     // Pin for button to activate capture
//#define USE_FLASH     // Uncomment to use the onboard high intensity white LED as flash

// TOP STRING COLOR (default cyan, the most readable)
#define TOP_R  255
#define TOP_G  0
#define TOP_B  255

// BOTTOM STRING COLOR (default cyan, the most readable)
#define BTM_R  255
#define BTM_G  0
#define BTM_B  255

int btnState = 0; 
int lastBtnState = 0;

int start = 0;

String overlayTextTop = "CALLSIGN HERE";    // Upper left, white
String overlayTextBottom = "SUBTEXT HERE"; // Lower right, black

volatile uint32_t FTW = FTOFTW * 1000;
volatile uint32_t PCW = 0;
volatile uint32_t TFLAG = 0;

#define FT_1000 (uint32_t) (1000 * FTOFTW)
#define FT_1100 (uint32_t) (1100 * FTOFTW)
#define FT_1200 (uint32_t) (1200 * FTOFTW)
#define FT_1300 (uint32_t) (1300 * FTOFTW)
#define FT_1500 (uint32_t) (1500 * FTOFTW)
#define FT_1900 (uint32_t) (1900 * FTOFTW)
#define FT_2200 (uint32_t) (2200 * FTOFTW)
#define FT_2300 (uint32_t) (2300 * FTOFTW)
#define FT_SYNC (uint32_t) (FT_1200)

#define MAX_WIDTH 320
#define MAX_HEIGHT 256

class SSTV_config_t {
  public:
    uint8_t vis_code;
    uint32_t width;
    uint32_t height;
    float line_time;
    float h_sync_time;
    float v_sync_time;
    float c_sync_time;
    float left_margin_time;
    float visible_pixels_time;
    float pixel_time;
    bool color;
    bool martin;
    bool robot;

    SSTV_config_t(uint8_t v) {
      vis_code = v;
      switch (vis_code) {
        case 2: // Robot B&W8
          robot = true; martin = false; color = false;
          width = 160; height = 120;
          line_time = 67.025; h_sync_time = 30.0; v_sync_time = 6.5;
          left_margin_time = 1.6;
          visible_pixels_time = line_time - v_sync_time - left_margin_time;
          pixel_time = visible_pixels_time / width;
          break;
        case 44: // Martin M1
          robot = false; martin = true; color = true;
          width = 320; height = 240;
          line_time = 446.4460001; h_sync_time = 30.0; v_sync_time = 4.862;
          c_sync_time = 0.572; left_margin_time = 0.0;
          visible_pixels_time = line_time - v_sync_time - left_margin_time - (2 * c_sync_time);
          pixel_time = visible_pixels_time / (width * 3);
          break;
      }
    }
};

camera_fb_t* fb;

SSTV_config_t* currentSSTV;

volatile uint16_t rasterX = 0;
volatile uint16_t rasterY = 0;
volatile uint8_t SSTVseq = 0;
double SSTVtime = 0;
double SSTVnext = 0;
uint8_t VISsr = 0;
uint8_t VISparity;
uint8_t HEADERptr = 0;
static uint32_t SSTV_HEADER[] = {FT_2300, 100, FT_1500, 100, FT_2300, 100, FT_1500, 100, FT_1900, 300, FT_1200, 10, FT_1900, 300, FT_1200, 30, 0, 0};
uint8_t SSTV_RUNNING = 0;

uint8_t *rgb_buf = NULL;
uint32_t rgb_buf_len;
uint16_t rgb_width;
uint16_t rgb_height;

TaskHandle_t sampleHandlerHandle;

void IRAM_ATTR audioISR() {
  PCW += FTW;
  TFLAG = 1;
}

void sampleHandler(void *p) {
  disableCore0WDT();
  while (1) {
    if (TFLAG) {
      TFLAG = 0;
      int v = SinTableH[((uint8_t*)&PCW)[3]];
      ledcWrite(SPEAKER_OUTPUT, v); // modulate the selected pin in PWM
      SSTVtime += TIME_PER_SAMPLE;
      if (!SSTV_RUNNING || SSTVtime < SSTVnext) continue;

      switch (SSTVseq) {
        case 0: // Start
          SSTVtime = 0; HEADERptr = 0; VISparity = 0; VISsr = currentSSTV->vis_code;
          FTW = SSTV_HEADER[HEADERptr++]; SSTVnext = (float)SSTV_HEADER[HEADERptr++];
          SSTVseq++; break;
        case 1: // VIS header
          if (SSTV_HEADER[HEADERptr + 1] == 0) {
            SSTVseq++; HEADERptr = 0;
          } else {
            FTW = SSTV_HEADER[HEADERptr++]; SSTVnext += (float)SSTV_HEADER[HEADERptr++];
          }
          break;
        case 2: // VIS code
          if (HEADERptr == 7) {
            HEADERptr = 0;
            FTW = VISparity ? FT_1100 : FT_1300;
            SSTVnext += 30.0; SSTVseq++;
          } else {
            FTW = (VISsr & 0x01) ? (VISparity ^= 0x01, FT_1100) : FT_1300;
            VISsr >>= 1; SSTVnext += 30.0; HEADERptr++;
          }
          break;
        case 3: // VIS stop bit/sync0
          FTW = FT_1200; SSTVnext += 30.0 + currentSSTV->h_sync_time;
          rasterX = 0; rasterY = 0; SSTVseq = 10; break;
        case 10: // Start of line Green
          if (rasterX == currentSSTV->width) {
            rasterX = 0; FTW = FT_1500; SSTVnext += currentSSTV->c_sync_time; SSTVseq++;
          } else {
            int G = rgb_buf[1 + (rasterX * 3) + (rasterY * currentSSTV->width * 3)];
            int f = map(G, 0, 255, 1500, 2300); FTW = FTOFTW * f;
            SSTVnext += currentSSTV->pixel_time; rasterX++;
          }
          break;
        case 11: // Blue
          if (rasterX == currentSSTV->width) {
            rasterX = 0; FTW = FT_1500; SSTVnext += currentSSTV->c_sync_time; SSTVseq++;
          } else {
            int B = rgb_buf[(rasterX * 3) + (rasterY * currentSSTV->width * 3)];
            int f = map(B, 0, 255, 1500, 2300); FTW = FTOFTW * f;
            SSTVnext += currentSSTV->pixel_time; rasterX++;
          }
          break;
        case 12: // Red
          if (rasterX == currentSSTV->width) {
            rasterX = 0; rasterY++;
            if (rasterY == currentSSTV->height) {
              SSTV_RUNNING = false; SSTVseq = 0; FTW = 0; PCW = 0;
            } else {
              FTW = FT_SYNC; SSTVnext += currentSSTV->v_sync_time; SSTVseq = 10;
            }
          } else {
            int R = rgb_buf[2 + (rasterX * 3) + (rasterY * currentSSTV->width * 3)];
            int f = map(R, 0, 255, 1500, 2300); FTW = FTOFTW * f;
            SSTVnext += currentSSTV->pixel_time; rasterX++;
          }
          break;
      }
    }
  }
}

void drawText(uint8_t *rgb_buf, uint16_t width, uint16_t height, const char *text_top, const char *text_bottom) {
  if (!rgb_buf) return;

  int char_width = 10;
  int char_height = 14;
  int spacing = 2;

  const uint8_t font[36][7] = {
    {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}, // A
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110}, // B
    {0b01111, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b01111}, // C
    {0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110}, // D
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111}, // E
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000}, // F
    {0b01111, 0b10000, 0b10000, 0b10011, 0b10001, 0b10001, 0b01111}, // G
    {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}, // H
    {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100}, // I
    {0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b10001, 0b01110}, // J
    {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001}, // K
    {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111}, // L
    {0b10001, 0b11011, 0b10101, 0b10001, 0b10001, 0b10001, 0b10001}, // M
    {0b10001, 0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001}, // N
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}, // O
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000}, // P
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101}, // Q
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001}, // R
    {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110}, // S
    {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100}, // T
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}, // U
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100}, // V
    {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010}, // W
    {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001}, // X
    {0b10001, 0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100}, // Y
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111}, // Z
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}, // 0
    {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}, // 1
    {0b01110, 0b10001, 0b00001, 0b00110, 0b01000, 0b10000, 0b11111}, // 2
    {0b01110, 0b10001, 0b00001, 0b00110, 0b00001, 0b10001, 0b01110}, // 3
    {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010}, // 4
    {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110}, // 5
    {0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110}, // 6
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000}, // 7
    {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110}, // 8
    {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100}  // 9
  };

  int x_start_top = 10;
  int y_start_top = 10;
  int text_top_len = strlen(text_top);

  for (int c = 0; c < text_top_len; c++) {
    int char_index = -1;
    if (text_top[c] >= 'A' && text_top[c] <= 'Z') char_index = text_top[c] - 'A';
    else if (text_top[c] >= '0' && text_top[c] <= '9') char_index = (text_top[c] - '0') + 26;
    if (char_index >= 0) {
      for (int y = 0; y < 7; y++) {
        for (int x = 0; x < 5; x++) {
          if (font[char_index][y] & (1 << (4 - x))) {
            for (int dy = 0; dy < 2; dy++) {
              for (int dx = 0; dx < 2; dx++) {
                int pixel_x = x_start_top + (c * (char_width + spacing)) + (x * 2) + dx;
                int pixel_y = y_start_top + (y * 2) + dy;
                if (pixel_x < width && pixel_y < height) {
                  int index = (pixel_y * width + pixel_x) * 3;
                  rgb_buf[index + 0] = TOP_B;
                  rgb_buf[index + 1] = TOP_G;
                  rgb_buf[index + 2] = TOP_R;
                }
              }
            }
          }
        }
      }
    }
  }

  int text_bottom_len = strlen(text_bottom);
  int x_start_bottom = width - (text_bottom_len * (char_width + spacing)) - 10;
  int y_start_bottom = height - char_height - 10;

  for (int c = 0; c < text_bottom_len; c++) {
    int char_index = -1;
    if (text_bottom[c] >= 'A' && text_bottom[c] <= 'Z') char_index = text_bottom[c] - 'A';
    else if (text_bottom[c] >= '0' && text_bottom[c] <= '9') char_index = (text_bottom[c] - '0') + 26;
    if (char_index >= 0) {
      for (int y = 0; y < 7; y++) {
        for (int x = 0; x < 5; x++) {
          if (font[char_index][y] & (1 << (4 - x))) {
            for (int dy = 0; dy < 2; dy++) {
              for (int dx = 0; dx < 2; dx++) {
                int pixel_x = x_start_bottom + (c * (char_width + spacing)) + (x * 2) + dx;
                int pixel_y = y_start_bottom + (y * 2) + dy;
                if (pixel_x < width && pixel_y < height && pixel_x >= 0) {
                  int index = (pixel_y * width + pixel_x) * 3;
                  rgb_buf[index + 0] = BTM_B;
                  rgb_buf[index + 1] = BTM_G;
                  rgb_buf[index + 2] = BTM_R;
                }
              }
            }
          }
        }
      }
    }
  }
}

void doImage() {
  camera_fb_t* fb = nullptr;
  // Skip first N frames.
  for (int i = 0; i < 3; i++) {
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = nullptr;
  }
  
  delay(1000);
  #ifdef USE_FLASH 
    digitalWrite(LED_FLASH,HIGH);
  #endif
  
  fb = esp_camera_fb_get();

  #ifdef USE_FLASH 
    digitalWrite(LED_FLASH,LOW);
  #endif
  delay(1000);
  
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  rgb_width = fb->width;
  rgb_height = fb->height;
  rgb_buf_len = rgb_width * rgb_height * 3 * sizeof(uint8_t);
  Serial.printf("Size:%ix%i*3=%i\n", rgb_width, rgb_height, rgb_buf_len);

  if (!rgb_buf) {
    rgb_buf = (uint8_t *)malloc(rgb_buf_len);
    Serial.println("RGB buff allocated");
  } else {
    Serial.println("RGB buff already allocated");
  }
  if (!rgb_buf) {
    Serial.println("RGB buff allocation failed");
    return;
  }

  fmt2rgb888(fb->buf, fb->len, fb->format, rgb_buf);
  drawText(rgb_buf, rgb_width, rgb_height, overlayTextTop.c_str(), overlayTextBottom.c_str());

  if (currentSSTV) delete currentSSTV;
  currentSSTV = new SSTV_config_t(44);
  Serial.print("Sending image");

  // Activate PTT
  Serial.println(" - Activating PTT");
  digitalWrite(PTT_PIN, HIGH);

  SSTVtime = 0; SSTVnext = 0; SSTVseq = 0; SSTV_RUNNING = true;
  vTaskResume(sampleHandlerHandle);
  while (SSTV_RUNNING) {
    Serial.print(".");
    delay(1000);
  }
  esp_task_wdt_reset();
  enableCore0WDT();
  vTaskSuspend(sampleHandlerHandle);
  Serial.println("Ok.123");

  // Deactivate PTT (LOW = inactive)
  Serial.println(" - Deactivating PTT");
  digitalWrite(PTT_PIN, LOW);

  esp_camera_fb_return(fb);
  ledcDetach(SPEAKER_OUTPUT); // this to really "shutup" the pin
  delay(10000);

  // Free the buffer after use
  free(rgb_buf);
  rgb_buf = NULL;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  setupCamera();

  pinMode(CAPT_BTN, INPUT_PULLUP);
  pinMode(LED_FLASH, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(PTT_PIN, OUTPUT); // Configure PTT as output
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_FLASH, LOW);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(PTT_PIN, LOW); // PTT inactive at startup

  hw_timer_t *timer = NULL;
  timer = timerBegin(10000000);
  timerAttachInterrupt(timer, &audioISR);
  timerAlarm(timer, 10000000 / F_SAMPLE, true, 0);

  ledcAttach(SPEAKER_OUTPUT, 200000, LEDC_TIMER_8_BIT);

  xTaskCreatePinnedToCore(sampleHandler, "IN", 4096, NULL, 1, &sampleHandlerHandle, 0);
  vTaskSuspend(sampleHandlerHandle);
}

void loop() {
  btnState = digitalRead(CAPT_BTN);
  lastBtnState = btnState;
  if (btnState == 0) {
    tone(BUZZER_PIN, 500, 100);
    tone(BUZZER_PIN, 440, 100);
    doImage();
    tone(BUZZER_PIN, 440, 100);
    tone(BUZZER_PIN, 480, 100);
    tone(BUZZER_PIN, 500, 100);
  } else {
    btnState = 1;
    lastBtnState = btnState;
  }

}
