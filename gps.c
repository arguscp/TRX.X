#include <stdint.h>
#include "gps.h"
#include "config.h"
#include "util.h"

uint8_t GPSerror = 0;
uint8_t buf[60];

struct gps GPSdata;

const uint8_t setgnss[] = {
    0xB5, 0x62, 0x06, 0x3E, 0x2C, 0x00, 0x00, 0x00, 0x20,
    0x05, 0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01,
    0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03,
    0x08, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01, 0x05, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x01, 0x01, 0x06, 0x08, 0x0E,
    0x00, 0x00, 0x00, 0x01, 0x01, 0xFC, 0x11
};

const uint8_t setdm3[] = {
    0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x03,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
    0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x76
};

const uint8_t setdm6[] = {
    0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
    0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC
};

const uint8_t setNMEAoff[] = {
    0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00,
    0x00, 0xD0, 0x08, 0x00, 0x00, 0x80, 0x25, 0x00, 0x00,
    0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9A,
    0x79
};



void setupGPS()
{
    UARTConfigure(UART2, UART_ENABLE_PINS_TX_RX_ONLY);
    UARTSetLineControl(UART2, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART2, GetPeripheralClock(), 9600);
    UARTEnable(UART2, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));
}

void sendUBX(const uint8_t *MSG, uint8_t len)
{
    uint8_t i;

    while (UARTReceivedDataIsAvailable(UART2))
        UARTGetDataByte(UART2);

    U2STAbits.OERR = 0;

    for (i = 0; i < len; i++)
    {
        while (!UARTTransmitterIsReady(UART2));
        UARTSendDataByte(UART2, MSG[i]);
    }
    while (!UARTTransmissionHasCompleted(UART2));
}

uint8_t getUBX_ACK(const uint8_t *MSG)
{
    uint8_t ubxi;
    uint8_t b;
    uint8_t ackByteID = 0;
    uint8_t ackPacket[10];

    unsigned long startTime = ReadCoreTimer();

    // Construct the expected ACK packet
    ackPacket[0] = 0xB5; // header
    ackPacket[1] = 0x62; // header
    ackPacket[2] = 0x05; // class
    ackPacket[3] = 0x01; // id
    ackPacket[4] = 0x02; // length
    ackPacket[5] = 0x00;
    ackPacket[6] = MSG[2]; // ACK class
    ackPacket[7] = MSG[3]; // ACK id
    ackPacket[8] = 0; // CK_A
    ackPacket[9] = 0; // CK_B

    // Calculate the checksums
    for (ubxi = 2; ubxi < 8; ubxi++)
    {
        ackPacket[8] = ackPacket[8] + ackPacket[ubxi];
        ackPacket[9] = ackPacket[9] + ackPacket[8];
    }

    while (1)
    {

        // Test for success
        if (ackByteID > 9)
        {
            // All packets in order!
            return 1;
        }

        // Timeout if no valid response in 2 seconds
        if ((ReadCoreTimer() - startTime) > milliseconds(2000))
        {
            return 0;
        }

        // Make sure data is available to read
        if (UARTReceivedDataIsAvailable(UART2))
        {
            b = UARTGetDataByte(UART2);

            // Check that bytes arrive in sequence as per expected ACK packet
            if (b == ackPacket[ackByteID])
            {
                ackByteID++;
            }
            else
            {
                ackByteID = 0; // Reset and look again, invalid order
            }

        }
    }
}

void setGPS_disableNMEA()
{
    sendUBX(setNMEAoff, sizeof (setNMEAoff) / sizeof (uint8_t));
}

void setGPS_GNSS()
{
    int gps_set_sucess = 0;

    while (!gps_set_sucess)
    {
        sendUBX(setgnss, sizeof (setgnss) / sizeof (uint8_t));
        gps_set_sucess = getUBX_ACK(setgnss);
    }
}

void setGPS_DynamicModel3()
{
    int gps_set_sucess = 0;

    while (!gps_set_sucess)
    {
        sendUBX(setdm3, sizeof (setdm3) / sizeof (uint8_t));
        gps_set_sucess = getUBX_ACK(setdm3);
    }
}

void setGPS_DynamicModel6()
{
    int gps_set_sucess = 0;

    while (!gps_set_sucess)
    {
        sendUBX(setdm6, sizeof (setdm6) / sizeof (uint8_t));
        gps_set_sucess = getUBX_ACK(setdm6);
    }
}

void gps_get_data()
{
    int i;

    while (UARTReceivedDataIsAvailable(UART2))
        UARTGetDataByte(UART2);

    U2STAbits.OERR = 0;
    // Clear buf[i]
    for (i = 0; i < 60; i++)
    {
        buf[i] = 0; // clearing buffer
    }

    i = 0;

    unsigned long startTime = ReadCoreTimer();

    while ((i < 60) && ((ReadCoreTimer() - startTime) < milliseconds(250))) //200ms for response
    {
        if (UARTReceivedDataIsAvailable(UART2))
        {
            buf[i] = UARTGetDataByte(UART2);
            i++;
        }
    }
}

