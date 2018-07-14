/*

  * E73 drives NRF24L01 *

Connect as follows:

SPI       E73            NRF24L01+
===============================
SS      10 (22) ---  CSN     4
MOSI    11 (23) ---  MOSI    6
MISO    12 (24) ---  MISO    7
SCK     13 (25) ---  SCK     5
===============================
         9 (20) ---  CE      3
         8 (19) ---  IRQ     8
         
 */

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
  spi_putc(r);
  uint8_t res = spi_putc(0xFF);
  digitalWrite(NRF24_PIN_CSN, HIGH);
  return res;
}

void setup() {

  NRF_SPI0->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M8;  /* SPI frequency = 8MHz  */

  NRF_SPI0->PSEL.MISO = g_ADigitalPinMap[MISO];
  NRF_SPI0->PSEL.MOSI = g_ADigitalPinMap[MOSI];
  NRF_SPI0->PSEL.SCK  = g_ADigitalPinMap[SCK];

  NRF_SPI0->ENABLE = 1;                              /* Enable SPI0           */

  digitalWrite(NRF24_PIN_CSN, HIGH);
  pinMode(NRF24_PIN_CSN, OUTPUT);

  digitalWrite(NRF24_PIN_CE, LOW);
  pinMode(NRF24_PIN_CE, OUTPUT);

  Serial.begin(9600);  /* Initialize serial console      */

}

void loop() {
  Serial.print("NRF24L01+ REGISTERS: ");
  for (uint8_t i = 0; i < 10; i++) {
    Serial.print(" 0x");
    Serial.print(nrf24_read_register(i), HEX);
  }
  Serial.println("");
  delay(5000);
}
