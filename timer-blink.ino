void setup() {
    // set 24-bit mode for TIMER0
  NRF_TIMER0->BITMODE = 2;
    // set 0.5s count period
  NRF_TIMER0->CC[0] = 500000;
    // reset counter on compare
  NRF_TIMER0->SHORTS =
    TIMER_SHORTS_COMPARE0_CLEAR_Enabled;
    // run timer
  NRF_TIMER0->TASKS_START = 1;
    // set LED pin to OUTPUT mode
  pinMode(LED_BUILTIN, OUTPUT);
}

static uint8_t led_state; // LED state

void loop() {
    // wait for compare flag
  while(NRF_TIMER0->EVENTS_COMPARE[0] == 0); 
    // reset flag
  NRF_TIMER0->EVENTS_COMPARE[0] = 0;
    // switch led
  digitalWrite(LED_BUILTIN, (led_state == 0) ? HIGH : LOW);
    // toggle led-status variable
  led_state ^= 1;
}
