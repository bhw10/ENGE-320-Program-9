//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "sam.h"
#include "timer.h"
#include "counter.h"
#include "event.h"
#include "spi.h"
#include "led.h"
#include "buttons.h"
#include "adc.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define RED (0)
#define YELLOW (1)
#define GREEN (2)
#define CYAN (3)
#define BLUE (4)
#define MAGENTA (5)

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

static volatile uint32_t millis;

// LED values
const uint16_t fade_up[360] = {
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,
	1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,
	4,   4,   4,   4,   5,   5,   5,   6,   6,   7,   7,   7,   8,   8,   9,   9,
	10,  10,  11,  11,  12,  13,  13,  14,  15,  16,  16,  17,  18,  19,  20,  20,
	21,  22,  23,  24,  25,  26,  28,  29,  30,  31,  32,  34,  35,  36,  38,  39,
	40,  42,  44,  45,  47,  48,  50,  52,  54,  55,  57,  59,  61,  63,  65,  67,
	69,  72,  74,  76,  79,  81,  83,  86,  88,  91,  94,  96,  99, 102, 105, 108,
	111, 114, 117, 120, 123, 127, 130, 134, 137, 141, 144, 148, 152, 155, 159, 163,
	167, 171, 176, 180, 184, 189, 193, 198, 202, 207, 212, 217, 221, 227, 232, 237,
	242, 247, 253, 258, 264, 270, 275, 281, 287, 293, 299, 305, 312, 318, 325, 331,
	338, 345, 351, 358, 365, 373, 380, 387, 395, 402, 410, 418, 426, 434, 442, 450,
	458, 467, 475, 484, 492, 501, 510, 519, 528, 538, 547, 557, 566, 576, 586, 596,
	606, 617, 627, 637, 648, 659, 670, 681, 692, 703, 715, 726, 738, 750, 761, 774,
	786, 798, 811, 823, 836, 849, 862, 875, 888, 902, 916, 929, 943, 957, 971, 986,
	1000,1015,1030,1045,1060,1075,1091,1106,1122,1138,1154,1170,1187,1203,1220,1237,
	1254,1271,1289,1306,1324,1342,1360,1378,1396,1415,1434,1453,1472,1491,1511,1530,
	1550,1570,1591,1611,1632,1652,1673,1694,1716,1737,1759,1781,1803,1825,1848,1871,
	1894,1917,1940,1964,1987,2011,2035,2060,2084,2109,2134,2159,2184,2210,2236,2262,
	2288,2315,2341,2368,2395,2423,2450,2478,2506,2534,2563,2591,2620,2649,2679,2708,
	2738,2768,2798,2829,2860,2891,2922,2954,2985,3017,3049,3082,3115,3148,3181,3214,
	3248,3282,3316,3351,3385,3420,3456,3491,3527,3563,3599,3636,3672,3709,3747,3784,
	3822,3860,3899,3938,3976,4016,4055,4095};
	
const uint16_t fade_down[360] = {
 4095,4055,4016,3976,3938,3899,3860,3822,3784,3747,3709,3672,3636,3599,3563,3527,
 3491,3456,3420,3385,3351,3316,3282,3248,3214,3181,3148,3115,3082,3049,3017,2985,
 2954,2922,2891,2860,2829,2798,2768,2738,2708,2679,2649,2620,2591,2563,2534,2506,
 2478,2450,2423,2395,2368,2341,2315,2288,2262,2236,2210,2184,2159,2134,2109,2084,
 2060,2035,2011,1987,1964,1940,1917,1894,1871,1848,1825,1803,1781,1759,1737,1716,
 1694,1673,1652,1632,1611,1591,1570,1550,1530,1511,1491,1472,1453,1434,1415,1396,
 1378,1360,1342,1324,1306,1289,1271,1254,1237,1220,1203,1187,1170,1154,1138,1122,
 1106,1091,1075,1060,1045,1030,1015,1000,986,971,957,943,929,916,902,888,875,862,
 849,836,823,811,798,786,774,761,750,738,726,715,703,692,681,670,659,648,637,627,
 617,606,596,586,576,566,557,547,538,528,519,510,501,492,484,475,467,458,450,442,
 434,426,418,410,402,395,387,380,373,365,358,351,345,338,331,325,318,312,305,299,
 293,287,281,275,270,264,258,253,247,242,237,232,227,221,217,212,207,202,198,193,
 189,184,180,176,171,167,163,159,155,152,148,144,141,137,134,130,127,123,120,117,
 114,111,108,105,102,99,96,94,91,88,86,83,81,79,76,74,72,69,67,65,63,61,
 59,57,55,54,52,50,48,47,45,44,42,40,39,38,36,35,34,32,31,30,
 29,28,26,25,24,23,22,21,20,20,19,18,17,16,16,15,14,13,13,12,
 11,11,10,10,9,9,8,8,7,7,7,6,6,5,5,5,4,4,4,4,
 3,3,3,3,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,
 1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0};

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

