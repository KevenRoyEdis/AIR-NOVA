#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

// ----------------- PINS -----------------
#define SSR1_PIN 5
#define VENT_SERVO_PIN 18

// ----------------- SERVO -----------------
Servo ventServo;
#define VENT_OPEN  0
#define VENT_CLOSE 95

// ----------------- LCD -----------------
LiquidCrystal_I2C lcd(0x27, 20, 4);

// ----------------- ESP-NOW MESSAGE -----------------
typedef struct struct_message {
  int hr;
  int spo2;
} struct_message;

struct_message incomingData;

// ----------------- STATES -----------------
int NebulizerState = 0;
int VentilationState = 0;
bool connected = false;

// ----------------- TIMEOUT -----------------
unsigned long lastReceiveTime = 0;
#define RX_TIMEOUT 3000

// ----------------- CALLBACK -----------------
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len != sizeof(struct_message)) return;

  memcpy(&incomingData, data, sizeof(incomingData));
  lastReceiveTime = millis();
  connected = true;

  int newVent = 0;
  int newNeb  = 0;

  // ----------------- LOGIC -----------------

  // Ventilator priority condition
  if ((incomingData.hr > 120) ||
      (incomingData.spo2 > 0 && incomingData.spo2 < 85)) {
    newVent = 1;
    newNeb  = 0;   // FORCE nebulizer OFF
  }
  // Nebulizer condition (only if vent OFF)
  else if (incomingData.spo2 > 0 && incomingData.spo2 < 92) {
    newNeb  = 1;
    newVent = 0;
  }

  // ----------------- APPLY STATES -----------------

  // Nebulizer
  if (newNeb != NebulizerState) {
    digitalWrite(SSR1_PIN, newNeb);
    NebulizerState = newNeb;
  }

  // Ventilator servo
  if (newVent != VentilationState) {
    ventServo.write(newVent ? VENT_OPEN : VENT_CLOSE);
    VentilationState = newVent;
  }

  // If nebulizer is ON → servo MUST be closed
  if (NebulizerState == 1) {
    ventServo.write(VENT_CLOSE);
    VentilationState = 0;
  }

  // ----------------- LCD -----------------
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("HR: ");
  incomingData.hr == 0 ? lcd.print("No Pulse") : lcd.print(incomingData.hr);

  lcd.setCursor(0, 1);
  lcd.print("SpO2: ");
  incomingData.spo2 == 0 ? lcd.print("No Pulse") : lcd.print(incomingData.spo2);

  lcd.setCursor(0, 2);
  lcd.print("Nebulizer: ");
  lcd.print(NebulizerState ? "ON" : "OFF");

  lcd.setCursor(0, 3);
  lcd.print("Vent: ");
  lcd.print(VentilationState ? "OPEN" : "CLOSED");
}

// ----------------- SETUP -----------------
void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  pinMode(SSR1_PIN, OUTPUT);
  digitalWrite(SSR1_PIN, LOW);

  ventServo.attach(VENT_SERVO_PIN);
  ventServo.write(VENT_CLOSE);

  lcd.init();
  lcd.backlight();
  lcd.print("Receiver Ready");

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (1);
  }

  esp_now_register_recv_cb(OnDataRecv);
}

// ----------------- LOOP -----------------
void loop() {
  // ----------------- NO DATA FAILSAFE -----------------
  if (connected && millis() - lastReceiveTime > RX_TIMEOUT) {
    connected = false;

    // FORCE SAFE STATE
    digitalWrite(SSR1_PIN, LOW);
    ventServo.write(VENT_CLOSE);

    NebulizerState = 0;
    VentilationState = 0;

    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Receiver Ready");
    lcd.setCursor(0, 2);
    lcd.print("Waiting for data");

    Serial.println("NO DATA - SAFE MODE");
  }
}
