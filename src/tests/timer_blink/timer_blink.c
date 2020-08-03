#include "timer_blink.h"

#define PORTA 0
#define PORTB 1
#define PORTC 2

void run() {
	//Setup
	//MCLK
	{
		MCLK->CPUDIV.reg = MCLK_CPUDIV_CPUDIV(MCLK_CPUDIV_CPUDIV_DIV1);
	}
	//PWM Setup
	{
		//PWM CLK Init
		{
			//Setup GClock
			GCLK->GENCTRL[0].reg = GCLK_GENCTRL_SRC_OSC48M | GCLK_GENCTRL_GENEN; //| GCLK_GENCTRL_DIVSEL | GCLK_GENCTRL_DIV(2)
			//Write to peripheral control register
			GCLK->PCHCTRL[TCC2_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
			//Set MCLK APBC Mask TCC2 bit
			MCLK->APBCMASK.reg |= MCLK_APBCMASK_TCC2;
		}
		//GPIO Pin Mux
		{
			//Clear Pullup enable?
			//PORT->Group[2].PINCFG[5].reg &= ~PORT_PINCFG_PULLEN;
			//Enable Pin Mux
			PORT->Group[2].PINCFG[5].reg |= 1 << PORT_PINCFG_PMUXEN_Pos;
			//Pin Mux Function Set
			PORT->Group[2].PMUX[5 >> 1].reg |= PORT_PMUX_PMUXO(PINMUX_PC05F_TCC2_WO1);
		}
		//PWM Init
		{
			//Enabled TCC2 and set prescaler
			TCC2->CTRLA.reg = TCC_CTRLA_ENABLE | TCC_CTRLA_PRESCALER_DIV256;
			//Lock update
			TCC2->CTRLBSET.reg = TCC_CTRLBCLR_LUPD;
			//Waveform
			TCC2->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM_Val;
			while (TCC2->SYNCBUSY.reg & TCC_SYNCBUSY_MASK) ;
			//Write Period
			TCC2->PER.reg = 0x1388;
			while (TCC2->SYNCBUSY.reg & TCC_SYNCBUSY_PER) ;
			//Invert Output
			TCC2->DRVCTRL.bit.INVEN1 = 1;
			//Write Duty Cycle
			TCC2->CC[1].reg = 0x9C4;
			while (TCC2->SYNCBUSY.reg & (TCC_SYNCBUSY_CC0 | TCC_SYNCBUSY_CC1 | TCC_SYNCBUSY_CC2 | TCC_SYNCBUSY_CC3)) ;
			//Clear lock bit
			TCC2->CTRLBCLR.reg = TCC_CTRLBCLR_LUPD;
		}

	}

	REG_PORT_DIRSET2 = PORT_PC05;

	/**
	 * 	Processor Running 4 Mhz
	 * 	MCLK Divider 1
	 * 	GCLK Gen 0 Divider 1
	 * 	TCC2 Prescaler Divider 1024
	 * 	
	 * 
	 * 	fPwm_ss = fGCLK_TCC / (N * (TOP+1))
	 * 
	 * 	N = prescale divider (1 thru 1024)
	 * 
	 * 	TOP = Period
	 * 
	 */


	//Loop
	while (1) {

	}
}

void holdthis() {
//	MUX Pins for TCC2/WO[1] output
   	PORT->Group[PORTC].DIRSET.reg |= PORT_PC05;
	// Enable Peripheral Mux
   	PORT->Group[PORTC].PINCFG[MUX_PC05F_TCC2_WO1].bit.PMUXEN = 1;
	//	Mux PC05 to TCC2 peripheral channel (F)
	PORT->Group[PORTC].PMUX[MUX_PC05F_TCC2_WO1 >> 1].bit.PMUXO = 0x6; 

	//Set TCC Peripheral Clock generator source to GenCtrl0
	GCLK->PCHCTRL[TCC2_GCLK_ID].bit.GEN = 0;
	//Enable TCC2 peripheral channel
	GCLK->PCHCTRL[TCC2_GCLK_ID].bit.CHEN = 1;
	//Await TCC2 peripheral channel enable sync. Not sure if required.
	while (!GCLK->PCHCTRL[TCC2_GCLK_ID].bit.CHEN);
	//Enable the generator, unclear if syncronisation is required?
	GCLK->GENCTRL[0].reg = GCLK_GENCTRL_GENEN;
	while (!GCLK->SYNCBUSY.bit.GENCTRL0); //Wait for sync. Not sure if required.

	// Enable TCC2 from main clock
	MCLK->APBCMASK.bit.TCC2_ = 1;

	//Set prescaler, divide by 256
	//TCC2->CTRLA.bit.PRESCALER = TCC_CTRLA_PRESCALER_DIV256_Val;
	//Normal Waveform Generation Mode
	TCC2->WAVE.bit.WAVEGEN = TCC_WAVE_WAVEGEN_NPWM_Val;
	TCC2->PER.bit.PER = 0xFFFF;
	//Enable the timer
	TCC2->CTRLA.bit.ENABLE = 1;
	while (TCC2->SYNCBUSY.bit.ENABLE); //Wait for sync
	
	//Set Compare Value
	TCC2->CC[1].bit.CC = 200;

	//Button Utilities
	// REG_PORT_DIRCLR1 = PORT_PB19;
	// REG_PORT_OUT1 |= PORT_PB19;
	// PORT->Group[1].PINCFG[19].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;

    while (1) {
		// if (REG_PORT_IN1 & PORT_PB19)
		// 	TCC2->CC[1].bit.CC = 100;
		// else
		// 	TCC2->CC[1].bit.CC = 200;
	}
}