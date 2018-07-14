#define aprintf(...) for(char _b[100]; snprintf(_b, sizeof(_b), __VA_ARGS__), Serial.print(_b), 0;)

#define PIN_MAP(PIN)     g_ADigitalPinMap[PIN]
#define NRF24_PIN_CE     9

uint8_t tx_buf[2] = {0x00, 0xFF};
uint8_t rx_buf[2] = {0x00, 0x00};

void setup() {
  digitalWrite(NRF24_PIN_CE, LOW);
  pinMode(NRF24_PIN_CE, OUTPUT);  

  digitalWrite(SS, HIGH);
  pinMode(SS, OUTPUT);  

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

  digitalWrite(SS, LOW);

  NRF_SPIM0->TASKS_START = 1;                /* Start SPI transaction          */
  
  while(NRF_SPIM0->EVENTS_END == 0);
  NRF_SPIM0->EVENTS_END = 0;

  digitalWrite(SS, HIGH);

  aprintf("R%02X = 0x%02X\n\r", tx_buf[0], rx_buf[1]);

  if(++tx_buf[0] > 0x1D) { 
    aprintf("=================\n\r");
    tx_buf[0] = 0;
    delay(5000);
  }
}
