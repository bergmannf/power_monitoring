#include <HTTPClient.h>
#include <WiFi.h>
#include <esp_camera.h>
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"
#define LED_PIN 4
#define uS_TO_S_FACTOR                                                         \
  1000000                 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 300 /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;
int lampChannel = 7;       // a free PWM channel (some channels used by camera)
const int pwmfreq = 50000; // 50K pwm frequency
const int pwmresolution = 9; // duty cycle bit range
const int pwmMax = pow(2, pwmresolution) - 1;

const char *ssid = "<WIFI_SSID>";
const char *password = "<WIFI_PASSWORD>";

const char *httpAddress = "http://<IP_OF_PYTHON_SERVER>/store/power_monitor";

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
  case 1:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case 2:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case 3:
    Serial.println("Wakeup caused by timer");
    break;
  case 4:
    Serial.println("Wakeup caused by touchpad");
    break;
  case 5:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.println("Wakeup was not caused by deep sleep");
    break;
  }
}

void setupCamera() {
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

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);       // flip it back
    s->set_brightness(s, 1);  // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
}

void setupWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("WiFi connected - ip is:");
  Serial.println(WiFi.localIP());
}

void setupLed() {
  ledcSetup(lampChannel, pwmfreq, pwmresolution); // configure LED PWM channel
  ledcAttachPin(LED_PIN, lampChannel); // attach the GPIO pin to the channel
  int val = 96;
  int brightness = round((pow(2, (1 + (val * 0.02))) - 2) / 6 * pwmMax);
  ledcWrite(lampChannel, brightness);
}

void turnOffLed() { ledcWrite(lampChannel, 0); }

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Serial.setDebugOutput(true);
  Serial.println();
  ++bootCount;

  Serial.println("Boot number: " + String(bootCount));
  print_wakeup_reason();

  setupWifi();
  setupCamera();
  pinMode(LED_PIN, OUTPUT);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
                 " Seconds");

  Serial.println("Using LED.");
  setupLed();
  delay(1000);
  Serial.println("Taking a picture.");
  camera_fb_t *pic = esp_camera_fb_get();
  Serial.printf("Picture taken! Its size was: %zu bytes\n", pic->len);
  turnOffLed();

  WiFiClient client;
  HTTPClient http;
  // The url will be "<base>" + "/" + "clientIdentifier"
  Serial.print("Calling URL: ");
  Serial.println(httpAddress);
  http.begin(client, httpAddress);
  http.addHeader("Content-Type", "multipart/form-data");
  int responseCode = http.POST(pic->buf, pic->len);
  Serial.print("HTTP Response code:");
  Serial.println(responseCode);

  // use pic->buf to access the image
  esp_camera_fb_return(pic);

  Serial.flush();
  esp_deep_sleep_start();
}
void loop() {
  // put your main code here, to run repeatedly:
}
