//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "i2c.h"
#include <stdbool.h>
#include "sam.h"
#include <string.h>
//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define I2C_SCL (PORT_PA23)
#define I2C_SCL_GROUP (0)
#define I2C_SCL_PIN (PIN_PA23%32)
#define I2C_SCL_PMUX (I2C_SCL_PIN/2)

#define I2C_SDA (PORT_PA22)
#define I2C_SDA_GROUP (0)
#define I2C_SDA_PIN (PIN_PA22%32)
#define I2C_SDA_PMUX (I2C_SDA_PIN/2)


#define I2C_SERCOM            SERCOM3
#define I2C_SERCOM_PMUX       PORT_PMUX_PMUXE_C_Val
#define I2C_SERCOM_GCLK_ID    SERCOM3_GCLK_ID_CORE
#define I2C_SERCOM_CLK_GEN    0
#define I2C_SERCOM_APBCMASK   PM_APBCMASK_SERCOM3



enum // states
{
	I2C_TRANSFER_WRITE = 0,
	I2C_TRANSFER_READ  = 1,
	I2C_TRANSFER_READ_SETUP = 2,
};

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
static uint8_t send_data_buffer [256] = {0};
static uint8_t i2c_send_count = 0;
static uint8_t i2c_send_size = 0;

//static uint8_t read_data_buffer [256];
static volatile uint8_t i2c_read_count = 0;
static volatile uint8_t i2c_read_size = 0;

static uint8_t direction_flag = 0;
static uint8_t i2c_reg_addr = 0;
static uint8_t setup_count = 0;
static uint8_t dev_addr = 0;
static uint8_t *i2c_read_ptr;
//static uint8_t i2c_reading = 0;


extern uint8_t datas[6]; // Being a little naughty with this one, I'll fix it for the program.
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
void i2c_init(void)
{

	// Set up the SCL Pin
	//Set the direction - it is an output, but we want the input enable on as well
	//so that we can read it at the same time ... because I2C. 
	PORT->Group[I2C_SCL_GROUP].DIRSET.reg = I2C_SCL;
	PORT->Group[I2C_SCL_GROUP].PINCFG[I2C_SCL_PIN].bit.INEN = 1;
	// Set the pullup
	PORT->Group[I2C_SCL_GROUP].OUTSET.reg = I2C_SCL;
	PORT->Group[I2C_SCL_GROUP].PINCFG[I2C_SCL_PIN].bit.PULLEN = 1;
	//Set the PMUX
	PORT->Group[I2C_SCL_GROUP].PINCFG[I2C_SCL_PIN].bit.PMUXEN = 1;
    if (I2C_SCL_PIN & 1)								
	  PORT->Group[I2C_SCL_GROUP].PMUX[I2C_SCL_PMUX].bit.PMUXO = I2C_SERCOM_PMUX;		
    else
	  PORT->Group[I2C_SCL_GROUP].PMUX[I2C_SCL_PMUX].bit.PMUXE = I2C_SERCOM_PMUX;					


    // Set up the SDA PIN
	//Set the direction - it is an output, but we want the input enable on as well
	//so that we can read it at the same time ... because I2C.
	PORT->Group[I2C_SDA_GROUP].DIRSET.reg = I2C_SDA;
	PORT->Group[I2C_SDA_GROUP].PINCFG[I2C_SDA_PIN].bit.INEN = 1;
	// Set the pullup
	PORT->Group[I2C_SDA_GROUP].OUTSET.reg = I2C_SDA;
	PORT->Group[I2C_SDA_GROUP].PINCFG[I2C_SDA_PIN].bit.PULLEN = 1;
	//Set the PMUX
	PORT->Group[I2C_SDA_GROUP].PINCFG[I2C_SDA_PIN].bit.PMUXEN = 1;
	if (I2C_SDA_PIN & 1)
	PORT->Group[I2C_SDA_GROUP].PMUX[I2C_SDA_PMUX].bit.PMUXO = I2C_SERCOM_PMUX;
	else
	PORT->Group[I2C_SDA_GROUP].PMUX[I2C_SDA_PMUX].bit.PMUXE = I2C_SERCOM_PMUX;
	
	// Turn on the clock
	PM->APBCMASK.reg |= I2C_SERCOM_APBCMASK;

    // Configure the clock
	GCLK->CLKCTRL.reg = I2C_SERCOM_GCLK_ID |
	                    GCLK_CLKCTRL_CLKEN | 
						I2C_SERCOM_CLK_GEN;

    //Turn off the I2C enable so that we can write the protected registers
	I2C_SERCOM->I2CM.CTRLA.bit.ENABLE = 0;
	while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

	// Turn on smart mode (because it is smart)
	I2C_SERCOM->I2CM.CTRLB.bit.SMEN = 1;
	while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

    // Set the baud rate - this is a confusing little formula as 
	// it involves the actual rise time of SCL on the board
	// We would need to measure this to predict the outcome,
	// Or, we can just change it until we like it. 
	// See 27.6.2.4 of the datasheet.
	I2C_SERCOM->I2CM.BAUD.reg = SERCOM_I2CM_BAUD_BAUD(232);
	while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

    // Set us up as a Master
	I2C_SERCOM->I2CM.CTRLA.bit.MODE = SERCOM_I2CM_CTRLA_MODE_I2C_MASTER_Val;
	while (I2C_SERCOM->I2CM.SYNCBUSY.reg);
	
	// Set the hold time to 600ns
	I2C_SERCOM->I2CM.CTRLA.bit.SDAHOLD = 3;
	while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

    //Turn on the I2C enable 
	I2C_SERCOM->I2CM.CTRLA.bit.ENABLE = 1;
	while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

	// Set the bus state to be IDLE (this has to be after the enable)
	I2C_SERCOM->I2CM.STATUS.reg |= SERCOM_I2CM_STATUS_BUSSTATE(1);
	while (I2C_SERCOM->I2CM.SYNCBUSY.reg);
	
	NVIC_EnableIRQ(SERCOM3_IRQn);

}

