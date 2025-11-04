//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "counter.h"
#include "spi.h"
#include "sam.h"
#include <stdbool.h>

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

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
static volatile uint8_t flag = 0;
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
	
	TC3->COUNT16.CTRLA.bit.SWRST = 1; // reset counter
	while (TC3->COUNT16.CTRLA.bit.SWRST || TC3->COUNT16.STATUS.bit.SYNCBUSY);

	// Put it in the 16-bit mode.
	TC3->COUNT16.CTRLA.bit.MODE = TC_CTRLA_MODE_COUNT16_Val;

	// Set up for normal frequency mode (count to period)
	TC3->COUNT16.CTRLA.bit.WAVEGEN = TC_CTRLA_WAVEGEN_MFRQ_Val;
	TC3->COUNT16.CC[0].reg = 1500;

	// Setup count event inputs
	TC3->COUNT16.EVCTRL.bit.EVACT = TC_EVCTRL_EVACT_COUNT_Val;
	
	// Enable the event input
	TC3->COUNT16.EVCTRL.bit.TCEI = 1;

	TC3->COUNT16.INTENSET.reg = TC_INTENSET_OVF;  // Enable overflow interrupt
	NVIC_EnableIRQ(TC3_IRQn);                     // Enable NVIC for TC3

	//counter_set(4095);
}

//============================================================================
void counter_set(uint16_t value)
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

//============================================================================

uint8_t counter_flagGet()
{
	return flag;
}

//============================================================================

void counter_flagSet(uint8_t value)
{
	__disable_irq();
	flag = value;
	__enable_irq();
}

//============================================================================

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
		// Blank on
		PORT->Group[0].OUTSET.reg = (1 << 07);
		// If write completed, pulse latch
		if (spi_write_completed())
		{
				
			PORT->Group[0].OUTSET.reg = (1 << 14);
			PORT->Group[0].OUTCLR.reg = (1 << 14);
		}
		// Blank off
		PORT->Group[0].OUTCLR.reg = (1 << 07);
		counter_flagSet(1);
	}
}