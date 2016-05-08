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

  while(1) {
    etimer_set(&et, CLOCK_SECOND/10);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    leds_toggle(LEDS_GREEN);
    printf("Potentiometre1:%d\n", phidgets.value(PHIDGET3V_2));

    if (phidgets.value(PHIDGET3V_2) < 300) {
      leds_on(LEDS_RED);
    } else {
      leds_off(LEDS_RED);
    }

  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
