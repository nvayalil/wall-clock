#define   I2C_SCL               RC3             // I2C SCL
#define   I2C_SDA               RC4             // I2C SDA

/* SSPCON1 REGISTER */
#define   SSPENB                0b00100000      /* Enable serial port and configures SCK, SDO, SDI*/
#define   SLAVE_7               0b00000110      /* I2C Slave mode, 7-bit address*/
#define   SLAVE_10              0b00000111      /* I2C Slave mode, 10-bit address*/
#define   MASTER                0b00001000      /* I2C Master mode */
#define   MASTER_FIRMW          0b00001011      // I2C Firmware Controlled Master mode (slave Idle)
#define   SLAVE_7_STSP_INT      0b00001110      // I2C Slave mode, 7-bit address with Start and Stop bit interrupts enabled
#define   SLAVE_10_STSP_INT     0b00001111      // I2C Slave mode, 10-bit address with Start and Stop bit interrupts enabled

/* SSPSTAT REGISTER */
#define   SLEW_OFF              0b10000000      /* Slew rate disabled for 100kHz mode */
#define   SLEW_ON               0b00000000      /* Slew rate enabled for 400kHz mode  */

/********************************************************************
*   Function Name:  OpenI2C                                         *
*   Return Value:   void                                            *
*   Parameters:     SSP peripheral setup bytes                      *
*   Description:    This function sets up the SSP module on a       *
*                   PIC16FXXX device for use with a Microchip I2C   *
********************************************************************/
void OpenI2C( unsigned char sync_mode, unsigned char slew );

/********************************************************************
*     Function Name:    StartI2C                                    *
*     Return Value:     void                                        *
*     Parameters:       void                                        *
*     Description:      Send I2C bus start condition.               *
********************************************************************/
void StartI2C( void );

/********************************************************************
*     Function Name:    RestartI2C                                  *
*     Return Value:     void                                        *
*     Parameters:       void                                        *
*     Description:      Send I2C bus restart condition.             *
********************************************************************/
void RestartI2C(void);

/********************************************************************
*     Function Name:    StopI2C                                     *
*     Return Value:     void                                        *
*     Parameters:       void                                        *
*     Description:      Send I2C bus stop condition.                *
********************************************************************/
void StopI2C( void );

/********************************************************************
*     Function Name:    WriteI2C                                    *
*     Return Value:     Status byte for WCOL detection.             *
*     Parameters:       Single data byte for I2C bus.               *
*     Description:      This routine writes a single byte to the    *
*                       I2C bus.                                    *
********************************************************************/
unsigned char WriteI2C(unsigned char data_out);
/********************************************************************
*     Function Name:    ReadI2C                                     *
*     Return Value:     contents of SSPBUF register                 *
*     Parameters:       void                                        *
*     Description:      Read single byte from I2C bus.              *
********************************************************************/
unsigned char ReadI2C( void );
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
void IdleI2C( void );
/********************************************************************
*     Function Name:    NotAckI2C                                   *
*     Return Value:     void                                        *
*     Parameters:       void                                        *
*     Description:      Initiate NOT ACK bus condition.             *
********************************************************************/
void NotAckI2C( void );
/********************************************************************
*     Function Name:    AckI2C                                      *
*     Return Value:     void                                        *
*     Parameters:       void                                        *
*     Description:      Initiate ACK bus condition.                 *
********************************************************************/
void AckI2C( void );
