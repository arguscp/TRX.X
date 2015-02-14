#include <stdio.h>
#include <math.h>
#include "config.h"
#include "spi.h"
#include "modem.h"



// Module globals
static uint8_t current_byte;
static uint8_t current_sample_in_baud; // 1 bit = SAMPLES_PER_BAUD samples
static uint16_t phase_delta; // 1200/2200 for standard AX.25
static uint16_t phase; // Fixed point 9.7 (2PI = TABLE_SIZE)
static uint16_t packet_pos; // Next bit to be sent out
static uint16_t sine_table[512];

static uint16_t PHASE_DELTA_1200; // Fixed point 9.7 // 1258 / 2516
static uint16_t PHASE_DELTA_2200; // 2306 / 4613


#define TABLE_SIZE          (sizeof(sine_table)/sizeof(sine_table[0]))
#define PLAYBACK_RATE       (SYS_FREQ / 256)    // 62.5KHz @ F_CPU=16MHz; 31.25kHz @ 8MHz
#define BAUD_RATE           1200
#define SAMPLES_PER_BAUD    (PLAYBACK_RATE / BAUD_RATE) // 52.083333333 / 26.041666667
#define TXBUFFSZ            (SAMPLES_PER_BAUD*2)
#define TXBUFFSZ_HALF       SAMPLES_PER_BAUD
#define DMA0                (0)

volatile uint16_t tx_buf[TXBUFFSZ];

// Exported globals
uint16_t modem_packet_size = 0;
uint8_t modem_packet[MODEM_MAX_PACKET];
volatile uint8_t modem_busy = 0;

void modem_setup()
{
    uint16_t i;

    PHASE_DELTA_1200 = (((TABLE_SIZE * (uint32_t) 1200) << 7) / PLAYBACK_RATE);
    PHASE_DELTA_2200 = (((TABLE_SIZE * (uint32_t) 2200) << 7) / PLAYBACK_RATE);

    // Generate modified sine Table
    for (i = 0; i < SIN_SIZE; i++)
    {
        sine_table[i] = 0x3000 | (uint16_t) (((sin(i * 2 * M_PI / SIN_SIZE) * VPP / 2 + OFFSET) / VSS)*(1 << SIN_FP));
    }

    DmaChnOpen(DMA0, DMA_CHN_PRI1, DMA_OPEN_AUTO);
    DmaChnSetEventControl(DMA0, DMA_EV_START_IRQ_EN | DMA_EV_START_IRQ(_TIMER_2_IRQ));

    DmaChnSetTxfer(DMA0, (void *) &tx_buf[0], (void*) &SPI2BUF, sizeof (tx_buf), 2, 2);
    DmaChnSetEvEnableFlags(DMA0, DMA_EV_BLOCK_DONE | DMA_EV_SRC_HALF); // enable the transfer done interrupt, when all buffer transferred

    INTSetVectorPriority(INT_VECTOR_DMA(DMA0), INT_PRIORITY_LEVEL_5); // set INT controller priority
    INTSetVectorSubPriority(INT_VECTOR_DMA(DMA0), INT_SUB_PRIORITY_LEVEL_2); // set INT controller sub-priority
    INTEnable(INT_SOURCE_DMA(DMA0), INT_ENABLED); // enable the chn interrupt in the INT controller

    DmaChnEnable(DMA0);
}

void modem_prepare_buffer(uint8_t start, uint8_t stop)
{
    uint16_t mx = 0;

    for (mx = start; mx < stop; mx++)
    {

   // If sent SAMPLES_PER_BAUD already, go to the next bit
    if (current_sample_in_baud == 0) {    // Load up next bit
      if ((packet_pos & 7) == 0)          // Load up next byte
        current_byte = modem_packet[packet_pos >> 3];
      else
        current_byte = current_byte / 2;  // ">>1" forces int conversion
      if ((current_byte & 1) == 0) {
        // Toggle tone (1200 <> 2200)
        phase_delta ^= (PHASE_DELTA_1200 ^ PHASE_DELTA_2200);
      }
    }

        phase += phase_delta;

        tx_buf[mx] = sine_table[((phase >> 7) & (TABLE_SIZE - 1))];

        if (++current_sample_in_baud == SAMPLES_PER_BAUD)
        {
            current_sample_in_baud = 0;
            packet_pos++;
        }
    }
}

void modem_send_frame()
{
    modem_busy = 1;
    phase_delta = PHASE_DELTA_2200;
    phase = 0;
    packet_pos = 0;
    current_sample_in_baud = 0;

    modem_prepare_buffer(0, TXBUFFSZ_HALF); //precalc buffer
    modem_prepare_buffer(TXBUFFSZ_HALF, TXBUFFSZ); //precalc buffer

    WriteTimer2(0x0000);
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_1, 256); // Enough speed for MCP transmission
}

// handler for the DMA channel 1 interrupt

void __ISR(_DMA0_VECTOR, IPL5AUTO) DmaHandler1(void)
{
    uint16_t evFlags; // event flags when getting the interrupt

    INTClearFlag(INT_SOURCE_DMA(DMA0)); // acknowledge the INT controller, we're servicing it

    if (packet_pos > (modem_packet_size))
    {
        CloseTimer2();
        DmaChnClrEvFlags(DMA0, DMA_EV_BLOCK_DONE | DMA_EV_SRC_HALF);
        modem_busy = 0;
        goto end;
    }

    evFlags = DmaChnGetEvFlags(DMA0); // get the event flags

    if (evFlags & DMA_EV_SRC_HALF)
    {
        DmaChnClrEvFlags(DMA0, DMA_EV_SRC_HALF);
        modem_prepare_buffer(0, TXBUFFSZ_HALF);
    }
    else if (evFlags & DMA_EV_BLOCK_DONE)
    {
        DmaChnClrEvFlags(DMA0, DMA_EV_BLOCK_DONE);
        modem_prepare_buffer(TXBUFFSZ_HALF, TXBUFFSZ);
    }
end:
    ;
}
