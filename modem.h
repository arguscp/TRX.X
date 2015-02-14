/* 
 * File:   APRS_modem.h
 * Author: Chris
 *
 * Created on 2. Dezember 2014, 20:32
 */
#ifndef __MODEM_H__
#define __MODEM_H__

#define SIN_SIZE 512
#define SIN_FP 12


extern uint8_t modem_packet[MODEM_MAX_PACKET];  // Upper layer data
extern uint16_t modem_packet_size;                // in bits
extern volatile uint8_t modem_busy;

void modem_setup();
void modem_send_frame();

#endif
