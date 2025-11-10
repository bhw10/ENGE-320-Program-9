// *****************************************************************
// *
// * Name Britton Williams;
// * Program : Program 8;
// * Class : ENGE 320
// * Date : 11/ 4/25;
// * Description : Buttons control modes. Mode 0 causes all LEDs to fade between colors concurrently. Mode 1 causes LEDs to fade individually in a figure-8 pattern.
// * Joystick controls speed of fade.
// *
// * =============================================================
// * Program Grading Criteria
// * =============================================================
// * REQUIRED (40)
// * Pressing S0 selects mode 0: yes
// * Mode 0 LED’s fade in correct sequence: yes
// * Pressing S1 selects mode 1: yes
// * Mode 1 LED’s fade in correct sequence: yes
// * OPTIONAL (80)
// * Without joystick, Mode 0 LED’s fade a full cycle in 1 second: (10) 10
// * When joystick up, Mode 0 LED’s fade a full cycle in 1/6 of a second: (10) 10
// * When joystick down, Mode 0 LED’s fade a full cycle in 6 seconds: (10) 10
// * Mode 0 Joystick up/down varies timing proportionally: (10) 0
// * Without joystick, Mode 1 LED’s fade a full cycle in 1 second: (10) 10
// * When joystick up, Mode 1 LED’s fade a full cycle in 1/6 of a second: (10) 10
// * When joystick down, Mode 1 LED’s fade a full cycle in 6 seconds: (10) 10
// * Mode 1 joystick up/down varies timing proportionally: (10) 0

// * Total (120) 100
// * =============================================================
//*****************************************************************

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
#include <stdlib.h>

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
static uint8_t color = RED;
static uint8_t mode = 0;
static uint16_t index = 0;
static uint8_t led = 1;
static uint8_t state = 0;
static uint8_t next_led = 2;
static uint8_t led_state = 0;

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
	
const uint16_t xy_fade_up[410] = {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,
	2,   2,   2,   3,   3,   3,   3,   4,   4,   4,   4,   5,   5,   5,   6,   6,
	7,   7,   7,   8,   8,   9,   9,  10,  11,  11,  12,  12,  13,  14,  14,  15,
	16,  16,  17,  18,  19,  20,  21,  21,  22,  23,  24,  25,  26,  27,  28,  30,
	31,  32,  33,  34,  35,  37,  38,  39,  41,  42,  44,  45,  47,  48,  50,  51,
	53,  55,  56,  58,  60,  62,  64,  65,  67,  69,  71,  73,  75,  78,  80,  82,
	84,  86,  89,  91,  93,  96,  98, 101, 103, 106, 109, 111, 114, 117, 120, 123,
	126, 128, 131, 135, 138, 141, 144, 147, 151, 154, 157, 161, 164, 168, 171, 175,
	179, 182, 186, 190, 194, 198, 202, 206, 210, 214, 219, 223, 227, 232, 236, 241,
	245, 250, 254, 259, 264, 269, 274, 279, 284, 289, 294, 299, 305, 310, 315, 321,
	326, 332, 338, 343, 349, 355, 361, 367, 373, 379, 385, 391, 398, 404, 411, 417,
	424, 430, 437, 444, 451, 458, 465, 472, 479, 486, 493, 501, 508, 516, 523, 531,
	539, 546, 554, 562, 570, 578, 587, 595, 603, 612, 620, 629, 637, 646, 655, 664,
	673, 682, 691, 700, 709, 719, 728, 738, 747, 757, 767, 777, 787, 797, 807, 817,
	827, 838, 848, 859, 869, 880, 891, 902, 913, 924, 935, 946, 958, 969, 981, 992,
	1004,1016,1028,1040,1052,1064,1076,1089,1101,1114,1126,1139,1152,1165,1178,1191,
	1204,1218,1231,1245,1258,1272,1286,1300,1314,1328,1342,1357,1371,1386,1400,1415,
	1430,1445,1460,1475,1490,1506,1521,1537,1552,1568,1584,1600,1616,1632,1649,1665,
	1682,1698,1715,1732,1749,1766,1783,1800,1818,1835,1853,1871,1889,1907,1925,1943,
	1961,1980,1998,2017,2036,2055,2074,2093,2112,2131,2151,2171,2190,2210,2230,2250,
	2270,2291,2311,2332,2352,2373,2394,2415,2436,2458,2479,2501,2522,2544,2566,2588,
	2610,2633,2655,2678,2700,2723,2746,2769,2792,2816,2839,2863,2887,2910,2934,2959,
	2983,3007,3032,3056,3081,3106,3131,3156,3182,3207,3233,3258,3284,3310,3336,3363,
	3389,3416,3442,3469,3496,3523,3550,3578,3605,3633,3661,3689,3717,3745,3773,3802,
	3831,3859,3888,3917,3947,3976,4006,4035,4065,4095};
	
