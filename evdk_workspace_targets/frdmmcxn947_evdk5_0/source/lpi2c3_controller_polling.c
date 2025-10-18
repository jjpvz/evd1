/*! ***************************************************************************
 *
 * \brief     Low-Power I2C controller polling
 * \file      lpi2c3_controller_polling.c
 * \author    Hugo Arends
 * \date      October 2025
 *
 * \copyright 2025 HAN University of Applied Sciences. All Rights Reserved.
 *            \n\n
 *            Permission is hereby granted, free of charge, to any person
 *            obtaining a copy of this software and associated documentation
 *            files (the "Software"), to deal in the Software without
 *            restriction, including without limitation the rights to use,
 *            copy, modify, merge, publish, distribute, sublicense, and/or sell
 *            copies of the Software, and to permit persons to whom the
 *            Software is furnished to do so, subject to the following
 *            conditions:
 *            \n\n
 *            The above copyright notice and this permission notice shall be
 *            included in all copies or substantial portions of the Software.
 *            \n\n
 *            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *            EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *            OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *            NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *            HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *            WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *            FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *            OTHER DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
#include "lpi2c3_controller_polling.h"
#include "board.h"
#include "fsl_common.h"
#include "fsl_lpi2c.h"
#include "fsl_lpflexcomm.h"
#include "fsl_port.h"

#include "pin_mux.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
#define SSD1306_I2C_INSTANCE 3
#define SSD1306_I2C_BASEADDR LPI2C3

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------
static inline void lpi2c3_wait_busy(void);
static inline bool lpi2c3_txfifo_full(void);

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void lpi2c3_controller_init(void)
{
    CLOCK_EnableClock(kCLOCK_Gpio1);
    CLOCK_EnableClock(kCLOCK_Port1);

    /* PORT1_0 is configured as FC3_P0 */
    PORT_SetPinMux(PORT1, 0U, kPORT_MuxAlt2);

    PORT1->PCR[0] = ((PORT1->PCR[0] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PS_MASK | PORT_PCR_ODE_MASK | PORT_PCR_DSE_MASK | PORT_PCR_IBE_MASK)))

                     /* Pull Select: Enables internal pullup resistor. */
                     | PORT_PCR_PS(PCR_PS_ps1)

                     /* Open Drain Enable: Enables. */
                     | PORT_PCR_ODE(PCR_ODE_ode1)

                     /* Drive Strength Enable: High. */
                     | PORT_PCR_DSE(PCR_DSE_dse1)

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT1_1 is configured as FC3_P1 */
    PORT_SetPinMux(PORT1, 1U, kPORT_MuxAlt2);

    PORT1->PCR[1] = ((PORT1->PCR[1] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PS_MASK | PORT_PCR_ODE_MASK | PORT_PCR_DSE_MASK | PORT_PCR_IBE_MASK)))

                     /* Pull Select: Enables internal pullup resistor. */
                     | PORT_PCR_PS(PCR_PS_ps1)

                     /* Open Drain Enable: Enables. */
                     | PORT_PCR_ODE(PCR_ODE_ode1)

                     /* Drive Strength Enable: High. */
                     | PORT_PCR_DSE(PCR_DSE_dse1)

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));

    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM3);
    CLOCK_EnableClock(kCLOCK_LPFlexComm3);
    CLOCK_EnableClock(kCLOCK_LPI2c3);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom3Clk, 1u);

    LP_FLEXCOMM_Init(SSD1306_I2C_INSTANCE, LP_FLEXCOMM_PERIPH_LPI2C);
    BOARD_LPI2C_Init(SSD1306_I2C_BASEADDR, CLOCK_GetLPFlexCommClkFreq(SSD1306_I2C_INSTANCE));

    // Update I2C timing parameters to setup the following specifications
    // I2C mode: FAST
    // Clock frequency: 12 MHz
    // I2C baud rate: 400 kbits/s
    LPI2C3->MCFGR1 = LPI2C_MCFGR1_PRESCALE(0);
    LPI2C3->MCFGR2 = LPI2C_MCFGR2_FILTSDA(0) | LPI2C_MCFGR2_FILTSCL(0);
    LPI2C3->MCCR0 = LPI2C_MCCR0_DATAVD(0x02) | LPI2C_MCCR0_SETHOLD(0x04) |
        LPI2C_MCCR0_CLKHI(0x05) | LPI2C_MCCR0_CLKLO(0x0B);
}

/*!
 * \brief Sends multiple commands to the Oled display
 *
 * All commands are transferred in a single I2C transfer to the Oled display.
 * If there is no response within the timeout, this function will return false.
 *
 * \param[in]  address  I2C address of the Oled display
 * \param[in]  cmd      Pointer to the array of commands to be transmitted
 * \param[in]  n        Number of commands
 *
 * \return True on successful communication, false otherwise
 */
