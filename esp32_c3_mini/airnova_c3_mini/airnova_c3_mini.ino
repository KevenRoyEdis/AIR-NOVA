#include <Wire.h>
#include "MAX30105.h"
#include <esp_now.h>
#include <WiFi.h>

MAX30105 particleSensor;

// ----------------- MESSAGE -----------------
typedef struct struct_message {
  int hr;
  int spo2;
} struct_message;

struct_message sendData;

// Broadcast MAC
uint8_t broadcastAddress[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

const int BUFFER_SIZE = 300;
const int HR_DETECTION_MIN_MS = 400;

uint32_t timestamps[BUFFER_SIZE];
uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];
int bufferIndex = 0;
int bufferCount = 0;

// ----------------- CALLBACK -----------------
volatile bool lastSendSuccess = false;
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  lastSendSuccess = (status == ESP_NOW_SEND_SUCCESS);
}

// ----------------- HELPERS -----------------
double meanWindow(const uint32_t *buf, int len) {
  uint64_t s = 0;
  for (int i=0;i<len;i++) s += buf[i];
  return (double)s / (double)len;
}

double acAmplitude(const uint32_t *buf, int len) {
  uint32_t mn = UINT32_MAX, mx = 0;
  for (int i=0;i<len;i++) {
    if (buf[i] < mn) mn = buf[i];
    if (buf[i] > mx) mx = buf[i];
  }
  return (double)(mx - mn) / 2.0;
}

int computeHRFromPeaks() {
  if (bufferCount < 3) return 0;
  double meanIR = meanWindow(irBuffer, bufferCount);
  double mx = 0;
  for (int i=0;i<bufferCount;i++) if (irBuffer[i] > mx) mx = irBuffer[i];
  double thresh = meanIR + 0.25 * (mx - meanIR);
  int lastPeakIdx=-1, prevPeakIdx=-1;
  for (int i=1;i<bufferCount-1;i++) {
    if (irBuffer[i]>thresh && irBuffer[i]>irBuffer[i-1] && irBuffer[i]>irBuffer[i+1]) {
      prevPeakIdx = lastPeakIdx;
      lastPeakIdx = i;
    }
  }
  if (prevPeakIdx<0 || lastPeakIdx<0) return 0;
  uint32_t dt = timestamps[lastPeakIdx] - timestamps[prevPeakIdx];
  if (dt < HR_DETECTION_MIN_MS) return 0;
  return int(round(60000.0 / dt));
}

int computeSpO2() {
  if (bufferCount<10) return 0;
  double dcIR=meanWindow(irBuffer, bufferCount);
  double dcRed=meanWindow(redBuffer, bufferCount);
  double acIR=acAmplitude(irBuffer, bufferCount);
  double acRed=acAmplitude(redBuffer, bufferCount);
  if (dcIR<=0 || dcRed<=0 || acIR<=1e-6) return 0;
  double ratio = (acRed/dcRed)/(acIR/dcIR);
  double spo2 = 104.0 - 17.0 * ratio;
  if (spo2>100) spo2=100;
  if (spo2<50) spo2=50;
  return int(round(spo2));
}

bool safeSendBroadcast(struct_message *data) {
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, broadcastAddress, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
  esp_err_t res = esp_now_send(broadcastAddress,(uint8_t*)data,sizeof(*data));
  return (res == ESP_OK);
}

// ----------------- SETUP -----------------
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 not found!");
    while(1) delay(1000);
  }
  
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeIR(0x1F);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while(1) delay(1000);
  }
  esp_now_register_send_cb(OnDataSent);

  Serial.println("Sender ready. Broadcasting HR/SpO2");
}

// ----------------- LOOP -----------------
void loop() {
  long ir = particleSensor.getIR();
  long red = particleSensor.getRed();
  uint32_t now = millis();

  irBuffer[bufferIndex] = ir;
  redBuffer[bufferIndex] = red;
  timestamps[bufferIndex] = now;
  bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
  if (bufferCount < BUFFER_SIZE) bufferCount++;

  int hr = 0;
  int spo2 = 0;

  // Lower threshold for finger detection
  if (ir > 15000) {
    hr = computeHRFromPeaks();
    spo2 = computeSpO2();
  }

  sendData.hr = (hr>0)?constrain(hr,30,210):0;
  sendData.spo2 = (spo2>0)?constrain(spo2,50,100):0;

  safeSendBroadcast(&sendData);

  // Debug print
  Serial.print("IR="); Serial.print(ir);
  Serial.print(" | Red="); Serial.print(red);
  Serial.print(" | HR="); Serial.print(sendData.hr);
  Serial.print(" | SpO2="); Serial.println(sendData.spo2);

  delay(250); // send ~4Hz
}
