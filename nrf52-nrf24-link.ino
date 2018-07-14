#define PL_SIZE 32

typedef struct {
  uint8_t len;
  uint8_t id;
  uint8_t pl[PL_SIZE + 1];
} esb_payload_t;

#define NRF24_PIN_CE     9
#define NRF24_PIN_CSN    SS

uint8_t spi_putc(uint8_t c) {
  NRF_SPI0->TXD = c;
  while (NRF_SPI0->EVENTS_READY == 0);
  NRF_SPI0->EVENTS_READY = 0;
  return NRF_SPI0->RXD;
}

uint8_t nrf24_read_register(uint8_t r) {
  digitalWrite(NRF24_PIN_CSN, LOW);
  /* 'R_REGISTER' command code = 0x00     */
  /* 'REGISTER_MASK' = 0x1F               */
  spi_putc(0x1F & r);
  uint res = spi_putc(0xFF);
  digitalWrite(NRF24_PIN_CSN, HIGH);
  return res;
}
 
void nrf24_write_register(uint8_t r, uint8_t d) {
  digitalWrite(NRF24_PIN_CSN, LOW);
  /* 'W_REGISTER' command code = 0x20     */
  /* 'REGISTER_MASK' = 0x1F               */
  spi_putc(0x20 | (0x1F & r));
  spi_putc(d);
  digitalWrite(NRF24_PIN_CSN, HIGH);
}

uint8_t nrf24_get_payload_size(void) {
  digitalWrite(NRF24_PIN_CSN, LOW);
  spi_putc(0x60); /* R_RX_PL_WID command code = 0x60    */
  uint8_t p_size = spi_putc(0xFF);
  digitalWrite(NRF24_PIN_CSN, HIGH);
  return p_size;
}

void nrf24_read_payload(uint8_t pl[], uint8_t len) {
  digitalWrite(NRF24_PIN_CSN, LOW);
  spi_putc(0x61); /* R_RX_PAYLOAD command code = 0x61 */
  for (uint8_t i = 0; i < len; i++) pl[i] = spi_putc(0xFF);
  digitalWrite(NRF24_PIN_CSN, HIGH);
}

static esb_payload_t nrf52_payload;
static uint8_t nrf24_payload[PL_SIZE + 1];

