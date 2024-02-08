#include <ESP32Servo.h>
#include <WiFi.h>
#include <esp_now.h>

#define NUM_SERVOS 12

enum SluiceLevel {
  SLUICE_LOW,
  SLUICE_MID,
  SLUICE_HIGH,
};

typedef struct payload {
  int servoIndex;
  SluiceLevel level;
} payload;

payload message;

int servoPins[NUM_SERVOS] = {25, 26, 27, 14, 13, 12, 19, 18, 5, 4, 2, 15};
ESP32Servo *servos[NUM_SERVOS];

void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  struct payload* message = (struct payload*)incomingData;
  int angle = message->level == SLUICE_LOW ? 0 : message->level == SLUICE_MID ? 90 : 180;
  servos[message->servoIndex]->write(angle);
  delay(200);
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
  
  for (int i = 0; i < NUM_SERVOS; i++) {
    servos[i] = new ESP32Servo(servoPins[i]);
    servos[i]->write(0);
    delay(500);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
