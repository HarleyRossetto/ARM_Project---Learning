#include "usart.h"

/**
 *  Xplained Pro
 * 
 *  Virtual COM Port
 *  PB10 - SERCOM4 UART TXD PAD 2
 *  PB11 - SERCOM4 UART RXD PAD 3
 * 
 *  8 bit frame
 *  2 bit stop
 *  MSB first
 *  Odd parity
 * 
 * 
 * 	Board Speed	4Mhz
 * 	GENCTRL 0 	4Mhz
 * 	USART Baud	63019 -> 9601 bits per second
 * 
 * 
 */

#define F_CPU 48000000

#define TIMER_PERIOD_FAST 100
#define TIMER_PERIOD_SLOW 500

void timer_set_period(uint16_t period) {
	//Set compare & capture register
	TC3->COUNT16.CC[0].reg = (F_CPU / 1000ul / 1024) * period;
	//Reset counter to 0
	TC3->COUNT16.COUNT.reg = 0;
}

//Timer Counter 3 Interrupt Handler
void TC3_Handler(void) {
	//If interrupt has been triggered toggle LED pin and clear interrupt register.
	if (TC3->COUNT16.INTFLAG.reg &  (TC_INTFLAG_MC_Msk & ((1) << TC_INTFLAG_MC_Pos))) {
		
		//Toggle LED pin
		PORT->Group[1].OUTTGL.reg = (1 << 5);

		//Clear interupts
		TC3->COUNT16.INTFLAG.reg = 0;
	}
}

static void uart_write_character(char c) {
	while (!(SERCOM4->USART.INTFLAG.reg & (_U_(0x1) << SERCOM_USART_INTFLAG_DRE_Pos)));
	SERCOM4->USART.DATA.reg = c;
}

static void uart_write_string(char *s) {
	while (*s)
		uart_write_character(*s++);
}

void usart_init() {
	//Enable SERCOM4 on MCLOCk
    MCLK->APBCMASK.reg |= MCLK_APBCMASK_SERCOM4;

	//Set SERCOM4 to Gen Clk 0 and enable.
    GCLK->PCHCTRL[SERCOM4_GCLK_ID_CORE].reg = (GCLK_PCHCTRL_GEN_GCLK0 | GCLK_PCHCTRL_CHEN);
	
	//Setup Gen Clk 0
    GCLK->GENCTRL[0].reg = (GCLK_GENCTRL_SRC_OSC48M | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_DIV(1));

	//IO Pin Mux
	REG_PORT_DIRSET1 |= PORT_PB10;
	//USART TX Pin Mux
	PORT->Group[1].PINCFG[10].reg |= PORT_PINCFG_PMUXEN | PORT_PINCFG_INEN;
	PORT->Group[1].PMUX[10 >> 1].reg |= PORT_PMUX_PMUXE(PINMUX_PB10D_SERCOM4_PAD2);
	//USART RX Pin Mux
	PORT->Group[1].PINCFG[11].reg |= PORT_PINCFG_PMUXEN | PORT_PINCFG_INEN;
	PORT->Group[1].PMUX[11 >> 1].reg |= PORT_PMUX_PMUXO(PINMUX_PB11D_SERCOM4_PAD3);


    //Setup SERCOM4
	//Ensure USART is disabled, awaiting syncronisation.
	SERCOM4->USART.CTRLA.bit.ENABLE = 0;
	while (SERCOM4->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_ENABLE);

	//Setup USART CTRLA register and await sync.
    SERCOM4->USART.CTRLA.reg = (
                                SERCOM_USART_CTRLA_MODE(0x1)    | 	//USART with internal clock
                                SERCOM_USART_CTRLA_FORM(0x0)    |	//USART frame
                                SERCOM_USART_CTRLA_TXPO(0x1)    |	//TX Pad 2
                                SERCOM_USART_CTRLA_RXPO(0x3)	);	//RX Pad 3

	while (SERCOM4->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_ENABLE);

	SERCOM4->USART.CTRLB.reg = (
								SERCOM_USART_CTRLB_RXEN		|	//Enable RX
								SERCOM_USART_CTRLB_TXEN		);	//Enable TX

	while (SERCOM4->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_CTRLB);

	uint32_t baud = 9600;
	uint64_t baudrate = (uint64_t)65536 * (F_CPU - 16 * baud) / F_CPU;

	SERCOM4->USART.BAUD.reg = baudrate;

	//Enable TX & Receive Interrupt
	SERCOM4->USART.INTENSET.reg |= (SERCOM_USART_INTFLAG_RXC);

	//Enable USART and await sync.
	SERCOM4->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;
	while (SERCOM4->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_ENABLE);


	NVIC_SetPriority(SERCOM4_IRQn, 3);
	NVIC_EnableIRQ(SERCOM4_IRQn);
}


void SERCOM4_Handler(void) {
	//If Receive Completed
	if (SERCOM4->USART.INTFLAG.bit.RXC == 1) {
		//Read character
		uint16_t inputMessage = SERCOM4->USART.DATA.bit.DATA;
		//Increase value by 1
		inputMessage++;

		//While Data Register is not empty
		while (SERCOM4->USART.INTFLAG.bit.DRE == 0);
		//Return message back.
		SERCOM4->USART.DATA.bit.DATA = inputMessage;

		//Clear Interrupt Flag
		SERCOM4->USART.INTFLAG.reg |= 0b00000100;
	}
	//If Transmit Completed
	if (SERCOM4->USART.INTFLAG.bit.TXC == 1) {

		//Clear Interrupt Flag
		SERCOM4->USART.INTFLAG.reg |= 0b00000010;
	}
}

void system_initialise() {
	//Set flash wait state to maximum for 48 Mhz Operation
	NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_RWS(2) | NVMCTRL_CTRLB_MANW;

	//Switch to 48Mhz clock (disable prescaler)
	OSCCTRL->OSC48MDIV.reg = OSCCTRL_OSC48MDIV_DIV(0);
	while (OSCCTRL->OSC48MSYNCBUSY.reg);
}


void run() {
	uint32_t count = 0;
	bool fastMode = false;
	
	//Set-up
	system_initialise();
	usart_init();

	uart_write_string("\rHello, World!\r\n");

	//Setup button for input and pullup enable
	PORT->Group[1].DIRCLR.reg = (1 << 19);
	PORT->Group[1].PINCFG[19].reg |= PORT_PINCFG_INEN;
	PORT->Group[1].PINCFG[19].reg &= ~PORT_PINCFG_INEN;

	PORT->Group[1].OUTSET.reg = (1 << 19);
	PORT->Group[1].PINCFG[19].reg |= PORT_PINCFG_PULLEN;

	//Setup LED for output
	PORT->Group[2].DIRSET.reg = (1 << 5);
	PORT->Group[2].PINCFG[5].reg |= PORT_PINCFG_INEN;

	//App Loop
	while (1) {
		//If button is pressed.
		if ((PORT->Group[1].IN.reg & (1 << 19)) != 0)
			count = 0;
		else if (count < 5000)
			count++;

		if (count == 5000) {
			fastMode = !fastMode;
			timer_set_period(fastMode ? TIMER_PERIOD_FAST : TIMER_PERIOD_SLOW);
			uart_write_character('.');
		}
	}
}