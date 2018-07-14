/* NRF52 INFO  */

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println("=======================================");
  Serial.print("Chip\t\t: nRF");
  Serial.print(NRF_FICR->INFO.PART, HEX);
  switch (NRF_FICR->INFO.VARIANT) {
    case 0x41414141: Serial.println(" AAAA"); break;
    case 0x41414142: Serial.println(" AAAB"); break;
    case 0x41414241: Serial.println(" AABA"); break;
    case 0x41414242: Serial.println(" AABB"); break;
    case 0x41414230: Serial.println(" AAB0"); break;
    case 0x41414530: Serial.println(" AAE0"); break;
  }

  Serial.print("Package\t\t: ");
  switch (NRF_FICR->INFO.PACKAGE) {
    case 0x2000: Serial.println("QFxx - 48-pin QFN"); break;
    case 0x2001: Serial.println("CHxx - 7x8 WLCSP 56 balls"); break;
    case 0x2002: Serial.println("CIxx - 7x8 WLCSP 56 balls"); break;
    case 0x2005: Serial.println("CKxx - 7x8 WLCSP 56 balls with backside coating"); break;
  }
  
  Serial.print("RAM size\t: ");
  switch (NRF_FICR->INFO.RAM) {
    case 0x10: Serial.println("16 kByte"); break;
    case 0x20: Serial.println("32 kByte"); break;
    case 0x40: Serial.println("64 kByte"); break;
  }
  Serial.print("Flash size\t: ");
  switch (NRF_FICR->INFO.FLASH) {
    case 0x80:  Serial.println("128 kByte"); break;
    case 0x100: Serial.println("256 kByte"); break;
    case 0x200: Serial.println("512 kByte"); break;
  }
  while(1); // halt
}