bool lpi2c3_write_cmd(const uint8_t address, const uint8_t cmd[], const uint32_t n)
{
    // Wait as long as bus or controller is busy
    lpi2c3_wait_busy();

    // Clear all status flags
    LPI2C3->MSR = LPI2C_MSR_STF_MASK | LPI2C_MSR_DMF_MASK |
        LPI2C_MSR_PLTF_MASK | LPI2C_MSR_FEF_MASK | LPI2C_MSR_ALF_MASK |
        LPI2C_MSR_NDF_MASK | LPI2C_MSR_SDF_MASK | LPI2C_MSR_EPF_MASK;

    // Command: 100b - Generate (repeated) Start on the I2C bus and transmit
    //          the address in DATA[7:0]
    // Data   : Slave address + w
    LPI2C3->MTDR = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA(address << 1);

    // Wait if the TXFIFO is full
    while(lpi2c3_txfifo_full())
    {}

    // Command: 000b - Transmit the value in DATA[7:0]
    // Data   : Send control byte: next byte is acted as a cmd
    LPI2C3->MTDR = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(0x00);

    for(uint32_t i=0; i<n; ++i)
    {
        // Wait if the TXFIFO is full
        while(lpi2c3_txfifo_full())
        {}

        // Command: 000b - Transmit the value in DATA[7:0]
        // Data   : Send control byte: next byte is acted as a data
        LPI2C3->MTDR = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(cmd[i]);
    }

    // Wait if the TXFIFO is full
    while(lpi2c3_txfifo_full())
    {}

    // Command: 010b - Generate Stop condition on I2C bus
    // Data   : n.a.
    LPI2C3->MTDR = LPI2C_MTDR_CMD(0b010);

    return true;
}

/*!
 * \brief Sends multiple data bytes to the Oled display
 *
 * All data bytes are transferred in a single I2C transfer to the Oled display.
 * If there is no response within the timeout, this function will return false.
 *
 * \param[in]  address  I2C address of the Oled display
 * \param[in]  data     Pointer to the array of data bytes to be transmitted
 * \param[in]  n        Number of data bytes
 *
 * \return True on successful communication, false otherwise
 */
bool lpi2c3_write_data(const uint8_t address, const uint8_t data[], const uint32_t n)
{
    // Wait as long as bus or controller is busy
    lpi2c3_wait_busy();

    // Clear all status flags
    LPI2C3->MSR = LPI2C_MSR_STF_MASK | LPI2C_MSR_DMF_MASK |
        LPI2C_MSR_PLTF_MASK | LPI2C_MSR_FEF_MASK | LPI2C_MSR_ALF_MASK |
        LPI2C_MSR_NDF_MASK | LPI2C_MSR_SDF_MASK | LPI2C_MSR_EPF_MASK;

    // Command: 100b - Generate (repeated) Start on the I2C bus and transmit
    //          the address in DATA[7:0]
    // Data   : Slave address + w
    LPI2C3->MTDR = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA(address << 1);

    // Wait if the TXFIFO is full
    while(lpi2c3_txfifo_full())
    {}

    // Command: 000b - Transmit the value in DATA[7:0]
    // Data   : Send control byte: next byte is acted as a data
    LPI2C3->MTDR = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(0x40);

    for(uint32_t i=0; i<n; ++i)
    {
        // Wait if the TXFIFO is full
        while(lpi2c3_txfifo_full())
        {}

        // Command: 000b - Transmit the value in DATA[7:0]
        // Data   : Send control byte: next byte is acted as a data
        LPI2C3->MTDR = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(data[i]);
    }

    // Wait if the TXFIFO is full
    while(lpi2c3_txfifo_full())
    {}

    // Command: 010b - Generate Stop condition on I2C bus
    // Data   : n.a.
    LPI2C3->MTDR = LPI2C_MTDR_CMD(0b010);

    return true;
}

static inline void lpi2c3_wait_busy(void)
{
    // Wait as long as Bus Busy Flag or Controller Busy Flag is set
    while((LPI2C3->MSR & (LPI2C_MSR_BBF_MASK | LPI2C_MSR_MBF_MASK)) != 0)
    {}
}

static inline bool lpi2c3_txfifo_full(void)
{
    uint32_t n = (LPI2C3->MFSR & LPI2C_MFSR_TXCOUNT_MASK) >> LPI2C_MFSR_TXCOUNT_SHIFT;

    // See reference manual section 66.3.6.1.3
    //
    // Command and transmit FIFO of 2^3 = 8 words (8-bit transmit data + 3-bit command)
    return n == 8;
}
