#include "spi.h"
#include "config.h"
#include "si446x.h"
#include "stdio.h"
#include "util.h"



sendCmdReceiveAnswer(uint8_t byteCountTx, uint8_t byteCountRx, const char* pData)
{
    uint8_t reply = 0x00;
    uint8_t j=0;
    /* There was a bug in A1 hardware that will not handle 1 byte commands.
       It was supposedly fixed in B0 but the fix didn't make it at the last minute, so here we go again */
  //  if (byteCountTx == 1)
  //      byteCountTx++;

	
/*
#if defined(_DEBUG)
	printf("\nSent: ");
	for (j = 0; j < byteCountTx; j++) // Loop through the bytes of the pData
    {
      printf("%2X ",pData[j]);
    }
	printf("\n");
#endif
*/
    SS=0;
 
    for (j = 0; j < byteCountTx; j++) // Loop through the bytes of the pData
    {
      SPI2_write8(pData[j]);
    }

	SS=1;

    _delay_us(20);

    SS=0;

    
    
    while (reply != 0xFF)
    {
       SPI2_write8(0x44);
       reply = SPI2_write8(0x44);
       if (reply != 0xFF)
       {
       
         SS=1;
		 _delay_us(20);
         SS=0;
     
       }
    }

  

#ifdef _DEBUG

  //  printf("\nReceived: ");
   
    for (j = 1; j < byteCountRx; j++)
    {
      reply=SPI2_write8(0x44);
//      printf("%2X ",reply);
    }
   
 //   printf("\n");
#endif

    SS=1;
    _delay_us(40);
}



void setFrequency(uint32_t freq)
{

  // Set the output divider according to recommended ranges given in Si446x datasheet
  uint8_t outdiv = 4;
  uint8_t band = 0;
  if (freq < 705000000UL) { outdiv = 6;  band = 1;};
  if (freq < 525000000UL) { outdiv = 8;  band = 2;};
  if (freq < 353000000UL) { outdiv = 12; band = 3;};
  if (freq < 239000000UL) { outdiv = 16; band = 4;};
  if (freq < 177000000UL) { outdiv = 24; band = 5;};

  uint32_t f_pfd = 2 * VCXO_FREQ / outdiv;

  uint16_t n = ((unsigned int)(freq / f_pfd)) - 1;

  float ratio = (float)freq / (float)f_pfd;
  float rest  = ratio - (float)n;

  uint32_t m = (uint32_t)(rest * 524288UL);

// set the band parameter
  char set_band_property_command[] = {0x11, 0x20, 0x01, 0x51, (band + 8)}; //
  // send parameters
  sendCmdReceiveAnswer(5, 1, set_band_property_command);

// Set the pll parameters
  uint16_t m2 = m / 0x10000;
  uint16_t m1 = (m - m2 * 0x10000) / 0x100;
  uint16_t m0 = (m - m2 * 0x10000 - m1 * 0x100);
  // assemble parameter string
  char set_frequency_property_command[] = {0x11, 0x40, 0x04, 0x00, n, m2, m1, m0};
  // send parameters
  sendCmdReceiveAnswer(8, 1, set_frequency_property_command);

  char set_pa_pwr_lvl_property_command[] = {0x11, 0x22, 0x01, 0x01, 126};
  // send parameters
  sendCmdReceiveAnswer(5, 1, set_pa_pwr_lvl_property_command);

}


void start_tx()
{
  char change_state_command[] = {0x34, 0x07}; //  Change to TX state
  sendCmdReceiveAnswer(2, 1, change_state_command);

}

void stop_tx()
{
  char change_state_command[] = {0x34, 0x03}; //  Change to Ready state
  sendCmdReceiveAnswer(2, 1, change_state_command);

}

void tune_tx()
{
  char change_state_command[] = {0x34, 0x05}; //  Change to TX tune state
  sendCmdReceiveAnswer(2, 1, change_state_command);

}

void setModem()
{
  // Set to CW mode

  char set_modem_mod_type_command[] = {0x11, 0x20, 0x01, 0x00, 0x00};
  sendCmdReceiveAnswer(5, 1, set_modem_mod_type_command);
}

void setTuning(uint8_t val)
{
  // Set to CW mode
//SPI2BUF=val;
  char set_cap[] = {0x11, 0x00, 0x01, 0x00, val};
  sendCmdReceiveAnswer(5, 1, set_cap);
}


void radio_SI_reset()
{
 // VCXO_ON=1;

#ifdef _DEBUG
//  printf("VCXO is enabled\n");
#endif
  _delay_us(60000);

  RADIO_SDN=1;
  _delay_us(60000);
  RADIO_SDN=0;   // booting
  _delay_us(60000);

#ifdef _DEBUG
 // printf("Si446x: Powered up\n");
#endif

  // Start talking to the Si446X radio chip

  const char PART_INFO_command[] = {0x01}; // Part Info
  sendCmdReceiveAnswer(1, 9, PART_INFO_command);
//  printf("Part info was checked\n");

//divide VCXO_FREQ into its bytes; MSB first
  unsigned int x3 = VCXO_FREQ / 0x1000000;
  unsigned int x2 = (VCXO_FREQ - x3 * 0x1000000) / 0x10000;
  unsigned int x1 = (VCXO_FREQ - x3 * 0x1000000 - x2 * 0x10000) / 0x100;
  unsigned int x0 = (VCXO_FREQ - x3 * 0x1000000 - x2 * 0x10000 - x1 * 0x100);

//POWER_UP
  const char init_command[] = {0x02, 0x01, 0x01, x3, x2, x1, x0};// no patch, boot main app. img, FREQ_VCXO, return 1 byte
  sendCmdReceiveAnswer(7, 1 ,init_command);
 
//  printf("Radio booted\n");

  const char get_int_status_command[] = {0x20, 0x00, 0x00, 0x00}; //  Clear all pending interrupts and get the interrupt status back
  sendCmdReceiveAnswer(4, 9, get_int_status_command);


  const char gpio_pin_cfg_command[] = {0x13, 0x04, 0x02, 0x02, 0x02, 0x08, 0x0B, 0x00}; //  Set GPIO0 as input, all other GPIOs LOW; Link NIRQ to CTS; Link SDO to MISO; Max drive strength
  sendCmdReceiveAnswer(8, 8, gpio_pin_cfg_command);

  setFrequency(RADIO_FREQUENCY);

 
  setModem();


 // tune_tx();
}
