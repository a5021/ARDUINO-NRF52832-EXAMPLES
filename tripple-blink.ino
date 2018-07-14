/* NRF52 -- Tripple blink PPI */

void setup() {
  for (int i = 0; i < 3; i++) {
    NRF_GPIOTE->CONFIG[i] = (
      (GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos)           |
      (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos) |
      ((i + 22) << GPIOTE_CONFIG_PSEL_Pos)                          |
      (GPIOTE_CONFIG_OUTINIT_Low << GPIOTE_CONFIG_OUTINIT_Pos)
    );

    NRF_PPI->CH[i].EEP = (uint32_t)&NRF_RTC2->EVENTS_COMPARE[i];
    NRF_PPI->CH[i].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[i];
    
    NRF_RTC2->CC[i] = 32768 * (i + 1) / 6;
  }

  NRF_PPI->FORK[2].TEP = (uint32_t)&NRF_RTC2->TASKS_CLEAR;
  NRF_PPI->CHEN = PPI_CHEN_CH0_Msk | PPI_CHEN_CH1_Msk | PPI_CHEN_CH2_Msk;

  NRF_RTC2->EVTENSET = RTC_EVTEN_COMPARE0_Msk | RTC_EVTEN_COMPARE1_Msk | RTC_EVTEN_COMPARE2_Msk;
  NRF_RTC2->TASKS_START = 1;
}

void loop() { /* NO ACTION */ }
