//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "accelerometer.h"
#include "i2c.h"
#include "delay.h"
#include "bmi160.h"

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
int8_t rslt = BMI160_OK;
struct bmi160_dev sensor;
struct bmi160_sensor_data accel;
struct bmi160_sensor_data temp_accel;

volatile uint8_t datas[6];

static int16_t msblsb_x;
static int16_t msblsb_y;
static int16_t msblsb_z;
//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------
int test_eeprom(void);
void user_delay_ms(uint32_t ms);
int8_t user_i2c_read(uint8_t dev_addr, uint8_t reg_addr,
uint8_t *data, uint16_t length);
int8_t user_i2c_write(uint8_t dev_addr, uint8_t reg_addr,
uint8_t *data, uint16_t length);
//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void accelerometer_init()
{

    volatile uint8_t retval;

    sensor.id = BMI160_I2C_ADDR;
    sensor.interface = BMI160_I2C_INTF;
    sensor.read = user_i2c_read;
    sensor.write = user_i2c_write;
    sensor.delay_ms = user_delay_ms;
    
    retval = bmi160_init(&sensor);
    if (retval != BMI160_OK) {
        // Initialization failed
		accelerometer_init();
    }
    
    /* Select the Output data rate, range of accelerometer sensor */
    sensor.accel_cfg.odr = BMI160_ACCEL_ODR_1600HZ;
    sensor.accel_cfg.range = BMI160_ACCEL_RANGE_2G;
    sensor.accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;
    /* Select the power mode of accelerometer sensor */
    sensor.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

    /* Set the sensor configuration */
    rslt = bmi160_set_sens_conf(&sensor);
    if (rslt != BMI160_OK) {
        // Configuration failed
        accelerometer_init(); // Stop here in debug
    }

    // Wait for power-up and configuration to take effect
    user_delay_ms(100);
}

//==============================================================================

uint8_t accelerometer_get()
{
    // Get sensor data
    rslt = bmi160_get_sensor_data(BMI160_ACCEL_SEL, &temp_accel, NULL, &sensor);
    if (rslt != BMI160_OK) {
        return 0; // Error reading sensor
    }
	uint8_t lsb_x, msb_x;
	
	uint8_t lsb_y, msb_y;
	
	uint8_t lsb_z, msb_z;
	
	/* Accel Data */
	lsb_x = datas[0];
	msb_x = datas[1];
	msblsb_x = (int16_t)((msb_x << 8) | lsb_x);
	//accel->x = msblsb_x; /* Data in X axis */

	lsb_y = datas[2];
	msb_y = datas[3];
	msblsb_y = (int16_t)((msb_y << 8) | lsb_y);
	//accel->y = msblsb_y; /* Data in Y axis */

	lsb_z = datas[4];
	msb_z = datas[5];
	msblsb_z = (int16_t)((msb_z << 8) | lsb_z);
	//accel->z = msblsb_z; /* Data in Z axis */
       
    return 1;
}

int16_t accelerometer_get_x()
{
	return msblsb_x;
}
int16_t accelerometer_get_y()
{
	return msblsb_y;
}
int16_t accelerometer_get_z()
{
	return msblsb_z;
}

//------------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//------------------------------------------------------------------------------
void user_delay_ms(uint32_t ms)
{
	DelayMs(ms);
}

//=============================================================================
int8_t user_i2c_read(uint8_t dev_addr, uint8_t reg_addr,
uint8_t *data, uint16_t length)
{
	// Call i2c_read with the register value (not its address).
	// i2c_read signature: i2c_read(uint8_t addr, uint8_t reg_addr, uint8_t *data, int size)
	i2c_read(dev_addr << 1, reg_addr, data, length);
	return BMI160_OK;
}

//=============================================================================
int8_t user_i2c_write(uint8_t dev_addr, uint8_t reg_addr,
uint8_t *data, uint16_t length)
{
	uint8_t mydata[32];
	// Limit the payload length to fit in our buffer (reserve 1 byte for reg addr)
	if (length > 31) length = 31;
	// Prepare buffer: first byte = register, following bytes = data
	mydata[0] = reg_addr;
	memcpy(mydata + 1, data, length);
	// Send register + data (length + 1)
	i2c_write(dev_addr << 1, mydata, length + 1);
	return BMI160_OK;
}
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
