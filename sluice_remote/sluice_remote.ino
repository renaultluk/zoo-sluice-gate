#include <WiFi.h>
#include <esp_now.h>

#define NUM_SWITCHES 12

/* E8:6B:EA:D4:6A:B8 */
uint8_t broadcastAddress[] = {0xE8, 0x6B, 0xEA, 0xD4, 0x6A, 0xB8};

esp_now_peer_info_t peerInfo;

enum SluiceLevel {
  SLUICE_LOW,
  SLUICE_MID,
  SLUICE_HIGH,
  SLUICE_UNINIT,
};

typedef struct payload {
  int servoIndex;
  SluiceLevel level;
} payload;

payload message;

int switchPins[NUM_SWITCHES * 2] = {
  36, 39, 
  34, 35, 
  32, 33, 
  25, 26, 
  27, 14, 
  12, 13, 
  15, 2, 
  4, 16, 
  17, 5, 
  18, 19,
  21, 3,
  1, 22,
 };

SluiceLevel switchStates[NUM_SWITCHES];

bool checkSwitchFlag = false;

void IRAM_ATTR switchISR() {
  checkSwitchFlag = true;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  for (int i = 0; i < NUM_SWITCHES; i++) {
    switchStates[i] = SLUICE_UNINIT;
  }
  Serial.println("Finished setup");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (checkSwitchFlag) {
    checkSwitchFlag = false;

    for (int i = 0; i < NUM_SWITCHES; i+=2) {
      SluiceLevel newState;
      int reading1 = digitalRead(switchPins[i]);
      int reading2 = digitalRead(switchPins[i+1]);
      if (reading1 == HIGH) newState = SLUICE_LOW;
      else if (reading2 == HIGH) newState = SLUICE_HIGH;
      else newState = SLUICE_MID;

      if (newState != switchStates[i]) {
        switchStates[i] = newState;
        message.servoIndex = i;
        message.level = newState;

        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &message, sizeof(message));
        if (result == ESP_OK) {
          Serial.println("Message sent successfully");
        }
        else {
          Serial.println("Error sending message");
        }
      }
    }
  }
}
