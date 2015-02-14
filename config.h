/* 
 * File:   config.h
 * Author: Chris
 *
 * Created on 30. Oktober 2014, 18:57
 */

#ifndef CONFIG_H
#define	CONFIG_H

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#define _DEBUG
#define _VERBOSE 2

#include "typedef.h"
#include <stdint.h>
#include <plib.h>


#define SYS_FREQ (16000000L)
#define GetPeripheralClock()   (SYS_FREQ/(1 << OSCCONbits.PBDIV))
#define GetInstructionClock()  (SYS_FREQ)


#define UART_MODULE_ID UART1

#define SS LATBbits.LATB10
#define VCXO_ON LATBbits.LATB14
#define RADIO_SDN LATBbits.LATB5



#define MODEM_MAX_PACKET 512

#define VSS 3.3001f
#define OFFSET (2.3f)
#define VPP 1.0f




// --------------------------------------------------------------------------
// AX25 config (ax25.c)
// --------------------------------------------------------------------------

//#define DEBUG_AX25

// --------------------------------------------------------------------------
// APRS config (aprs.c)
// --------------------------------------------------------------------------

// Set your callsign and SSID here. Common values for the SSID are
// (from http://zlhams.wikidot.com/aprs-ssidguide):
//
// - Balloons:  11
// - Cars:       9
// - Buoy:       8
// - Home:       0
// - IGate:      5
//
#define S_CALLSIGN      "DL1VV"
#define S_CALLSIGN_ID   2

// Destination callsign: APRS (with SSID=0) is usually okay.
//#define D_CALLSIGN      "APZ"  // APZ = Experimental
#define D_CALLSIGN      "BEACON"  // APExxx = Telemetry devices

#define D_CALLSIGN_ID   0


// APRS Symbol
// Consists of Symbol table (/ or \ or a dew overlay symbols) and the symbol ID
#define APRS_SYMBOL_TABLE '/' // Default table
//#define APRS_SYMBOL_TABLE '\\' // Secondary table (needs to be escaped => two \)

//#define APRS_SYMBOL_ID    '>' // /O = Car
#define APRS_SYMBOL_ID    'O' // /O = Balloon
//#define APRS_SYMBOL_ID    'N' // \N = Buoy

// Digipeating paths:
// (read more about digipeating paths here: http://wa8lmf.net/DigiPaths/ )
// The recommended digi path for a balloon is WIDE2-1 or pathless. The default
// is to use WIDE2-1. Comment out the following two lines for pathless:
//#define DIGI_PATH1      "WIDE1"
//#define DIGI_PATH1_TTL  1
//#define DIGI_PATH2      "WIDE2"
//#define DIGI_PATH2_TTL  1

// If we want to pass selected packets through the International Space Station
#define DIGI_PATH1_SAT "ARISS"
#define DIGI_PATH1_TTL_SAT   0
//#define DIGI_PATH2_SAT "SGATE"
//#define DIGI_PATH2_TTL_SAT   0


// APRS comment: this goes in the comment portion of the APRS message. You
// might want to keep this short. The longer the packet, the more vulnerable
// it is to noise.
#define APRS_COMMENT    "Dies_ist_die_Currywurst_Bedingung"


#endif	/* CONFIG_H */

