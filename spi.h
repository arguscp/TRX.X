/* 
 * File:   spi.h
 * Author: Chris
 *
 * Created on 30. Oktober 2014, 21:52
 */

#ifndef __SPI_H
#define	__SPI_H
#include <stdint.h>

enum device { SI446X = 0x00008320 , MCP4129 = 0x10008720};

void SPI2_init(uint32_t);
void SPI2_clock(int);
void SPI2_BRG(uint8_t);
unsigned char SPI2_write8(unsigned char);
void SPI2_write16(uint16_t);
unsigned char SPI2_read(void);

#endif	/* __SPI_H */