static uint8_t calculate_adc();

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

int main(void)
{
	uint8_t color = RED;
	uint8_t mode = 0;
	uint16_t index = 0;
	uint16_t red, green, blue, next_red, next_green, next_blue;
	uint8_t step;
	uint32_t old_millis = 0;
	uint8_t led = 1;
	uint8_t state = 0;
	uint8_t next_led = 2;
	uint8_t led_state = 0;

	// Initialize the SAM system 
	SystemInit();
	SysTick_Config(48000); //  Configure the SysTick timer for 1 ms

	// Initialize drivers
	led_init();
	timer_init();
	counter_init();
	spi_init();
	event_init();
	buttons_init();
	adc_init();
	
	// Turn on the timer and the counter
    timer_enable();
	counter_enable();
	led_writeAll(0, 0, 0);
	spi_write();

	while (1)
	{	
		while (mode == 0)
		{
			if ((millis - old_millis) > 20) // button debounce
			{
				if (buttons_get(1)) // if button 1 pressed
				{
					old_millis = millis;
					mode = 1; // go to mode 1
					led_writeAll(0, 0, 0); // turn off LEDs
					spi_write();
					index = 0;
					state = 0;
					led = 1;
					next_led = 2;
					led_state = 0;
				}
			}
			if (counter_flagGet()) // if flag set
			{
				counter_flagSet(0); // clear flag
				step = calculate_adc();
				switch (color)
				{
					case RED: // fade to yellow
						red = 4095;
						green = fade_up[index];
						blue = 0;
						break;
					
					case YELLOW: // fade to green
						red = fade_down[index];
						green = 4095;
						blue = 0;
						break;
					
					case GREEN: // fade to cyan
						red = 0;
						green = 4095;
						blue = fade_up[index];
						break;
					
					case CYAN:
						red = 0; // fade to blue
						green = fade_down[index];
						blue = 4095;
						break;
					
					case BLUE: // fade to magenta
						red = fade_up[index];
						green = 0;
						blue = 4095;
						break;
					
					case MAGENTA: // fade to red
						red = 4095;
						green = 0;
						blue = fade_down[index];
						break;
					
					default:
						red = 0;
						green = 0;
						blue = 0;
						break;
				}
				led_writeAll(red, green, blue);
				spi_write(); // send first spi write
				index += step; // step thru table
				if (index >= 360) // if table has been stepped through completely
				{
					index = 0; // reset index
					color += 1; // increment color state
					if (color > 5) // after magenta, go to red
					{
						color = RED;
					}
				}
			}
		}
		
		while (mode == 1)
		{
			if ((millis - old_millis) > 20) // button debounce
			{
				if (buttons_get(0)) // if button 0 pressed
				{
					old_millis = millis;
					mode = 0; // go to mode 0
					color = RED;
					led_writeAll(0, 0, 0); // turn off LEDs
					spi_write();
					index = 0;
				}
			}
			if (counter_flagGet()) // if flag set
			{
				counter_flagSet(0); // clear flag
				step = calculate_adc();
				switch (state)
				{
					case 0: // off to blue
					//led_write(led, 0, 0, fade_up[index]);
					red = 0;
					green = 0;
					blue = fade_up[index];
					next_red = 0;
					next_green = 0;
					next_blue = 0;
					break;
					
					case 1: // blue to magenta
					//led_write(led, fade_up[index], 0, 4095);
					red = fade_up[index];
					green = 0;
					blue = 4095;
					next_red = 0;
					next_green = 0;
					next_blue = 0;
					break;
					
					case 2: // magenta to red
					//led_write(led, 4095, 0, fade_down[index]);
					red = 4095;
					green = 0;
					blue = fade_down[index];
					next_red = 0;
					next_green = 0;
					next_blue = 0;
					break;
					
					case 3: // red to yellow
					//led_write(led, 4095, fade_up[index], 0);
					red = 4095;
					green = fade_up[index];
					blue = 0;
					next_red = 0;
					next_green = 0;
					next_blue = 0;
					break;
					
					case 4: // yellow to green
					//led_write(led, fade_down[index], 4095, 0);
					red = fade_down[index];
					green = 4095;
					blue = 0;
					next_red = 0;
					next_green = 0;
					next_blue = 0;
					break;
					
					case 5: // green to cyan
					//led_write(led, 0, 4095, fade_up[index]);
					red = 0;
					green = 4095;
					blue = fade_up[index];
					next_red = 0;
					next_green = 0;
					next_blue = 0;
					break;
					
					case 6:
					//led_write(next_led, 0, 0, fade_up[index]); // off to blue for next LED
					//led_write(led, 0, fade_down[index], 4095); // cyan to blue
					red = 0;
					green = fade_down[index];
					blue = 4095;
					next_red = 0;
					next_green = 0;
					next_blue = fade_up[index];
					break;

					case 7:
					//led_write(next_led, fade_up[index], 0, 4095); // blue to magenta for next LED
					//led_write(led, 0, 0, fade_down[index]); // blue to off
					red = 0;
					green = 0;
					blue = fade_down[index];
					if (blue == 1) // normalize blue at high step size
					{
						blue = 0;
					}
					next_red = fade_up[index];
					next_green = 0;
					next_blue = 4095;
					break;
				}
				led_write(led, red, green, blue);
				led_write(next_led, next_red, next_green, next_blue);
				spi_write(); // send spi write
				index += step; // step thru table
				if (index >= 360) // if table has been stepped through completely
				{
					index = 0; // reset index
					state += 1; // increment color state
					if (state > 7) // at end of state machine
					{
						state = 2; // go to state 2 (first 2 states covered in case 6 and 7 for next led)

						switch (led_state) // to get LEDs in figure-eight pattern
						{
							case 0:
							led = 2;
							next_led = 5;
							break;

							case 1:
							led = 5;
							next_led = 4;
							break;

							case 2:
							led = 4;
							next_led = 3;
							break;

							case 3:
							led = 3;
							next_led = 5;
							break;

							case 4:
							led = 5;
							next_led = 1;
							break;

							case 5:
							led = 1;
							next_led = 2;
							break;
						}
						led_state++;
						led_state %= 6;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//-----------------------------------------------------------------------------

static uint8_t calculate_adc()
{
	uint8_t step;
	if (adc_get() < 10) // joystick at max up
	{
		step = 36;	
	}
	else if (adc_get() > 245) // joystick at max down
	{
		step = 1; 
	}
	else if (adc_get() > 140) // joystick down
	{
		step = ((adc_get() - 128) / 128) * 30;
	}
	else if (adc_get() < 116) // joystick up
	{
		step = ((128 - adc_get()) / 128) * 5;
	}
	else // joystick neutral
	{
		step = 6;
	}
	return step;
}

//-----------------------------------------------------------------------------
//        __   __   __
//     | /__` |__) /__`
//     | .__/ |  \ .__/
//
//-----------------------------------------------------------------------------
void SysTick_Handler ()
{
	millis++;
}