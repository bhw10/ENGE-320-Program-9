#include "sam.h"
#include "timer.h"
#include "counter.h"
#include "event.h"
#include "spi.h"
#include "led.h"
#include "buttons.h"

#define RED (0)
#define YELLOW (1)
#define GREEN (2)
#define CYAN (3)
#define BLUE (4)
#define MAGENTA (5)
#define OFF (6)

static volatile uint32_t millis;

// LED values
const uint16_t fade_up[360] = {
	  0,   0,   0,   0,   1,   1,   1,   2,   2,   3,   3,   4,   5,   5,   6,   7,
     8,   9,  10,  11,  13,  14,  15,  17,  18,  20,  21,  23,  25,  27,  29,  31,
    33,  35,  37,  39,  41,  43,  46,  48,  51,  53,  56,  59,  62,  64,  67,  70,
    73,  76,  79,  83,  86,  89,  93,  96, 100, 103, 107, 111, 114, 118, 122, 126,
   130, 134, 138, 143, 147, 151, 156, 160, 165, 169, 174, 179, 184, 188, 193, 198,
   203, 208, 214, 219, 224, 230, 235, 240, 246, 252, 257, 263, 269, 275, 281, 287,
   293, 299, 305, 311, 318, 324, 331, 337, 344, 350, 357, 364, 371, 378, 384, 391,
   399, 406, 413, 420, 428, 435, 442, 450, 458, 465, 473, 481, 489, 496, 504, 512,
   521, 529, 537, 545, 554, 562, 571, 579, 588, 596, 605, 614, 623, 632, 641, 650,
   659, 668, 677, 687, 696, 705, 715, 724, 734, 744, 754, 763, 773, 783, 793, 803,
   813, 824, 834, 844, 855, 865, 876, 886, 897, 907, 918, 929, 940, 951, 962, 973,
   984, 995,1007,1018,1029,1041,1052,1064,1076,1087,1099,1111,1123,1135,1147,1159,
  1171,1184,1196,1208,1221,1233,1246,1258,1271,1284,1296,1309,1322,1335,1348,1361,
  1375,1388,1401,1415,1428,1442,1455,1469,1482,1496,1510,1524,1538,1552,1566,1580,
  1594,1609,1623,1637,1652,1666,1681,1695,1710,1725,1740,1755,1770,1785,1800,1815,
  1830,1845,1861,1876,1892,1907,1923,1938,1954,1970,1986,2002,2018,2034,2050,2066,
  2082,2099,2115,2131,2148,2164,2181,2198,2214,2231,2248,2265,2282,2299,2316,2333,
  2351,2368,2385,2403,2420,2438,2456,2473,2491,2509,2527,2545,2563,2581,2599,2617,
  2635,2654,2672,2691,2709,2728,2746,2765,2784,2803,2822,2841,2860,2879,2898,2917,
  2936,2956,2975,2995,3014,3034,3053,3073,3093,3113,3133,3153,3173,3193,3213,3233,
  3254,3274,3294,3315,3335,3356,3377,3398,3418,3439,3460,3481,3502,3523,3545,3566,
  3587,3608,3630,3651,3673,3695,3716,3738,3760,3782,3804,3826,3848,3870,3892,3915,
  3937,3959,3982,4004,4027,4050,4072,4095};
	
