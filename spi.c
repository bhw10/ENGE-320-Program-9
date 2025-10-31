//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "sam.h"
#include "spi.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define SPI_MOSI (PORT_PB10)
#define SPI_MOSI_GROUP (1)
#define SPI_MOSI_PIN (PIN_PB10%32)
#define SPI_MOSI_PMUX (SPI_MOSI_PIN/2)

#define SPI_MISO (PORT_PA12)
#define SPI_MISO_GROUP (0)
#define SPI_MISO_PIN (PIN_PA12%32)
#define SPI_MISO_PMUX (SPI_MISO_PIN/2)

#define SPI_SCK (PORT_PB11)
#define SPI_SCK_GROUP (1)
#define SPI_SCK_PIN (PIN_PB11%32)
#define SPI_SCK_PMUX (SPI_SCK_PIN/2)


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
extern uint8_t packed_stuff[24];
uint8_t i = 1;
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
void spi_init()
{
  //////////////////////////////////////////////////////////////////////////////
  // Configure the PORTS
  //////////////////////////////////////////////////////////////////////////////

  // MOSI
  // Configure the appropriate peripheral
#if (SPI_MOSI_PIN % 2) // Odd Pin
  PORT->Group[SPI_MOSI_GROUP].PMUX[SPI_MOSI_PMUX].bit.PMUXO = PORT_PMUX_PMUXO_D_Val;
#else                  // Even Pin
  PORT->Group[SPI_MOSI_GROUP].PMUX[SPI_MOSI_PMUX].bit.PMUXE = PORT_PMUX_PMUXE_D_Val;
#endif
  // Enable the PMUX
  PORT->Group[SPI_MOSI_GROUP].PINCFG[SPI_MOSI_PIN].bit.PMUXEN = 1;

  // MISO
  // Configure the appropriate peripheral
#if (SPI_MISO_PIN % 2) // Odd Pin
  PORT->Group[SPI_MISO_GROUP].PMUX[SPI_MISO_PMUX].bit.PMUXO = PORT_PMUX_PMUXO_D_Val;
#else                  // Even Pin
  PORT->Group[SPI_MISO_GROUP].PMUX[SPI_MISO_PMUX].bit.PMUXE = PORT_PMUX_PMUXE_D_Val;
#endif
  // Enable the PMUX
  PORT->Group[SPI_MISO_GROUP].PINCFG[SPI_MISO_PIN].bit.PMUXEN = 1;

  // SCK
  // Configure the appropriate peripheral
#if (SPI_SCK_PIN % 2) // Odd Pin
  PORT->Group[SPI_SCK_GROUP].PMUX[SPI_SCK_PMUX].bit.PMUXO = PORT_PMUX_PMUXO_D_Val;
#else                  // Even Pin
  PORT->Group[SPI_SCK_GROUP].PMUX[SPI_SCK_PMUX].bit.PMUXE = PORT_PMUX_PMUXE_D_Val;
#endif
  // Enable the PMUX
  PORT->Group[SPI_SCK_GROUP].PINCFG[SPI_SCK_PIN].bit.PMUXEN = 1;
  
  //////////////////////////////////////////////////////////////////////////////
  // Disable the SPI - 26.6.2.1
  //////////////////////////////////////////////////////////////////////////////
  SERCOM4->SPI.CTRLA.bit.ENABLE = 0;
  // Wait for it to complete
  while (SERCOM4->SPI.SYNCBUSY.bit.ENABLE);

  //////////////////////////////////////////////////////////////////////////////
  // Set up the PM (default on, but let's just do it) and the GCLK
  //////////////////////////////////////////////////////////////////////////////  
  PM->APBCMASK.reg |= PM_APBCMASK_SERCOM4;

  // Initialize the GCLK
  // Setting clock for the SERCOM4_CORE clock
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_SERCOM4_CORE | 
                      GCLK_CLKCTRL_GEN_GCLK0       | 
                      GCLK_CLKCTRL_CLKEN ;

  // Wait for the GCLK to be synchronized
  while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

  //////////////////////////////////////////////////////////////////////////////
  // Initialize the SPI
  //////////////////////////////////////////////////////////////////////////////

  // Reset the SPI
  SERCOM4->SPI.CTRLA.bit.SWRST = 1;
  // Wait for it to complete
  while (SERCOM4->SPI.CTRLA.bit.SWRST || SERCOM4->SPI.SYNCBUSY.bit.SWRST);

  // Set up CTRLA 
  SERCOM4->SPI.CTRLA.bit.DIPO = 0; // MISO on PAD0
  SERCOM4->SPI.CTRLA.bit.DOPO = 1; // MOSI on PAD2, SCK on PAD3, SS on PAD 1
  SERCOM4->SPI.CTRLA.bit.DORD = 0; // MSB Transferred first
  SERCOM4->SPI.CTRLA.bit.CPOL = 0; // SCK Low when Idle
  SERCOM4->SPI.CTRLA.bit.CPHA = 0; // Data sampled on leading edge and change on trailing edge
  SERCOM4->SPI.CTRLA.bit.MODE = 3; // Set MODE as SPI Master

  // Set up CTRLB
  SERCOM4->SPI.CTRLB.bit.RXEN = 1; // Enable the receiver

  // Set up the BAUD rate
  SERCOM4->SPI.BAUD.reg = 11; // 100KHz - too slow, but easy to see on the Logic Analyzer


  //////////////////////////////////////////////////////////////////////////////
  // Enable the SPI
  ////////////////////////////////////////////////////////////////////////////// 
  SERCOM4->SPI.CTRLA.bit.ENABLE = 1;
  SERCOM4->SPI.INTENSET.reg = SERCOM_SPI_INTENSET_TXC | SERCOM_SPI_INTENSET_DRE;
  NVIC_EnableIRQ(SERCOM4_IRQn);
  // Wait for it to complete
  while (SERCOM4->SPI.SYNCBUSY.bit.ENABLE);

}


