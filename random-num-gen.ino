/* nRF52 RNG -- random number generator    */

#define vector_size 8

typedef uint8_t  r8_arr_t[vector_size];
typedef uint16_t r16_arr_t[vector_size / 2];
typedef uint32_t r32_arr_t[vector_size / 4];

r8_arr_t r8_arr;
const r16_arr_t *r16 = (r16_arr_t *)&r8_arr;
const r32_arr_t *r32 = (r32_arr_t *)&r8_arr;

#define r16_arr (*r16)
#define r32_arr (*r32)

static uint32_t l_count;
char c_buf[120];

void setup() {
  Serial.begin(921600);
  NRF_RNG->CONFIG = 1;                         // Enable Bias correction
}

void loop() {
  NRF_RNG->TASKS_START = 1;                    // start generation
  for (uint16_t i = 0; i < vector_size; i++) { // for each element in vector
    while (NRF_RNG->EVENTS_VALRDY == 0);       // wait till data becomes ready
    NRF_RNG->EVENTS_VALRDY = 0;                // reset data-ready flag
    r8_arr[i] = NRF_RNG->VALUE;                // get new random number (byte)
  }
  NRF_RNG->TASKS_STOP = 1;                     // stop generation

  sprintf(
    c_buf, 
    "RND-%010u: %02x %02x %02x %02x %02x %02x %02x %02x;   %04x %04x %04x %04x;   %08x %08x\n\r", 
    ++l_count, 
    r8_arr[0], r8_arr[1], r8_arr[2], r8_arr[3], 
    r8_arr[4], r8_arr[5], r8_arr[6], r8_arr[7],
    r16_arr[0], r16_arr[1], r16_arr[2], r16_arr[3],
    r32_arr[0], r32_arr[1]
  );
  Serial.print(c_buf);
}