const uint16_t fade_down[360] = {
	4095, 4072, 4050, 4027, 4004, 3982, 3959, 3937, 3915, 3892, 3870, 3848, 3826, 3804, 3782, 3760,
  3738, 3716, 3695, 3673, 3651, 3630, 3608, 3587, 3566, 3545, 3523, 3502, 3481, 3460, 3439, 3418,
  3398, 3377, 3356, 3335, 3315, 3294, 3274, 3254, 3233, 3213, 3193, 3173, 3153, 3133, 3113, 3093,
  3073, 3053, 3034, 3014, 2995, 2975, 2956, 2936, 2917, 2898, 2879, 2860, 2841, 2822, 2803, 2784,
  2765, 2746, 2728, 2709, 2691, 2672, 2654, 2635, 2617, 2599, 2581, 2563, 2545, 2527, 2509, 2491,
  2473, 2456, 2438, 2420, 2403, 2385, 2368, 2351, 2333, 2316, 2299, 2282, 2265, 2248, 2231, 2214,
  2198, 2181, 2164, 2148, 2131, 2115, 2099, 2082, 2066, 2050, 2034, 2018, 2002, 1986, 1970, 1954,
  1938, 1923, 1907, 1892, 1876, 1861, 1845, 1830, 1815, 1800, 1785, 1770, 1755, 1740, 1725, 1710,
  1695, 1681, 1666, 1652, 1637, 1623, 1609, 1594, 1580, 1566, 1552, 1538, 1524, 1510, 1496, 1482,
  1469, 1455, 1442, 1428, 1415, 1401, 1388, 1375, 1361, 1348, 1335, 1322, 1309, 1296, 1284, 1271,
  1258, 1246, 1233, 1221, 1208, 1196, 1184, 1171, 1159, 1147, 1135, 1123, 1111, 1099, 1087, 1076,
  1064, 1052, 1041, 1029, 1018, 1007, 995, 984, 973, 962, 951, 940, 929, 918, 907, 897,
  886, 876, 865, 855, 844, 834, 824, 813, 803, 793, 783, 773, 763, 754, 744, 734,
  724, 715, 705, 696, 687, 677, 668, 659, 650, 641, 632, 623, 614, 605, 596, 588,
  579, 571, 562, 554, 545, 537, 529, 521, 512, 504, 496, 489, 481, 473, 465, 458,
  450, 442, 435, 428, 420, 413, 406, 399, 391, 384, 378, 371, 364, 357, 350, 344,
  337, 331, 324, 318, 311, 305, 299, 293, 287, 281, 275, 269, 263, 257, 252, 246,
  240, 235, 230, 224, 219, 214, 208, 203, 198, 193, 188, 184, 179, 174, 169, 165,
  160, 156, 151, 147, 143, 138, 134, 130, 126, 122, 118, 114, 111, 107, 103, 100,
  96, 93, 89, 86, 83, 79, 76, 73, 70, 67, 64, 62, 59, 56, 53, 51,
  48, 46, 43, 41, 39, 37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18,
  17, 15, 14, 13, 11, 10, 9, 8, 7, 6, 5, 5, 4, 3, 3, 2,
  2, 1, 1, 1, 0, 0, 0, 0};

int main(void)
{
	uint8_t color = RED;
	uint8_t mode = 0;
	uint16_t index = 0;
	uint16_t red, green, blue;
	uint8_t step = 1;
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
	
	// Turn on the timer and the counter
    timer_enable();
	counter_enable();
	led_writeAll(0, 0, 0);
	spi_write();

	while (1)
	{	
		if ((millis - old_millis) > 20) // button debounce
		{
			if (buttons_get(0)) // if button 0 pressed
			{
				old_millis = millis;
				mode = 0; // go to mode 0
				color = RED;
			}
			else if (buttons_get(1))
			{
				old_millis = millis;
				mode = 1; // go to mode 1
			}
		}

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
				switch (state)
				{
					case 0: // off to blue
					led_write(led, 0, 0, fade_up[index]);
					break;
					
					case 1: // blue to magenta
					led_write(led, fade_up[index], 0, 4095);
					break;
					
					case 2: // magenta to red
					led_write(led, 4095, 0, fade_down[index]);
					break;
					
					case 3: // red to yellow
					led_write(led, 4095, fade_up[index], 0);
					break;
					
					case 4: // yellow to green
					led_write(led, fade_down[index], 4095, 0);
					break;
					
					case 5: // green to cyan
					led_write(led, 0, 4095, fade_up[index]);
					break;
					
					case 6:
					led_write(next_led, 0, 0, fade_up[index]); // off to blue for next LED
					led_write(led, 0, fade_down[index], 4095); // cyan to blue
					break;

					case 7:
					led_write(next_led, fade_up[index], 0, 4095); // blue to magenta for next LED
					led_write(led, 0, 0, fade_down[index]); // blue to off
					break;

					default:
					red = 0;
					green = 0;
					blue = 0;
					break;
				}
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


void SysTick_Handler ()
{
	millis++;
}

