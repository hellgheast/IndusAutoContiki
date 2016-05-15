/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/leds.h"
#include "sys/clock.h"
#include "dev/spi.h"
#include "pn532.h"

#define PN532_ADDRESS (0x48)
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS(test_adc, "Test the adc");
AUTOSTART_PROCESSES(&test_adc);
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(test_adc, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();  


	uint8_t status;
	status=SAMConfig();
	if(status){ 
		leds_toggle(LEDS_BLUE);
	}
	//uint32_t versiondata=getFirmwareVersion();
  //printf("Found chip PN532"); 
  //setPassiveActivationRetries(0xFF);
  uint8_t success;                          // Flag to check if there was an error with the PN532
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength=0;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  
	if(readPassiveTargetID(0x00, uid, &uidLength)){
		leds_toggle(LEDS_RED);
		etimer_set(&et, CLOCK_SECOND/20);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	}

	if(success){
		leds_toggle(LEDS_RED);
		uint8_t i;
		printf("\nCard detected uid:");
		for(i=0;i<uidLength;i++){
			printf(" 0x%02x", uid[i]);
		}
	}


  while(1) {
    etimer_set(&et, CLOCK_SECOND/50);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		leds_toggle(LEDS_GREEN);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
