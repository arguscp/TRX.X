/* 
 * File:   main.c
 * Author: Chris
 *
 * Created on 30. Oktober 2014, 18:51
 */

#include <stdio.h>
#include <stdlib.h>

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#include <plib.h>

#include <xc.h>
#include "util.h"

#include "aprs.h"

// DEVCFG3
// USERID = No Setting
#pragma config PMDL1WAY = OFF           // Peripheral Module Disable Configuration (Allow multiple reconfigurations)
#pragma config IOL1WAY = OFF            // Peripheral Pin Select Configuration (Allow multiple reconfigurations)
#pragma config FUSBIDIO = OFF           // USB USID Selection (Controlled by Port Function)
#pragma config FVBUSONIO = OFF          // USB VBUS ON Selection (Controlled by Port Function)

// DEVCFG2
#pragma config FPLLIDIV = DIV_1         // PLL Input Divider (2x Divider)
#pragma config FPLLMUL = MUL_16         // PLL Multiplier (20x Multiplier)
#pragma config UPLLIDIV = DIV_12        // USB PLL Input Divider (12x Divider)
#pragma config UPLLEN = OFF             // USB PLL Enable (Disabled and Bypassed)
#pragma config FPLLODIV = DIV_4         // System PLL Output Clock Divider (PLL Divide by 8)

// DEVCFG1
#pragma config FNOSC = PRIPLL           // Oscillator Selection Bits (Primary Osc w/PLL (XT+,HS+,EC+PLL))
#pragma config FSOSCEN = OFF            // Secondary Oscillator Enable (Disabled)
#pragma config IESO = ON                // Internal/External Switch Over (Enabled)
#pragma config POSCMOD = XT             // Primary Oscillator Configuration (XT osc mode)
#pragma config OSCIOFNC = OFF           // CLKO Output Signal Active on the OSCO Pin (Disabled)
#pragma config FPBDIV = DIV_1           // Peripheral Clock Divisor (Pb_Clk is Sys_Clk/1)
#pragma config FCKSM = CSECME           // Clock Switching and Monitor Selection (Clock Switch Enable, FSCM Enabled)
#pragma config WDTPS = PS1048576        // Watchdog Timer Postscaler (1:1048576)
#pragma config WINDIS = OFF             // Watchdog Timer Window Enable (Watchdog Timer is in Non-Window Mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (WDT Disabled (SWDTEN Bit Controls))
#pragma config FWDTWINSZ = WINSZ_25     // Watchdog Timer Window Size (Window Size is 25%)

// DEVCFG0
#pragma config JTAGEN = OFF             // JTAG Enable (JTAG Disabled)
#pragma config ICESEL = ICS_PGx1        // ICE/ICD Comm Channel Select (Communicate on PGEC1/PGED1)
#pragma config PWP = OFF                // Program Flash Write Protect (Disable)
#pragma config BWP = OFF                // Boot Flash Write Protect bit (Protection Disabled)
#pragma config CP = OFF                 // Code Protect (Protection Disabled)


#include "config.h"
#include "spi.h"
#include "modem.h"




void initDebugUART()
{
     __XC_UART=1;

     UARTConfigure(UART_MODULE_ID, UART_ENABLE_PINS_TX_RX_ONLY);
     UARTSetLineControl(UART_MODULE_ID, UART_DATA_SIZE_8_BITS |UART_PARITY_NONE | UART_STOP_BITS_1);
     UARTSetDataRate(UART_MODULE_ID, GetPeripheralClock(), 115200);
     UARTEnable(UART_MODULE_ID, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));
}


void map_peripherals()
{

   PORTSetPinsDigitalIn(IOPORT_A, BIT_4);   //UART1 RX
   PORTSetPinsDigitalOut(IOPORT_B, BIT_4);  //UART1 TX

   PORTSetPinsDigitalOut(IOPORT_B, BIT_10); //SPI2 SS
   PORTSetPinsDigitalOut(IOPORT_B, BIT_11); //SPI2 SDO
   PORTSetPinsDigitalIn(IOPORT_B, BIT_13);  //SPI2 SDI
   PORTSetPinsDigitalOut(IOPORT_B, BIT_15); //SPI2 SCK

   PORTSetPinsDigitalOut(IOPORT_B, BIT_14); //VCXO enable
   PORTSetPinsDigitalOut(IOPORT_B, BIT_5); //SI4x6x enable
   PORTSetPinsDigitalOut(IOPORT_B, BIT_9); //SS2

 //  PORTSetPinsDigitalOut(IOPORT_B, BIT_0); //GPS_TX
 //  PORTSetPinsDigitalIn(IOPORT_A, BIT_1);  //GPS_RX

   PPSUnLock;
    //UART1
    PPSInput(3,U1RX,RPA4);
    PPSOutput(1,RPB4,U1TX);

    //SPI2
    PPSInput(3,SDI2,RPB13);
    PPSOutput(2,RPB11,SDO2);

    //SS
    PPSOutput(4,RPB9,SS2);

    //UART2
  //  PPSInput(2,U2RX,RPA1);
 //   PPSOutput(4,RPB0,U2TX);

   PPSLock;
}




int main(int argc, char** argv)
{
    uint16_t tune=0;

    SYSTEMConfig(SYS_FREQ, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);   //enable maximum processor performance
    DDPCONbits.JTAGEN = 0;                                          //disable JTAG

    ANSELA = 0x0000; // all ports digital
    ANSELB = 0x0000; // all ports digital
  
    map_peripherals();

    /* enable multi vector interrupts */
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    INTEnableInterrupts();

 
   // uart_init_highspeed();
    initDebugUART();

    
    RADIO_SDN=1;           // turn off SI446x

    SPI2_init(MCP4129);
    SPI2_clock(100000);
    SPI2_write16(0x3FFF);  //Set lowest capacitance startup condition for SI osc

    _delay_us(10000);       // wait 10ms

    SPI2_init(SI446X);
    SPI2_clock(100000);

    radio_SI_reset();
   
    setTuning(0);
    tune_tx();
    start_tx();
    
    printf("Device up and running!\n");

    

    

    SPI2_init(MCP4129);
    SPI2_BRG(0x05);         // Enough speed for MCP transmission
    SPI2_write16(0x3888);

    modem_setup();

   // setupGPS();
    _delay_us(100000);
   // setGPS_disableNMEA();
    _delay_us(100000);
  //  setGPS_DynamicModel3();
    _delay_us(100000);

  while(1)
  {



      

        aprs_send();
      
/*

        modem_setup();
        
      for(tune=0;tune<500;tune+=9)
      {
          modem_packet[tune]=0x00;
          modem_packet[tune+1]=0x00;
            modem_packet[tune+2]=0x00;
          modem_packet[tune+3]=0x00;
                   modem_packet[tune+4]=0xFF;
          modem_packet[tune+5]=0xFF;
            modem_packet[tune+6]=0xFF;
          modem_packet[tune+7]=0xFF;
      }
     modem_packet_size=10;
 



    modem_send_frame();
*/
    _delay_us(1000000);
  }

 

    return (EXIT_SUCCESS);
}

