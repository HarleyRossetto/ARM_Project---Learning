#include "led_button.h"

void run() {
    REG_PORT_DIRCLR1 = BTN; //Clear port direction on buttons pin.
	REG_PORT_DIRSET2 = LED; //Set port direction to output on LED pin.

   //Button - input enable with pullup
	REG_PORT_OUT1 |= BTN;
	PORT->Group[1].PINCFG[19].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;

	while (1)
   	{
		if (REG_PORT_IN1 & (BTN)) { //Button off
	   		REG_PORT_OUT2 &= ~(LED);   //LED on
	   	} else {
	   		REG_PORT_OUT2 |= LED;   //LED Off
      	}
   	}

}