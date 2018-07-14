/* Blink without Delay -- музыка Arduino, слова народные  */

#define LED_PIN      13
#define BLINK_SPEED  10

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, (millis() >> BLINK_SPEED) & 1);
}
