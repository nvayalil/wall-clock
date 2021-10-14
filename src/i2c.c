#include <pic.h>
#include "i2c.h"
/********************************************************************
*   Function Name:  OpenI2C                                         *
*   Return Value:   void                                            *
*   Parameters:     SSP peripheral setup bytes                      *
*   Description:    This function sets up the SSP module on a       *
*                   PIC16FXXX device for use with a Microchip I2C   *
********************************************************************/
void OpenI2C( unsigned char sync_mode, unsigned char slew )
{
  SSPSTAT   &= 0x3F;                    // power on state
  SSPCON    = 0x00;                     // power on state
  SSPCON    |= sync_mode;               // select serial mode
  SSPSTAT   |= slew;                    // slew rate on/off

  I2C_SCL   = 1;
  I2C_SDA   = 1;
  SSPCON    |= SSPENB;                  // enable synchronous serial port

}

/********************************************************************
*     Function Name:    StartI2C                                    *
*     Return Value:     void                                        *
*     Parameters:       void                                        *
*     Description:      Send I2C bus start condition.               *
********************************************************************/
void StartI2C( void )
{
    SEN = 1;                                // Initiate bus start condition
    if( ((SSPCON & 0x0F)==0x08) || ((SSPCON & 0x0F)==0x0B) )    //master mode only
    {
        while(SEN);                         // Automatically cleared by hardware.
    }
}
/********************************************************************
*     Function Name:    RestartI2C                                  *
*     Return Value:     void                                        *
*     Parameters:       void                                        *
*     Description:      Send I2C bus restart condition.             *
********************************************************************/
void RestartI2C(void)
{
    RSEN = 1;                           // initiate bus restart condition
    while(RSEN);                        // Automatically cleared by hardware.
}
/********************************************************************
*     Function Name:    StopI2C                                     *
*     Return Value:     void                                        *
*     Parameters:       void                                        *
*     Description:      Send I2C bus stop condition.                *
********************************************************************/
void StopI2C( void )
{
    PEN = 1;                            // initiate bus stop condition
    while(PEN);                         // Automatically cleared by hardware.
}

/********************************************************************
*     Function Name:    WriteI2C                                    *
*     Return Value:     Status byte for WCOL detection.             *
*     Parameters:       Single data byte for I2C bus.               *
*     Description:      This routine writes a single byte to the    *
*                       I2C bus.                                    *
********************************************************************/
unsigned char WriteI2C(unsigned char data_out)
{
    SSPBUF = data_out;              // write single byte to SSPBUF
    if (WCOL)                       // test if write collision occurred
    return (-1);                    // if WCOL bit is set return negative #
    else
    {
        while(BF);                  // wait until write cycle is complete
        IdleI2C();                  // ensure module is idle
        if (ACKSTAT)                // test for ACK condition received
            return (-2);            // return NACK
        else return (0);            // return ACK
    }
}

/********************************************************************
*     Function Name:    ReadI2C                                     *
*     Return Value:     contents of SSPBUF register                 *
*     Parameters:       void                                        *
*     Description:      Read single byte from I2C bus.              *
********************************************************************/
unsigned char ReadI2C( void )
{
    RCEN = 1;                       // enable master for 1 byte reception
    while (!BF);                    // wait until byte received
    while(RCEN);                    // wait until receive sequence is over
    return (SSPBUF);                // return with read byte
}
/**********************************************************************************************
Function Prototype : void IdleI2C(void)
Description        : This function generates Wait condition until I2C bus is Idle.
Arguments          : None
Return Value       : None
Remarks            : This function will be in a wait state until Start Condition Enable bit,
                     Stop Condition Enable bit, Receive Enable bit, Acknowledge Sequence
                     Enable bit of I2C Control register and Transmit Status bit I2C Status
                     register are clear. The IdleI2C function is required since the hardware
                     I2C peripheral does not allow for spooling of bus sequence. The I2C
                     peripheral must be in Idle state before an I2C operation can be initiated
                     or write collision will be generated.
***********************************************************************************************/
void IdleI2C( void )
{
  while ( ( SSPCON2 & 0x1F ) || (RW) )
     continue;
}

/********************************************************************
*     Function Name:    NotAckI2C                                   *
*     Return Value:     void                                        *
*     Parameters:       void                                        *
*     Description:      Initiate NOT ACK bus condition.             *
********************************************************************/
void NotAckI2C( void )
{
    ACKDT = 1;                      // set acknowledge bit for not ACK
    ACKEN = 1;                      // initiate bus acknowledge sequence
    while(ACKEN);
}

/********************************************************************
*     Function Name:    AckI2C                                      *
*     Return Value:     void                                        *
*     Parameters:       void                                        *
*     Description:      Initiate ACK bus condition.                 *
********************************************************************/
void AckI2C( void )
{
    ACKDT = 0;                      // set acknowledge bit state for ACK
    ACKEN = 1;                      // initiate bus acknowledge sequence
    while(ACKEN);                   // wait until ACK sequence is over
}
