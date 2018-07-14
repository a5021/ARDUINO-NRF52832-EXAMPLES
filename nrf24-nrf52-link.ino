#include <stdio.h>
#include <string.h>

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

void nrf24_write_register(uint8_t r, uint8_t d) {
  digitalWrite(NRF24_PIN_CSN, LOW);
  /* 'W_REGISTER' command code = 0x20     */
  /* 'REGISTER_MASK' = 0x1F               */
  spi_putc(0x20 | (0x1F & r));
  spi_putc(d);
  digitalWrite(NRF24_PIN_CSN, HIGH);
}

void nrf24_write_payload(uint8_t pl[]) {
  digitalWrite(NRF24_PIN_CSN, LOW);
  spi_putc(0xB0); /* 0xB0 = W_TX_PAYLOAD_NOACK */
  for (uint8_t i = 0; pl[i] != 0; i++) spi_putc(pl[i]);
  digitalWrite(NRF24_PIN_CSN, HIGH);
}

static esb_payload_t nrf52_payload;
static uint8_t nrf24_payload[PL_SIZE + 1];

void setup() {

  /************* START CRYSTALL OSCILLATORS *********************************************************/
  
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

  /************* SET RADIO SHORTCUTS ****************************************************************/

  NRF_RADIO->SHORTS = (   /* Shortcut register                                                      */
    (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos)    |
    (RADIO_SHORTS_END_START_Enabled << RADIO_SHORTS_END_START_Pos)                                       
  );

  /************* SET RF PARAMETERS ******************************************************************/

  NRF_RADIO->MODE = RADIO_MODE_MODE_Nrf_2Mbit;    /* Data rate and modulation                       */

  /************* SET DATA PACKET PARAMETERS *********************************************************/

  NRF_RADIO->PCNF0 = (                            /* Packet configuration register 0                */
    (6 << RADIO_PCNF0_LFLEN_Pos)                     | /* 6 bits for LENGTH field                   */
    (3 << RADIO_PCNF0_S1LEN_Pos)                       /* 3 bits for PKT ID & NO_ACK field          */
  );

  NRF_RADIO->PCNF1 = (                            /* Packet configuration register 1                */
    (RADIO_PCNF1_ENDIAN_Big << RADIO_PCNF1_ENDIAN_Pos) | /* Most significant bit on air first       */
    (5 << RADIO_PCNF1_BALEN_Pos)                       | /* Base address length in number of bytes  */
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

  NRF_RADIO->PACKETPTR = (uint32_t)&nrf52_payload; /* Set pointer to RX buffer                      */

  /************* START RADIO ************************************************************************/
  
  NRF_RADIO->TASKS_RXEN = 1;                    /* Enable RADIO in RX mode                          */

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
    1 * (1 << 2)     |     /* Enable Dynamic Payload             */
    0 * (1 << 1)     |     /* Disable Payload with ACK           */
    1 * (1 << 0)           /* Enable W_TX_PAYLOAD_NOACK          */
  );
  
  /* Power NRF24L01's transmitter up                             */
  /* 'CONFIG' register address = 0x00                            */
  nrf24_write_register(0x00,
    0 * (1 << 0)     |     /* RX/TX control:  1=PRX, 0=PTX       */
    1 * (1 << 1)     |     /* Power control:  1=UP, 0=DOWN       */
    1 * (1 << 2)     |     /* CRC encoding:   0=8bit, 1=16bit    */
    1 * (1 << 3)     |     /* CRC control:    0=OFF, 1=ON        */
    1 * (1 << 4)     |     /* MAX_RT IRQ:     0=ON,  1=OFF       */
    1 * (1 << 5)     |     /* TS_DS  IRQ:     0=ON,  1=OFF       */
    1 * (1 << 6)           /* RX_DR  IRQ:     0=ON,  1=OFF       */
  );
  
  /***** RISE CE PIN HIGH TO SWITCH NRF24 TO STANDBY-II MODE *****/
  
  digitalWrite(NRF24_PIN_CE, HIGH);

  Serial.begin(9600);  /* Initialize serial console              */
}

void loop() {

  /*********** CLEAR TX & RX BUFFERS *****************************/

  memset(&nrf24_payload, 0, sizeof(nrf24_payload));
  memset(&nrf52_payload, 0, sizeof(nrf52_payload));

  /*********** GENERATE PAYLOAD AND SEND IT TO TRANSMITTER *******/

  sprintf((char *)nrf24_payload, "Radio-TEST (%08X / %08X)", rand(), rand());
  nrf24_write_payload(nrf24_payload);

  /*********** WAIT FOR INCOMING PACKET **************************/
  uint32_t timeout_counter = 0;
  while(NRF_RADIO->EVENTS_CRCOK == 0) { /* wait for CRC OK flag  */
    if (++timeout_counter > 1000000) return;
  }
  NRF_RADIO->EVENTS_CRCOK = 0;          /* clear event flag      */

  /*********** PRINT DATA SENT ***********************************/

  Serial.print("TX: ");
  Serial.println((char*)&nrf24_payload);
  
  /*********** PRINT DATA RECEIVED *******************************/
  
  Serial.print("RX: ");
  Serial.println((char*)&nrf52_payload.pl);
  
}
