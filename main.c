#include "sam.h"
#include "timer.h"
#include "counter.h"
#include "event.h"
#include "spi.h"
#include "led.h"

uint8_t toggle = 0;
static volatile uint32_t millis;
uint8_t data_sent = 1;
uint8_t q = 1;


int main(void)
{

	/* Initialize the SAM system */
	SystemInit();
	SysTick_Config(48000); //  Configure the SysTick timer for a ms

	// Init the timer and the counter
	timer_init();
	timer_set_period(1000);
	counter_init();
	spi_init();

	//counter_set(9);
	
	// Set up the events
	event_init();
	
	// Turn on the timer and the counter
    timer_enable();
	counter_enable();

	
	// Configure the Arduino LED
	REG_PORT_DIR0 |= PORT_PA17;
	REG_PORT_OUTSET0 = PORT_PA17;
	
	while (1)
	{

		if (millis > 299)
		{

			led_write(q,0,0,0x0fff);
			led_write((q-1), 0, 0, 0);
			q++;
			if(q>5)q=1;
			if(q==2)led_write((5), 0, 0, 0);
			

			REG_PORT_OUTTGL0 = PORT_PA17;
			__disable_irq();
			millis = 0;
			__enable_irq();
			
		}
	}
}


void SysTick_Handler ()
{
	millis++;
}

