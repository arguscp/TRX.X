/* 
 * File:   gps.h
 * Author: Chris
 *
 * Created on 8. Januar 2015, 18:58
 */

#ifndef GPS_H
#define	GPS_H

struct gps
{
    uint8_t navmode ;
    uint8_t lock;
    uint8_t sats;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    int32_t lat;
    int32_t lon;
    int32_t alt;
};

void setupGPS(void);
void setGPS_disableNMEA(void);
void setGPS_DynamicModel3(void);
void gps_check_lock(void);
void gps_get_time(void);
void gps_get_position(void);

#endif	/* GPS_H */

