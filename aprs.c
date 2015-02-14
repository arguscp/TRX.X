#include "config.h"
#include "aprs.h"
#include "ax25.h"


// changed from const
s_address addresses[] =
{
    {D_CALLSIGN, D_CALLSIGN_ID},  // Destination callsign
    {S_CALLSIGN, S_CALLSIGN_ID},  // Source callsign (-11 = balloon, -9 = car)
#ifdef DIGI_PATH1
    {DIGI_PATH1, DIGI_PATH1_TTL}, // Digi1 (first digi in the chain)
#endif
#ifdef DIGI_PATH2
    {DIGI_PATH2, DIGI_PATH2_TTL}, // Digi2 (second digi in the chain)
#endif
};



// Exported functions
void aprs_send()
{
    char temp[12];
  ax25_send_header(addresses, sizeof(addresses)/sizeof(s_address));

  // TODO:
ax25_send_string("!");
ax25_send_string("5334.10N");
ax25_send_byte('/');

ax25_send_string("01004.51E");
ax25_send_byte('O');
snprintf(temp, 4, "%03d", (int)(270 + 0.5));
   ax25_send_string(temp);             // Course (degrees)
   ax25_send_byte('/');                // and

  snprintf(temp, 4, "%03d", (int)(5 + 0.5));
   ax25_send_string(temp);             // speed (knots)
   ax25_send_string("/A=");            // Altitude (feet). Goes anywhere in the comment area

  snprintf(temp, 7, "%06ld", (long)(120));
   ax25_send_string(temp);
   ax25_send_string("/Ti=");

    //ax25_send_string(itoa(sensors_int_lm60(), temp, 10));

 //ax25_send_string(itoa(sensors_ext_lm60(), temp, 10));
   ax25_send_byte(' ');
   ax25_send_string(APRS_COMMENT);     // Comment
   ax25_send_footer();
  ax25_flush_frame();                 // Tell the modem to go
}

