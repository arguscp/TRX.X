/* 
 * File:   util.h
 * Author: Chris
 *
 * Created on 3. Januar 2015, 23:02
 */

#ifndef __UTIL_H
#define	__UTIL_H

#include <stdint.h>
#include "config.h"

#define milliseconds(X) (((SYS_FREQ/2000))*X)

void _delay_us(uint32_t);


#endif	/* __UTIL_H */

