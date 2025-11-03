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
static volatile uint8_t i = 1;
volatile uint8_t busy = 0;
extern volatile uint8_t data_sent;
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
void spi_init(void)
{
	//////////////////////////////////////////////////////////////////////////////
	// Configure the PORTS
	//////////////////////////////////////////////////////////////////////////////

	// MOSI
	#if (SPI_MOSI_PIN % 2)
	PORT->Group[SPI_MOSI_GROUP].PMUX[SPI_MOSI_PMUX].bit.PMUXO = PORT_PMUX_PMUXO_D_Val;
	#else
	PORT->Group[SPI_MOSI_GROUP].PMUX[SPI_MOSI_PMUX].bit.PMUXE = PORT_PMUX_PMUXE_D_Val;
	#endif
	PORT->Group[SPI_MOSI_GROUP].PINCFG[SPI_MOSI_PIN].bit.PMUXEN = 1;

	// MISO
	#if (SPI_MISO_PIN % 2)
	PORT->Group[SPI_MISO_GROUP].PMUX[SPI_MISO_PMUX].bit.PMUXO = PORT_PMUX_PMUXO_D_Val;
	#else
	PORT->Group[SPI_MISO_GROUP].PMUX[SPI_MISO_PMUX].bit.PMUXE = PORT_PMUX_PMUXE_D_Val;
	#endif
	PORT->Group[SPI_MISO_GROUP].PINCFG[SPI_MISO_PIN].bit.PMUXEN = 1;

	// SCK
	#if (SPI_SCK_PIN % 2)
	PORT->Group[SPI_SCK_GROUP].PMUX[SPI_SCK_PMUX].bit.PMUXO = PORT_PMUX_PMUXO_D_Val;
	#else
	PORT->Group[SPI_SCK_GROUP].PMUX[SPI_SCK_PMUX].bit.PMUXE = PORT_PMUX_PMUXE_D_Val;
	#endif
	PORT->Group[SPI_SCK_GROUP].PINCFG[SPI_SCK_PIN].bit.PMUXEN = 1;

	//////////////////////////////////////////////////////////////////////////////
	// Disable and Reset SPI
	//////////////////////////////////////////////////////////////////////////////
	SERCOM4->SPI.CTRLA.bit.ENABLE = 0;
	while (SERCOM4->SPI.SYNCBUSY.bit.ENABLE);

	PM->APBCMASK.reg |= PM_APBCMASK_SERCOM4;

	// GCLK setup
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_SERCOM4_CORE |
	GCLK_CLKCTRL_GEN_GCLK0 |
	GCLK_CLKCTRL_CLKEN;
	while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

	// Reset SPI
	SERCOM4->SPI.CTRLA.bit.SWRST = 1;
	while (SERCOM4->SPI.CTRLA.bit.SWRST || SERCOM4->SPI.SYNCBUSY.bit.SWRST);

	//////////////////////////////////////////////////////////////////////////////
	// Configure SPI
	//////////////////////////////////////////////////////////////////////////////
	SERCOM4->SPI.CTRLA.bit.DIPO = 0; // MISO on PAD0
	SERCOM4->SPI.CTRLA.bit.DOPO = 1; // MOSI on PAD2, SCK on PAD3
	SERCOM4->SPI.CTRLA.bit.DORD = 0; // MSB first
	SERCOM4->SPI.CTRLA.bit.CPOL = 0; // Clock idle low
	SERCOM4->SPI.CTRLA.bit.CPHA = 0; // Sample on leading edge
	SERCOM4->SPI.CTRLA.bit.MODE = 3; // Master mode

	SERCOM4->SPI.CTRLB.bit.RXEN = 1;
	while (SERCOM4->SPI.SYNCBUSY.bit.CTRLB);

	// Baud: Fspi = 48MHz / (2 * (BAUD + 1)) = ~2MHz
	SERCOM4->SPI.BAUD.reg = 11;

	SERCOM4->SPI.CTRLA.bit.ENABLE = 1;
	while (SERCOM4->SPI.SYNCBUSY.bit.ENABLE);

	NVIC_EnableIRQ(SERCOM4_IRQn);
}


//==============================================================================
void spi_write()
{
	if (!spi_lock())
	{
		return; // spi busy
	}
	
	data_sent = 0;
	i = 1;
	
    SERCOM4->SPI.DATA.reg = packed_stuff[0];
	SERCOM4->SPI.INTENSET.reg = SERCOM_SPI_INTENSET_DRE;
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

//==============================================================================

uint8_t spi_lock()
{
	if (busy) // spi already busy
	{
		return 0;
	}
	else
	{
		busy = 1;
		return 1; // lock spi
	}
}

//==============================================================================

void spi_unlock()
{
	busy = 0; // unlock spi
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
	// Data register empty
	if (SERCOM4->SPI.INTFLAG.bit.DRE) 
	{
	    // Ready to transmit next byte
		if (i < 24)
		{
			SERCOM4->SPI.DATA.reg = packed_stuff[i++];
		}
		else
		{
			SERCOM4->SPI.INTENCLR.reg = SERCOM_SPI_INTENCLR_DRE;
			SERCOM4->SPI.INTENSET.reg = SERCOM_SPI_INTENSET_TXC;
		}
		SERCOM4->SPI.INTFLAG.reg = SERCOM_SPI_INTFLAG_DRE; // Clear flag
	   }
	
	// Transfer complete
	if (SERCOM4->SPI.INTFLAG.bit.TXC) 
	{
		SERCOM4->SPI.INTFLAG.reg = SERCOM_SPI_INTFLAG_TXC;
		SERCOM4->SPI.INTENCLR.reg = SERCOM_SPI_INTENCLR_TXC;
		i = 1;
		data_sent = 1;
		spi_unlock();  // release SPI lock
	}
}