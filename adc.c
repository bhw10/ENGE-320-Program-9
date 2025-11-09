//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "adc.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define Y (0)
#define X (1)

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
static volatile uint8_t dir = Y;
static volatile uint16_t y_val = 0;
static volatile uint16_t x_val = 0;
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
void adc_init()
{
	// enable ADC
	PM->APBCMASK.bit.ADC_ = 1;
	
	// Enable gen clk
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_ADC |
	GCLK_CLKCTRL_GEN_GCLK0 |
	GCLK_CLKCTRL_CLKEN;
	while (GCLK->STATUS.bit.SYNCBUSY);
	
	// Reset ADC
	ADC->CTRLA.bit.SWRST = 1;
	while (ADC->CTRLA.bit.SWRST || ADC->STATUS.bit.SYNCBUSY);
	
	// Disable run standby and ADC enable
	ADC->CTRLA.bit.RUNSTDBY = 0;
	ADC->CTRLA.bit.ENABLE = 0;
	
	// Select reference voltage
	ADC->REFCTRL.bit.REFSEL = 0x02; // 0.5*VDDANA
	
	// Select average ctrl
	ADC->AVGCTRL.bit.ADJRES = 0;
	ADC->AVGCTRL.bit.SAMPLENUM = 0;
	
	// Select sample ctrl
	ADC->SAMPCTRL.bit.SAMPLEN = 0;
	
	// Set prescaler, resolution, freerun mode, and gain correction
	ADC->CTRLB.bit.PRESCALER = 0x6; // prescaler of 256
	ADC->CTRLB.bit.RESSEL = 0x00; // 12-bit resolution
	ADC->CTRLB.bit.FREERUN = 1;
	ADC->CTRLB.bit.CORREN = 0;
	while (ADC->STATUS.bit.SYNCBUSY);
	
	// Set gain and mux selection
	ADC->INPUTCTRL.bit.GAIN = 0xF; // 0.5 * gain
	ADC->INPUTCTRL.bit.MUXNEG = 0x18; // GND
	ADC->INPUTCTRL.bit.MUXPOS = 0x02; // Joystick (y-direction)
	while (ADC->STATUS.bit.SYNCBUSY);
	
	// Enable interrupts
	NVIC_EnableIRQ(ADC_IRQn);
	
	// Enable ADC
	ADC->CTRLA.bit.ENABLE = 1;
	
	// Start first conversion
	ADC->SWTRIG.bit.START = 1;
	while (ADC->STATUS.bit.SYNCBUSY);
	
}

//==============================================================================
uint16_t adc_get()
{
	while (ADC->STATUS.bit.SYNCBUSY);
	return ADC->RESULT.bit.RESULT;
}

//==============================================================================

void adc_reset()
{
	ADC->CTRLB.bit.FREERUN = 1; // enable freerun mode
	ADC->INTENCLR.bit.RESRDY = 1; // disable interrupt
	ADC->INPUTCTRL.bit.MUXPOS = 0x02; // select joystick y dir
}

//==============================================================================

void adc_interruptSet()
{
	ADC->CTRLB.bit.FREERUN = 0; // disable freerun mode
	ADC->INTENSET.bit.RESRDY = 1; // enable interrupts
	dir = Y;
	ADC->INPUTCTRL.bit.MUXPOS = 0x02; // select joystick y dir
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
void ADC_Handler(void)
{
	if (ADC->INTFLAG.bit.RESRDY)
	{
		ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY; // clear flag
		if (dir == Y)
		{
			y_val = ADC->RESULT.reg; // store result
			ADC->INPUTCTRL.bit.MUXPOS = 0x00; // Set ADC to read x dir
			dir = X; // switch state
		}
		else if (dir == X)
		{
			x_val = ADC->RESULT.reg; // store result
			ADC->INPUTCTRL.bit.MUXPOS = 0x02; // Set ADC to read y dir
			dir = Y; // switch state
		}
		ADC->SWTRIG.bit.START = 1; // Start next conversion
	}
}