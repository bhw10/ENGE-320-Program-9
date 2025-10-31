//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "counter.h"
#include "spi.h"
#include "sam.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------
#define DelayTicks(ticks)       {volatile uint32_t n=ticks; while(n--);}//takes 8 cycles
//------------------------------------------------------------------------------
//     ___      __   ___  __   ___  ___  __
//      |  \ / |__) |__  |  \ |__  |__  /__`
//      |   |  |    |___ |__/ |___ |    .__/
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//                __          __        ___  __
//     \  /  /\  |__) |  /\  |__) |    |__  /__`
//      \/  /~~\ |  \ | /~~\ |__) |___ |___ .__/
//
//------------------------------------------------------------------------------
extern uint8_t data_sent;
//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void counter_init()
{
  	// Enable the bus clk for the peripheral
  	PM->APBCMASK.bit.TC3_ = 1;
  	
	// Set the direction on the blank pin
	PORT->Group[0].DIRSET.reg = (1 << 7);
	// Set the direction on the latch pin
	PORT->Group[0].DIRSET.reg = (1 << 14);
	
	
	// Configure the General Clock with the 48MHz clk
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TCC2_TC3) |
	GCLK_CLKCTRL_GEN_GCLK0 |
	GCLK_CLKCTRL_CLKEN;
    // Wait for the GCLK to be synchronized
    while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);
  	
    counter_disable();
	
	TC3->COUNT16.INTENSET.reg = TC_INTENSET_OVF;  // Enable overflow interrupt
	NVIC_EnableIRQ(TC3_IRQn);                     // Enable NVIC for TC3
	// Enable peripheral function E on PA14 (LED_LAT)
	//PORT->Group[0].PINCFG[14].bit.PMUXEN = 1;
	//PORT->Group[0].PMUX[7].bit.PMUXE = PORT_PMUX_PMUXE_E_Val;

	// Put it in the 8-bit mode.
	TC3->COUNT8.CTRLA.bit.MODE = TC_CTRLA_MODE_COUNT8_Val;

    // Set up for normal frequency mode (count to period)	
	TC3->COUNT8.CTRLA.bit.WAVEGEN = TC_CTRLA_WAVEGEN_NFRQ_Val;
	// Could be in MFRQ mode and use the CC reg, but we are in NFRQ mode and using PER
	//TC3->COUNT8.CTRLA.bit.WAVEGEN = TC_CTRLA_WAVEGEN_MFRQ_Val;
	
	// Setup count event inputs
	TC3->COUNT8.EVCTRL.bit.EVACT = TC_EVCTRL_EVACT_COUNT_Val;
	
	// Enable the event input
	TC3->COUNT8.EVCTRL.bit.TCEI = 1;
	
	// Set the Period to be 10 Events - zero-based counting
	counter_set((2 * 4096 - 1));

	// Enable TC3
	//counter_enable();
}

//============================================================================
void counter_set(uint8_t value)
{
	// Set the Period to be value + 1 Events - as we are zero-base counting
	TC3->COUNT8.PER.reg	 = value;
	// If we were in MFRQ mode, do this
	//TC3->COUNT8.CC->reg	 = value;
	while (TC3->COUNT8.STATUS.bit.SYNCBUSY);
}

//============================================================================
void counter_enable()
{
	TC3->COUNT8.CTRLA.bit.ENABLE = 1;
	while (TC3->COUNT8.STATUS.bit.SYNCBUSY);

}

//============================================================================
void counter_disable()
{
	TC3->COUNT8.CTRLA.bit.ENABLE = 0;
	while (TC3->COUNT8.STATUS.bit.SYNCBUSY);
}

//------------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//      __                  __        __        __
//     /  `  /\  |    |    |__)  /\  /  ` |__/ /__`
//     \__, /~~\ |___ |___ |__) /~~\ \__, |  \ .__/
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//        __   __  , __
//     | /__` |__)  /__`   
//     | .__/ |  \  .__/
//
//------------------------------------------------------------------------------
void TC3_Handler(void)
{
	if (TC3->COUNT16.INTFLAG.bit.OVF)
	{
		TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_OVF;  // Clear interrupt flag
		// Blank PA07
		// Latch PA14 if data flag set.
		PORT->Group[0].OUTSET.reg = (1 << 07);
		DelayTicks(75);

		PORT->Group[0].OUTSET.reg = (1 << 14);
		DelayTicks(200);
		PORT->Group[0].OUTCLR.reg = (1 << 14);
		DelayTicks(75);
		PORT->Group[0].OUTCLR.reg = (1 << 07);
		if (data_sent)
		{
			spi_write();
		}
	}
}