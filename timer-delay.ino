void tim_delay(uint32_t ms) {
    // set period
  NRF_TIMER0->CC[0] = ms * 1000;
    // run timer
  NRF_TIMER0->TASKS_START = 1;
    // wait for counting period
  while(NRF_TIMER0->EVENTS_COMPARE[0] == 0);
    // reset compare flag
  NRF_TIMER0->EVENTS_COMPARE[0] = 0;
}

void setup() {
    // set 24-bit mode for TIMER0
  NRF_TIMER0->BITMODE = 2;
    // stop counter on compare & clear on compare
  NRF_TIMER0->SHORTS = (
    TIMER_SHORTS_COMPARE0_STOP_Enabled << TIMER_SHORTS_COMPARE0_STOP_Pos |
    TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos
  );
    // reset compare flag
  NRF_TIMER0->EVENTS_COMPARE[0] = 0;
    // set LED pin to OUTPUT mode
  pinMode(LED_BUILTIN, OUTPUT);
}

static uint8_t led_state; // LED state

void loop() {
  tim_delay(500);
    // switch led
  digitalWrite(LED_BUILTIN, (led_state == 0) ? HIGH : LOW);
    // toggle led-status variable
  led_state ^= 1;
}
