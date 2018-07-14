/* nRF52 -- temperature read  */

/*
* Workaround for PAN_028 rev2.0A anomalies 28, 29,30 and 31. PAN 43 is not covered.
*  - PAN_028 rev2.0A anomaly 28 - TEMP: Negative measured values are not represented correctly
*  - PAN_028 rev2.0A anomaly 29 - TEMP: Stop task clears the TEMP register.   
*  - PAN_028 rev2.0A anomaly 30 - TEMP: Temp module analog front end does not power down when DATARDY event occurs.
*  - PAN_028 rev2.0A anomaly 31 - TEMP: Temperature offset value has to be manually loaded to the TEMP module
*  - PAN_028 rev2.0A anomaly 43 - TEMP: Using PPI between DATARDY event and START task is not functional. 
*/

#define MASK_SIGN           (0x00000200UL)
#define MASK_SIGN_EXTENSION (0xFFFFFC00UL)

/**
 * @brief Function for reading temperature measurement.
 *
 * The function reads the 10 bit 2's complement value and transforms it to a 32 bit 2's complement value.
 */
static __INLINE int32_t nrf_temp_read(void)
{    
    /**@note Workaround for PAN_028 rev2.0A anomaly 28 - TEMP: Negative measured values are not represented correctly */
    return ((NRF_TEMP->TEMP & MASK_SIGN) != 0) ? (NRF_TEMP->TEMP | MASK_SIGN_EXTENSION) : (NRF_TEMP->TEMP);    
}

void setup() {
  Serial.begin(115200);

  /**@note Workaround for PAN_028 rev2.0A anomaly 31 - TEMP: Temperature offset value has to be manually loaded to the TEMP module */
  *(uint32_t *) 0x4000C504 = 0;
}

void loop() {
  
  NRF_TEMP->TASKS_START = 1; /** Start the temperature measurement. */

  /* Busy wait while temperature measurement is not finished*/
  while (NRF_TEMP->EVENTS_DATARDY == 0);
  NRF_TEMP->EVENTS_DATARDY = 0;

  /**@note Workaround for PAN_028 rev2.0A anomaly 29 - TEMP: Stop task clears the TEMP register. */
  double temp = nrf_temp_read() * 0.25;

  /**@note Workaround for PAN_028 rev2.0A anomaly 30 - TEMP: Temp module analog front end does not power down when DATARDY event occurs. */
  NRF_TEMP->TASKS_STOP = 1; /** Stop the temperature measurement. */
  
   Serial.print("Actual temperature: ");
   Serial.println(temp);

  delay(5000);
  
}