void gps_ubx_checksum(uint8_t* data, uint8_t len, uint8_t* cka,
                      uint8_t* ckb)
{
    uint8_t i;

    *cka = 0;
    *ckb = 0;
    for (i = 0; i < len; i++)
    {
        *cka += *data;
        *ckb += *cka;
        data++;
    }
}

uint8_t _gps_verify_checksum(uint8_t* data, uint8_t len)
{
    uint8_t a, b;
    gps_ubx_checksum(data, len, &a, &b);
    if (a != *(data + len) || b != *(data + len + 1))
        return 0;
    else
        return 1;
}

uint8_t gps_check_nav(void)
{
    uint8_t request[8] = {0xB5, 0x62, 0x06, 0x24, 0x00, 0x00, 0x2A, 0x84};
    sendUBX(request, 8);

    // Get the message back from the GPS
    gps_get_data();

    // Verify sync and header bytes
    if (buf[0] != 0xB5 || buf[1] != 0x62)
    {
        GPSerror = 41;
    }
    if (buf[2] != 0x06 || buf[3] != 0x24)
    {
        GPSerror = 42;
    }
    // Check 40 bytes of message checksum
    if (!_gps_verify_checksum(&buf[2], 40))
    {
        GPSerror = 43;
    }

    // Return the navigation mode and let the caller analyse it
    GPSdata.navmode = buf[8];
}

void gps_check_lock()
{
    GPSerror = 0;

    // Construct the request to the GPS
    uint8_t request[8] = {0xB5, 0x62, 0x01, 0x06, 0x00, 0x00, 0x07, 0x16};
    sendUBX(request, 8);

    // Get the message back from the GPS
    gps_get_data();
    // Verify the sync and header bits
    if (buf[0] != 0xB5 || buf[1] != 0x62)
    {
        GPSerror = 11;
    }
    if (buf[2] != 0x01 || buf[3] != 0x06)
    {
        GPSerror = 12;
    }

    // Check 60 bytes minus SYNC and CHECKSUM (4 bytes)
    if (!_gps_verify_checksum(&buf[2], 56))
    {
        GPSerror = 13;
    }

    if (GPSerror == 0)
    {
        // Return the value if GPSfixOK is set in 'flags'
        if (buf[17] & 0x01)
            GPSdata.lock = buf[16];
        else
            GPSdata.lock = 0;

        GPSdata.sats = buf[53];
    }
    else
    {
        GPSdata.lock = 0;
    }
}

void gps_get_time()
{
    GPSerror = 0;

    // Send a NAV-TIMEUTC message to the receiver
    uint8_t request[8] = {0xB5, 0x62, 0x01, 0x21, 0x00, 0x00, 0x22, 0x67};
    sendUBX(request, 8);

    // Get the message back from the GPS
    gps_get_data();

    // Verify the sync and header bits
    if (buf[0] != 0xB5 || buf[1] != 0x62)
        GPSerror = 31;
    if (buf[2] != 0x01 || buf[3] != 0x21)
        GPSerror = 32;

    if (!_gps_verify_checksum(&buf[2], 24))
    {
        GPSerror = 33;
    }

    if (GPSerror == 0)
    {
        if (GPSdata.hour > 23 || GPSdata.minute > 59 || GPSdata.second > 59)
        {
            GPSerror = 34;
        }
        else
        {
            GPSdata.hour = buf[22];
            GPSdata.minute = buf[23];
            GPSdata.second = buf[24];
        }
    }
}

void gps_get_position()
{
    GPSerror = 0;

    // Request a NAV-POSLLH message from the GPS
    uint8_t request[8] = {0xB5, 0x62, 0x01, 0x02, 0x00, 0x00, 0x03, 0x0A};
    sendUBX(request, 8);

    // Get the message back from the GPS
    gps_get_data();

    // Verify the sync and header bits
    if (buf[0] != 0xB5 || buf[1] != 0x62)
        GPSerror = 21;
    if (buf[2] != 0x01 || buf[3] != 0x02)
        GPSerror = 22;

    if (!_gps_verify_checksum(&buf[2], 32))
    {
        GPSerror = 23;
    }

    if (GPSerror == 0)
    {
        if (GPSdata.sats < 4)
        {
            GPSdata.lat = 0;
            GPSdata.lon = 0;
            GPSdata.alt = 0;
        }
        else
        {
            GPSdata.lon = (int32_t) buf[10] | (int32_t) buf[11] << 8 |
                    (int32_t) buf[12] << 16 | (int32_t) buf[13] << 24;
            GPSdata.lat = (int32_t) buf[14] | (int32_t) buf[15] << 8 |
                    (int32_t) buf[16] << 16 | (int32_t) buf[17] << 24;
            GPSdata.alt = (int32_t) buf[22] | (int32_t) buf[23] << 8 |
                    (int32_t) buf[24] << 16 | (int32_t) buf[25] << 24;
        }

        // 4 bytes of altitude above MSL (mm)

        GPSdata.alt /= 1000; // Correct to meters

    }



}