//==============================================================================
void spi_write()
{
	while (!SERCOM4->SPI.INTFLAG.bit.DRE);
    SERCOM4->SPI.DATA.reg = packed_stuff[0];
	SERCOM4->SPI.INTENSET.reg = SERCOM_SPI_INTENSET_DRE;
    // Wait until transmission complete (TXC)
    while (!SERCOM4->SPI.INTFLAG.bit.DRE);

}


//==============================================================================
uint8_t spi_read()
{
  // Wait for something to show up in the data register
  while( SERCOM4->SPI.INTFLAG.bit.DRE == 0 || SERCOM4->SPI.INTFLAG.bit.RXC == 0 );
  // Read it and return it. 
  return SERCOM4->SPI.DATA.bit.DATA;
}

//==============================================================================
uint8_t spi(uint8_t data)
{
  // Wait for the data register to be empty
  while (SERCOM4->SPI.INTFLAG.bit.DRE == 0); 
  // Send the data
  SERCOM4->SPI.DATA.bit.DATA = data;
  // Wait for something to show up in the data register
  while( SERCOM4->SPI.INTFLAG.bit.DRE == 0 || SERCOM4->SPI.INTFLAG.bit.RXC == 0 );
  // Read it and return it. 
  return SERCOM4->SPI.DATA.bit.DATA;
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
void SERCOM4_Handler()
{
	    //uint8_t data;
	    
	    if (SERCOM4->SPI.INTFLAG.bit.DRE) {
		    // Ready to transmit next byte

			if(i < 24)
			{
				SERCOM4->SPI.DATA.reg = packed_stuff[i++];
				data_sent = 0;
				while (!SERCOM4->SPI.INTFLAG.bit.TXC);
			}
			else
			{
				SERCOM4->SPI.INTENCLR.reg = SERCOM_SPI_INTENCLR_DRE;
				i = 1;
				SERCOM4->SPI.INTENSET.reg = SERCOM_SPI_INTENSET_TXC;
			}
		SERCOM4->SPI.INTFLAG.reg = SERCOM_SPI_INTFLAG_DRE; // Clear flag
	    }
		
		if (SERCOM4->SPI.INTFLAG.bit.TXC) {
			SERCOM4->SPI.INTFLAG.reg = SERCOM_SPI_INTFLAG_TXC; // clear flag
			SERCOM4->SPI.INTENCLR.reg = SERCOM_SPI_INTENCLR_TXC; // disable TXC
			data_sent = 1;
		}
}