#include "config.h"
#include "util.h"



void _delay_us( uint32_t delay )
{
	// note that 1 core tick = 2 SYS cycles (this is fixed)
	int us_ticks=(SYS_FREQ/1000000)/2;
	WriteCoreTimer( 0 );
	while( ReadCoreTimer() < delay*us_ticks );
} // END delay_us()