void setup() {

  /************* START CRYSTAL OSCILLATORS **********************************************************/
  
  if (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {
    /* Start 32 MHz crystal oscillator */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;
    
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);     /* Wait for the external oscillator to start up */
  }

  if (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0) {
    /* Start low frequency crystal oscillator */
    NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1;
  
    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0);
  }

  /************* SET RF PARAMETERS ******************************************************************/

  NRF_RADIO->MODE = RADIO_MODE_MODE_Nrf_2Mbit;    /* Data rate and modulation                       */

  /************* SET DATA PACKET PARAMETERS *********************************************************/

  NRF_RADIO->PCNF0 = (                            /* Packet configuration register 0                */
    (6 << RADIO_PCNF0_LFLEN_Pos)                     | /* 6 bits for LENGTH field                   */
    (3 << RADIO_PCNF0_S1LEN_Pos)                       /* 3 bits for PKT ID & NO_ACK field          */
  );

  NRF_RADIO->PCNF1 = (                            /* Packet configuration register 1                */
    (RADIO_PCNF1_ENDIAN_Big << RADIO_PCNF1_ENDIAN_Pos) | /* Most significant bit on air first       */
    (4 << RADIO_PCNF1_BALEN_Pos)                       | /* Base address length in number of bytes  */
    (32 << RADIO_PCNF1_MAXLEN_Pos)                       /* Max payload size in bytes               */
  );

  /************* SET ADDRESS ************************************************************************/

  NRF_RADIO->BASE0 = 0xE7E7E7E7;                /* Base address 0                                   */
  NRF_RADIO->PREFIX0 = 0xE7;                    /* Prefixes bytes for logical addresses 0           */
  NRF_RADIO->RXADDRESSES = 0x01;                /* Receive address select                           */

  /************* SET CRC PARAMETERS *****************************************************************/

  NRF_RADIO->CRCCNF = RADIO_CRCCNF_LEN_Two;     /* CRC configuration: 16bit                         */
  NRF_RADIO->CRCINIT = 0xFFFFUL;                /* CRC initial value                                */
  NRF_RADIO->CRCPOLY = 0x11021UL;               /* CRC poly: x^16+x^12^x^5+1                        */

  /************* SET DATA BUFFER ********************************************************************/

  NRF_RADIO->PACKETPTR = (uint32_t)&nrf52_payload; /* Set pointer to payload buffer                 */

  /************* START RADIO ************************************************************************/
  
  NRF_RADIO->TASKS_TXEN = 1;                    /* Enable RADIO in TX mode                          */

  /************* INITIALIZE GPIO ********************************************************************/

  digitalWrite(NRF24_PIN_CSN, HIGH);
  pinMode(NRF24_PIN_CSN, OUTPUT);

  digitalWrite(NRF24_PIN_CE, LOW);
  pinMode(NRF24_PIN_CE, OUTPUT);

  /************* INITIALIZE SPI *********************************************************************/

  NRF_SPI0->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M8;  /* SPI frequency = 8MHz */

  NRF_SPI0->PSEL.MISO = g_ADigitalPinMap[MISO];
  NRF_SPI0->PSEL.MOSI = g_ADigitalPinMap[MOSI];
  NRF_SPI0->PSEL.SCK  = g_ADigitalPinMap[SCK];

  NRF_SPI0->ENABLE = 1;  /* Enable SPI0  */

  /************* CONFIGURE NRF24L01+ *****************************/

  /* Enable dynamic payload and W_TX_PAYLOAD_NOACK command       */
  /* 'FEATURE' register address = 0x1D  */
  nrf24_write_register(0x1D,
    1 * (1 << 2)     |     /* Dynamic Payload:     1=ON, 0=OFF   */
    0 * (1 << 1)     |     /* Payload with ACK:    1=ON, 0=OFF   */
    1 * (1 << 0)           /* W_TX_PAYLOAD_NOACK:  1=ON, 0=OFF   */
  );

  /* 'DYNPD' register address = 0x1C                             */ 
  nrf24_write_register(0x1C, 0x01); /* enable DYNPD on RX0       */
  
  /***** POWER NRF24L01's RECEIVER ON ****************************/
  
  /* 'CONFIG' register address = 0x00                            */
  nrf24_write_register(0x00,
    1 * (1 << 0)     |     /* RX/TX control:  1=PRX, 0=PTX       */
    1 * (1 << 1)     |     /* Power control1: 1=UP, 0=DOWN       */
    1 * (1 << 2)     |     /* CRC encoding:   0=8bit, 1=16bit    */
    1 * (1 << 3)     |     /* CRC control:    0=OFF, 1=ON        */
    1 * (1 << 4)     |     /* MAX_RT IRQ:     0=ON,  1=OFF       */
    1 * (1 << 5)     |     /* TS_DS  IRQ:     0=ON,  1=OFF       */
    1 * (1 << 6)           /* RX_DR  IRQ:     0=ON,  1=OFF       */
  );

  /***** RISE CE PIN HIGH TO BEGIN RECEIVING *********************/

  digitalWrite(NRF24_PIN_CE, HIGH);

  Serial.begin(9600);  /* Initialize serial console              */
  
  delay(5);
}

uint32_t pkt_cnt = 0, pkt_err = 0;

void loop() {
  uint8_t i, p_len;

  /*********** CLEAR RX & TX BUFFERS *****************************/

  for (i = 0; i < PL_SIZE + 1; i++) {
    nrf24_payload[i] = nrf52_payload.pl[i] = 0;
  }

  /*********** MAKE PAYLOAD **************************************/
  
  nrf52_payload.len = (rand() % (PL_SIZE / 2) + 1) * 2;
  for (i = 0; i < nrf52_payload.len; i++) {
    nrf52_payload.pl[i] = rand() & 1 ? rand() % 10 + '0' : rand() % 6 + 'A';
  }
  nrf52_payload.id = rand() & 0x06; /* rnadom pkt-id */

  /*********** SEND PAYLOAD **************************************/

  NRF_RADIO->TASKS_START = 1;   /* start transmission            */
  delay(3);                     /* wait for data transmitting    */

  /*********** READ THE DATA RECEIVED ****************************/

  p_len = nrf24_get_payload_size();
  nrf24_read_payload(nrf24_payload, p_len);

  /*********** COMPARE DATA SENT AND RECEIVED ********************/

  for (i = 0; i < nrf52_payload.len; i++) {
    if (nrf52_payload.pl[i] != nrf24_payload[i]) {
      pkt_err++;
      break;
    }
  }

  /*********** PRINT DATA SENT AND RECEIVED **********************/

  Serial.print("TX: ");
  Serial.println((char*)&nrf52_payload.pl);

  /* pad payload sting with saces */
  for (i = p_len; i < sizeof(nrf24_payload); i++) nrf24_payload[i] = ' ';

  Serial.print("RX: ");
  Serial.print((char*)&nrf24_payload);

  /*********** PRINT STATISTICS **********************************/

  Serial.print(" Pkt stat (err/total): ");
  Serial.print(pkt_err);
  Serial.print(" / ");
  Serial.println(++pkt_cnt);
}
