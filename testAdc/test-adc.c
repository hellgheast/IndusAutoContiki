#include <stdio.h>
#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "dev/z1-phidgets.h"

/*---------------------------------------------------------------------------*/
PROCESS(test_adc, "Test the adc");
AUTOSTART_PROCESSES(&test_adc);
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(test_adc, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();
  SENSORS_ACTIVATE(phidgets);
	double indcutance;
	double weight;
	double capacity;

  while(1) {
    etimer_set(&et, CLOCK_SECOND/10);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    leds_toggle(LEDS_GREEN);

		// Convertion capteur	
		indcutance=phidgets.value(PHIDGET3V_1)/4;
		weight=phidgets.value(PHIDGET5V_1)/4096*11;
		capacity=phidgets.value(PHIDGET3V_2)/4;
		

    printf("Inductance :%f mH\n", indcutance);
		printf("capacity :%f nF\n", capacity);
		printf("weight : %f kg\n",weight);

		// Actionneur
    if (weight < 3.) {
      leds_on(LEDS_RED);
    } else {
      leds_off(LEDS_RED);
    }

  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
