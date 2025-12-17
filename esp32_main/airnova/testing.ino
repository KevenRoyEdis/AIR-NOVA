// #include <ESP32Servo.h>

// #define VENT_SERVO_PIN 18
// #define VENT_OPEN   0
// #define VENT_CLOSE  90

// Servo ventServo;
// int currentPos = VENT_CLOSE;

// void setup() {
//   Serial.begin(115200);

//   ventServo.attach(VENT_SERVO_PIN);
//   ventServo.write(VENT_CLOSE);

//   Serial.println("Vent Servo Manual Test Ready");
//   Serial.println("Type:");
//   Serial.println("  o = OPEN");
//   Serial.println("  c = CLOSE");
//   Serial.println("  s = STATUS");
// }

// void loop() {
//   if (Serial.available()) {
//     char cmd = Serial.read();

//     if (cmd == 'o' || cmd == 'O') {
//       ventServo.write(VENT_OPEN);
//       currentPos = VENT_OPEN;
//       Serial.println("Vent OPEN");
//     }
//     else if (cmd == 'c' || cmd == 'C') {
//       ventServo.write(VENT_CLOSE);
//       currentPos = VENT_CLOSE;
//       Serial.println("Vent CLOSED");
//     }
//     else if (cmd == 's' || cmd == 'S') {
//       Serial.print("Current Position: ");
//       Serial.println(currentPos);
//     }
//   }
// }
