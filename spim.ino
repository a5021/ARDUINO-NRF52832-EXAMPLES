#define aprintf(...) for(char _b[100]; snprintf(_b, sizeof(_b), __VA_ARGS__), Serial.print(_b), 0;)

#define PIN_MAP(PIN)     g_ADigitalPinMap[PIN]

uint8_t tx_buf[2] = {0x00, 0xFF};
uint8_t rx_buf[2] = {0x00, 0x00};

void setup() {

  NRF_GPIOTE->CONFIG[0] = (
    (GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos)           |
    ((g_ADigitalPinMap[SS]) << GPIOTE_CONFIG_PSEL_Pos) |
    (GPIOTE_CONFIG_OUTINIT_High << GPIOTE_CONFIG_OUTINIT_Pos)
  );

  NRF_PPI->CH[0].EEP = (uint32_t)&NRF_SPIM0->EVENTS_STARTED;
  NRF_PPI->CH[0].TEP = (uint32_t)&NRF_GPIOTE->TASKS_CLR[0];
  
  NRF_PPI->CH[1].EEP = (uint32_t)&NRF_SPIM0->EVENTS_END;
  NRF_PPI->CH[1].TEP = (uint32_t)&NRF_GPIOTE->TASKS_SET[0]; 

  NRF_PPI->CHEN = PPI_CHEN_CH0_Msk | PPI_CHEN_CH1_Msk;

  NRF_SPIM0->PSEL.SCK  = PIN_MAP(SCK);       /* Pin select for SCK             */
  NRF_SPIM0->PSEL.MOSI = PIN_MAP(MOSI);      /* Pin select for MOSI signal     */
  NRF_SPIM0->PSEL.MISO = PIN_MAP(MISO);      /* Pin select for MISO signal     */  

  NRF_SPIM0->FREQUENCY = SPIM_FREQUENCY_FREQUENCY_M8;  /* SPI frequency.       */

  NRF_SPIM0->TXD.PTR = (uint32_t) &tx_buf;   /* set pointer to TX buffer       */
  NRF_SPIM0->RXD.PTR = (uint32_t) &rx_buf;   /* set pointer to TX buffer       */

  NRF_SPIM0->RXD.MAXCNT = 2;   /* Maximum number of bytes in receive buffer    */  
  NRF_SPIM0->TXD.MAXCNT = 2;   /* Maximum number of bytes in transmit buffer   */

  NRF_SPIM0->ENABLE = SPIM_ENABLE_ENABLE_Enabled;      /* Enable SPIM          */

  Serial.begin(9600);
}

void loop() {

  NRF_SPIM0->TASKS_START = 1;                /* Start SPI transaction          */
  
  while(NRF_SPIM0->EVENTS_END == 0);
  NRF_SPIM0->EVENTS_END = 0;

  aprintf("R%02X = 0x%02X\r\n", tx_buf[0], rx_buf[1]);

  if(++tx_buf[0] > 0x1D) { 
    aprintf("=================\n\r");
    tx_buf[0] = 0;
    delay(5000);
  }
}
