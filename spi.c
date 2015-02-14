#include <stdio.h>
#include <stdlib.h>
#include <p32xxxx.h>

#include "config.h"


void SPI2_init(uint32_t dev)
{
    unsigned char rData;

    SPI2CON = 0;
    rData = SPI2BUF; //Clear the receive buffer.
    SPI2STATbits.SPIROV = 0; // clear the Overflow
    SPI2STATCLR = 0x40; // clear the Overflow

    SPI2CON = dev;
}

void SPI2_BRG(uint8_t speed)
{
     SPI2BRG = speed;
}

void SPI2_clock(uint32_t speed)
{
    uint32_t Fpb;
    uint16_t clk;

    Fpb = GetPeripheralClock();

    if (speed > (Fpb / 2))
    {
        SPI2BRG = 0; // use the maximum baud rate possible
        return;
    }
    else
    {
        clk = (Fpb / (2 * speed)) - 1;
        if (clk > 511)
            SPI2BRG = 511; // use the minimum baud rate possible
        else
            SPI2BRG = clk; // use calculated divider-baud rate
    }
}

unsigned char SPI2_write8(unsigned char data_out)
{
    SPI2BUF = data_out;   // write to buffer for TX
    while (!SPI2STATbits.SPIRBF); // wait for the receive flag (transfer complete)
    return SPI2BUF;
}

void SPI2_write16(uint16_t data_out)
{
    SPI2BUF = data_out;   // write to buffer for TX
}

unsigned char SPI2_read(void)
{
    SPI2BUF = 0x00; // dummy byte to capture the response
    while (!SPI2STATbits.SPIRBF); // wait until cycle complete
    return SPI2BUF; // return with byte read
}