//==============================================================================
void i2c_write(uint8_t addr, uint8_t *data, int size)
{
    // Wait for bus to become idle
    //while(I2C_SERCOM->I2CM.STATUS.bit.BUSSTATE != 0x01);

    // Clear all relevant interrupt flags before starting
    I2C_SERCOM->I2CM.INTFLAG.reg = SERCOM_I2CM_INTFLAG_MB | 
                                   SERCOM_I2CM_INTFLAG_SB | 
                                   SERCOM_I2CM_INTFLAG_ERROR;

    // Reset internal state
    i2c_send_count = 0;
    i2c_send_size = size;
    memcpy(send_data_buffer, data, size);
    direction_flag = I2C_TRANSFER_WRITE;

    // Enable interrupts
    I2C_SERCOM->I2CM.INTENSET.reg = SERCOM_I2CM_INTENSET_MB |    // Master on bus
                                    SERCOM_I2CM_INTENSET_SB |    // Slave on bus
                                    SERCOM_I2CM_INTENSET_ERROR;  // Errors

    // Start transaction by sending address
    I2C_SERCOM->I2CM.ADDR.reg = addr | I2C_TRANSFER_WRITE;
}


//==============================================================================
void i2c_read(uint8_t addr, uint8_t reg_addr, uint8_t *data, int size)
{
    // Wait for bus to become idle
    //while(I2C_SERCOM->I2CM.STATUS.bit.BUSSTATE != 0x01);

    // Clear all relevant interrupt flags before starting
    I2C_SERCOM->I2CM.INTFLAG.reg = SERCOM_I2CM_INTFLAG_MB | 
                                   SERCOM_I2CM_INTFLAG_SB | 
                                   SERCOM_I2CM_INTFLAG_ERROR;

    // Reset internal state
    i2c_reg_addr = reg_addr;
    direction_flag = I2C_TRANSFER_READ_SETUP;
    setup_count = 0;
    dev_addr = addr;
    i2c_read_size = size;
    i2c_read_count = 0;
    i2c_read_ptr = data;
    
    // Enable interrupts
    I2C_SERCOM->I2CM.INTENSET.reg = SERCOM_I2CM_INTENSET_MB |    // Master on bus
                                    SERCOM_I2CM_INTENSET_SB |    // Slave on bus
                                    SERCOM_I2CM_INTENSET_ERROR;  // Errors

    // Start transaction by sending address
    I2C_SERCOM->I2CM.ADDR.reg = addr | I2C_TRANSFER_WRITE;
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
void SERCOM3_Handler(void)
{
    // Handle Master on Bus interrupt
    if(I2C_SERCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB)
    {
        // Check for NACK
        if (I2C_SERCOM->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
        {
            // On NACK, send stop condition
            I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
            return;
        }

        switch(direction_flag)
        {
            case I2C_TRANSFER_WRITE:
                if(i2c_send_count < i2c_send_size)
                {
                    // Send next byte
                    I2C_SERCOM->I2CM.DATA.reg = send_data_buffer[i2c_send_count++];
                }
                else
                {
                    // All bytes sent, issue STOP
                    I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
                }
                break;

            case I2C_TRANSFER_READ_SETUP:
                if (setup_count == 0) // Only do the setup part once
                {
                    // Write the register address we'll be reading
                    I2C_SERCOM->I2CM.DATA.reg = i2c_reg_addr;
                    setup_count++; // Move to next state.
                }
                else
                {
                    // Prepare for reading
                    I2C_SERCOM->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;  // ACK
                    I2C_SERCOM->I2CM.ADDR.reg = dev_addr | I2C_TRANSFER_READ;  // Send repeated start with read bit
                    direction_flag = I2C_TRANSFER_READ; // We're in a read state now, so until a new I2C is issued we'll kinda ignore MB
                }
                break;

            case I2C_TRANSFER_READ:
                // Nothing to do here, data handling is in SB interrupt
                break;
        }
		I2C_SERCOM->I2CM.INTFLAG.reg = SERCOM_I2CM_INTFLAG_MB;
    }

    // Handle Slave on Bus interrupt (for reading)
    if(I2C_SERCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_SB)
    {
        if(i2c_read_count < i2c_read_size-1)
        {
            // Not the last byte
			uint8_t new_data = I2C_SERCOM->I2CM.DATA.reg;
			datas[i2c_read_count] = new_data;			// Workaround because I'm lazy and the next line wasn't working.
            i2c_read_ptr[i2c_read_count++] = new_data;	// This line isn't working but if you remove it I2C breaks.
            I2C_SERCOM->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;  // ACK
        }
        else
        {
			// These need to go before the last read otherwise they don't trigger properly (shrug)
			I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;   // NACK
            I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);   // STOP
            // Last byte
			uint8_t new_data = I2C_SERCOM->I2CM.DATA.reg;
			datas[i2c_read_count] = new_data;			// Workaround because I'm lazy and the next line wasn't working.
            i2c_read_ptr[i2c_read_count++] = new_data;	// This line isn't working but if you remove it I2C breaks.
			I2C_SERCOM->I2CM.STATUS.reg |= SERCOM_I2CM_STATUS_BUSSTATE(1); // Send to idle

        }
		I2C_SERCOM->I2CM.INTFLAG.reg = SERCOM_I2CM_INTFLAG_SB; // Clear SB flag
    }

    // Clear interrupt flags
    I2C_SERCOM->I2CM.INTFLAG.reg = SERCOM_I2CM_INTFLAG_ERROR;
}