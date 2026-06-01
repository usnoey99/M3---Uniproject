#include <Arduino.h>
#define LED_PIN 2

void setup() {
  // LED Output Mode
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // LED On
  digitalWrite(LED_PIN, HIGH);
  delay(1000);

  // LED Off
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}