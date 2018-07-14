void print_clock_cfg(void) {
  Serial.print("High frequency clock is ");
  if ( (NRF_CLOCK->HFCLKSTAT & CLOCK_HFCLKSTAT_STATE_Msk) == 0) {
    Serial.println("not running.");
  } else {
    Serial.print("running. Clock source: 64 MHz ");
    if ((NRF_CLOCK->HFCLKSTAT & CLOCK_HFCLKSTAT_SRC_Msk) == 0) {
      Serial.print("internal (HFINT) ");
    } else {
      Serial.print("crystal (HFXO) ");
    }
    Serial.println("oscillator.");
  }

  Serial.print("Low frequency clock is ");
  if ((NRF_CLOCK->LFCLKSTAT & CLOCK_LFCLKSTAT_STATE_Msk) == 0) {
    Serial.println("not running.");
  } else {
    Serial.print("running. Clock source: 32.768 kHz ");
    if ((NRF_CLOCK->LFCLKSTAT & CLOCK_LFCLKSTAT_SRC_Msk) == CLOCK_LFCLKSTAT_SRC_RC) {
      Serial.println("RC oscillator.");
    } else if ((NRF_CLOCK->LFCLKSTAT & CLOCK_LFCLKSTAT_SRC_Msk) == CLOCK_LFCLKSTAT_SRC_Xtal) {
      Serial.println("crystal oscillator.");
    } else if ((NRF_CLOCK->LFCLKSTAT & CLOCK_LFCLKSTAT_SRC_Msk) == CLOCK_LFCLKSTAT_SRC_Synth) {
      Serial.println("synthesized from HFCLK.");
    }
  }
}

__STATIC_INLINE void clock_switch(void) {
  if (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {
    /* Start 32 MHz crystal oscillator */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;
     /* Wait for the external oscillator to start up */
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
  }

  if (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0) {
    /* Start low frequency crystal oscillator */
    NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1;
     /* Wait for the external oscillator to start up */
    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0);
  }
}


void setup() {
  Serial.begin(9600);
}

void loop() {

  Serial.println("=== BEFORE:");

  print_clock_cfg();
 
  Serial.println("=== AFTER:");

  clock_switch();
  
  print_clock_cfg();
  while(1);
  
}
