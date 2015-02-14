/* 
 * File:   ax25.h
 * Author: Chris
 *
 * Created on 3. Januar 2015, 22:32
 */

#ifndef AX25_H
#define AX25_H

typedef struct s_addresses {
	char callsign[7];
	unsigned char ssid;
} s_address;

//void ax25_send_header(struct s_address *addresses, int num_addresses);
void ax25_send_header(s_address addresses[], int num_addresses);

void ax25_send_byte(unsigned char byte);
void ax25_send_string(const char *string);
void ax25_send_footer();
void ax25_flush_frame();

#endif