const uint16_t xy_fade_down[410] = {4095, 4065, 4035, 4006, 3976, 3947, 3917, 3888, 3859, 3831, 3802, 3773, 3745, 3717, 3689, 3661,
	3633, 3605, 3578, 3550, 3523, 3496, 3469, 3442, 3416, 3389, 3363, 3336, 3310, 3284, 3258, 3233,
	3207, 3182, 3156, 3131, 3106, 3081, 3056, 3032, 3007, 2983, 2959, 2934, 2910, 2887, 2863, 2839,
	2816, 2792, 2769, 2746, 2723, 2700, 2678, 2655, 2633, 2610, 2588, 2566, 2544, 2522, 2501, 2479,
	2458, 2436, 2415, 2394, 2373, 2352, 2332, 2311, 2291, 2270, 2250, 2230, 2210, 2190, 2171, 2151,
	2131, 2112, 2093, 2074, 2055, 2036, 2017, 1998, 1980, 1961, 1943, 1925, 1907, 1889, 1871, 1853,
	1835, 1818, 1800, 1783, 1766, 1749, 1732, 1715, 1698, 1682, 1665, 1649, 1632, 1616, 1600, 1584,
	1568, 1552, 1537, 1521, 1506, 1490, 1475, 1460, 1445, 1430, 1415, 1400, 1386, 1371, 1357, 1342,
	1328, 1314, 1300, 1286, 1272, 1258, 1245, 1231, 1218, 1204, 1191, 1178, 1165, 1152, 1139, 1126,
	1114, 1101, 1089, 1076, 1064, 1052, 1040, 1028, 1016, 1004, 992, 981, 969, 958, 946, 935,
	924, 913, 902, 891, 880, 869, 859, 848, 838, 827, 817, 807, 797, 787, 777, 767,
	757, 747, 738, 728, 719, 709, 700, 691, 682, 673, 664, 655, 646, 637, 629, 620,
	612, 603, 595, 587, 578, 570, 562, 554, 546, 539, 531, 523, 516, 508, 501, 493,
	486, 479, 472, 465, 458, 451, 444, 437, 430, 424, 417, 411, 404, 398, 391, 385,
	379, 373, 367, 361, 355, 349, 343, 338, 332, 326, 321, 315, 310, 305, 299, 294,
	289, 284, 279, 274, 269, 264, 259, 254, 250, 245, 241, 236, 232, 227, 223, 219,
	214, 210, 206, 202, 198, 194, 190, 186, 182, 179, 175, 171, 168, 164, 161, 157,
	154, 151, 147, 144, 141, 138, 135, 131, 128, 126, 123, 120, 117, 114, 111, 109,
	106, 103, 101, 98, 96, 93, 91, 89, 86, 84, 82, 80, 78, 75, 73, 71,
	69, 67, 65, 64, 62, 60, 58, 56, 55, 53, 51, 50, 48, 47, 45, 44,
	42, 41, 39, 38, 37, 35, 34, 33, 32, 31, 30, 28, 27, 26, 25, 24,
	23, 22, 21, 21, 20, 19, 18, 17, 16, 16, 15, 14, 14, 13, 12, 12,
	11, 11, 10, 9, 9, 8, 8, 7, 7, 7, 6, 6, 5, 5, 5, 4,
	4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

static uint8_t calculate_adc();
static void mode_select();

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

int main(void)
{
	uint16_t red, green, blue, next_red, next_green, next_blue, x_red, y_red, x_green, y_green, x_blue, y_blue, middle_red, middle_green, middle_blue;
	uint8_t step;
	uint32_t old_millis = 0;

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
	adc_reset();
	led_writeAll(0, 0, 0);
	spi_write();

	while (1)
	{
		while (mode == 0)
		{
			if ((millis - old_millis) > 20) // button debounce
			{
				old_millis = millis;
				if (buttons_get(1)) // if button 1 pressed
				{
					adc_reset();
					mode = 1; // go to mode 1
					led_writeAll(0, 0, 0); // turn off LEDs
					spi_write();
					counter_flagSet(0); // make sure flag is cleared before next spi write
					// Initialize state variables
					index = 0;
					state = 0;
					led = 1;
					next_led = 2;
					led_state = 0;
				}
				if (buttons_get(2))
				{
					adc_interruptSet();
					mode = 2;
					led_writeAll(0, 0, 0);
					spi_write();
					counter_flagSet(0);
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
			if ((millis - old_millis) > 20)
			{
				old_millis = millis;
				if (buttons_get(0)) // if button 0 pressed
				{
					adc_reset();
					mode = 0; // go to mode 0
					color = RED;
					led_writeAll(0, 0, 0); // turn off LEDs
					spi_write();
					index = 0;
				}
				if (buttons_get(2))
				{
					adc_interruptSet();
					mode = 2;
					led_writeAll(0, 0, 0);
					spi_write();
					counter_flagSet(0);
				}
			}
			if (counter_flagGet()) // if flag set
			{
				counter_flagSet(0); // clear flag
				step = calculate_adc();
				switch (state)
				{
					case 0: // off to blue
					red = 0;
					green = 0;
					blue = fade_up[index];
					next_red = 0;
					next_green = 0;
					next_blue = 0;
					break;
					
					case 1: // blue to magenta
					red = fade_up[index];
					green = 0;
					blue = 4095;
					next_red = 0;
					next_green = 0;
					next_blue = 0;
					break;
					
					case 2: // magenta to red
					red = 4095;
					green = 0;
					blue = fade_down[index];
					next_red = 0;
					next_green = 0;
					next_blue = 0;
					break;
					
					case 3: // red to yellow
					red = 4095;
					green = fade_up[index];
					blue = 0;
					next_red = 0;
					next_green = 0;
					next_blue = 0;
					break;
					
					case 4: // yellow to green
					red = fade_down[index];
					green = 4095;
					blue = 0;
					next_red = 0;
					next_green = 0;
					next_blue = 0;
					break;
					
					case 5: // green to cyan
					red = 0;
					green = 4095;
					blue = fade_up[index];
					next_red = 0;
					next_green = 0;
					next_blue = 0;
					break;
					
					case 6:
					red = 0;
					green = fade_down[index];
					blue = 4095;
					next_red = 0;
					next_green = 0;
					next_blue = fade_up[index];
					break;

					case 7:
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
		while (mode == 2)
		{
			if ((millis - old_millis) > 20) // button debounce
			{
				old_millis = millis;
				if (buttons_get(0)) // if button 0 pressed
				{
					adc_reset();
					mode = 0; // go to mode 0
					color = RED;
					led_writeAll(0, 0, 0); // turn off LEDs
					spi_write();
					index = 0;
				}
				if (buttons_get(1)) // if button 1 pressed
				{
					adc_reset();
					mode = 1; // go to mode 1
					led_writeAll(0, 0, 0); // turn off LEDs
					spi_write();
					counter_flagSet(0); // make sure flag is cleared before next spi write
					// Initialize state variables
					index = 0;
					state = 0;
					led = 1;
					next_led = 2;
					led_state = 0;
				}
			}
			if (counter_flagGet())
			{
				counter_flagSet(0); // clear flag
				led_writeAll(0, 0, 0); // turn off all LEDs
				
				// Set LED values
				if (abs(adc_get_X()) < 500)
				{
					x_blue = xy_fade_up[abs(adc_get_X())]; // fade blue up
				}
				else if (abs(adc_get_X()) < 820)
				{
					x_red = xy_fade_up[abs(adc_get_X())]; // fade red up
					x_blue = xy_fade_down[abs(adc_get_X())]; // fade blue down
				}
				else if (abs(adc_get_X()) < 1230)
				{
					x_green = xy_fade_up[abs(adc_get_X())]; // fade green up
					x_red = xy_fade_down[abs(adc_get_X())]; // fade red down
				}
				else if (abs(adc_get_X()) < 1640)
				{
					x_green = xy_fade_up[abs(adc_get_X())]; // fade green up
					x_blue = xy_fade_up[abs(adc_get_X())]; // fade blue up
				}
				else
				{
					x_green = 4095; // max green
					x_blue = 4095; // max blue
					x_red = xy_fade_up[abs(adc_get_X())]; // fade red up
				}
				if (abs(adc_get_Y()) < 500)
				{
					y_blue = xy_fade_up[abs(adc_get_Y())]; // fade blue up
				}
				else if (abs(adc_get_Y()) < 820)
				{
					y_red = xy_fade_up[abs(adc_get_Y())]; // fade red up
					y_blue = xy_fade_down[abs(adc_get_Y())]; // fade blue down
				}
				else if (abs(adc_get_Y()) < 1230)
				{
					y_green = xy_fade_up[abs(adc_get_Y())]; // fade green up
					y_red = xy_fade_down[abs(adc_get_Y())]; // fade red down
				}
				else if (abs(adc_get_Y()) < 1640)
				{
					y_green = xy_fade_up[abs(adc_get_Y())]; // fade green up
					y_blue = xy_fade_up[abs(adc_get_Y())]; // fade blue up
				}
				else
				{
					y_green = 4095; // max green
					y_blue = 4095; // max blue
					y_red = xy_fade_up[abs(adc_get_Y())]; // fade red up
				}
				
				// Write to correct LEDs
				if (adc_get_X() < 0)
				{
					led_write(4, x_red, x_green, x_blue);
				}
				else if (adc_get_X() > 0)
				{
					led_write(2, x_red, x_green, x_blue);
				}
				if (adc_get_Y() > 0)
				{
					led_write(1, y_red, y_green, y_blue);
				}
				else if (adc_get_Y() < 0)
				{
					led_write(3, y_red, y_green, y_blue);
				}
				spi_write();
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
	if (adc_get() < 100) // joystick at max up
	{
		step = 36;
	}
	else if (adc_get() > 3095) // joystick at max down
	{
		step = 1;
	}
	else if ((adc_get() > 1950) && (adc_get() < 2150))// joystick neutral
	{
		step = 6;
	}
	else
	{
		step = (adc_get() / 4095) * 36;
	}
	
	return step;
}


static void mode_select()
{
	if (buttons_get(0)) // if button 0 pressed
	{
		adc_reset();
		mode = 0; // go to mode 0
		color = RED;
		led_writeAll(0, 0, 0); // turn off LEDs
		spi_write();
		index = 0;
	}
	if (buttons_get(1)) // if button 1 pressed
	{
		adc_reset();
		mode = 1; // go to mode 1
		led_writeAll(0, 0, 0); // turn off LEDs
		spi_write();
		counter_flagSet(0); // make sure flag is cleared before next spi write
		// Initialize state variables
		index = 0;
		state = 0;
		led = 1;
		next_led = 2;
		led_state = 0;
	}
	if (buttons_get(2))
	{
		adc_interruptSet();
		mode = 2;
		led_writeAll(0, 0, 0);
		spi_write();
		counter_flagSet(0);
		}
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