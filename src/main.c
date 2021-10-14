/****************************************************************************************
*               DS3231 Extremely Accurate Digital Clock                                 *
*                                                                                       *
*       Date            : Monday, 10 May 2010                                           *
*       Author          : C.V.Niras/VU3CNS                                              *
*       Copyright       : (C) 2010 C. V. Niras                                          *
*       Email           : cvniras@gmail.com                                             *
*       Processor       : 16F873A                                                       *
*       First Release   : --/01/2011                                                    *
*       Ver             : 0.1                                                           *
*   Change History:                                                                     *
*       Rev     Date        Description                                                 *
*                                                                                       *
*****************************************************************************************/
#include <pic.h>
#include "type_def.h"
#include "i2c.h"
#include "rtc.h"
__CONFIG(WDTDIS & XT & UNPROTECT & PWRTEN & LVPDIS & BORDIS);
__IDLOC7('0','.','0','1');
//Defintions
#define COM0    RA0
#define COM1    RA1
#define COM2    RA2
#define COM3    RA3
#define COM4    RC1
#define COM5    RA5
#define SET_KEY RC2
#define UP_KEY  RC5
#define DP_LED  RC0
#define ON      1
#define OFF     0
#define MIN_ON  5
#define TIME_OUT_TIME   30  // in sec
#define _XTAL_FREQ      4000000
#define I2C_BAUD        100000
#define SSPADD_VAL      (_XTAL_FREQ /(4*I2C_BAUD) - 1)

// F L A G S
#define Read        flag.b0
//#define HR12          flag.b1                         // Defined in rtc.h
//#define PM            flag.b2                         // Defined in rtc.h
#define Menu            flag.b3
#define Colon           flag.b4
#define DisplayOn       flag.b5
#define DispTemp        flag.b6
//#define Negative      flag.b7                         // Defined in rtc.c

// V A R I A B L E S
volatile unsigned char Digit[6];
volatile BYTE flag;
volatile unsigned char TimeOut;
volatile near unsigned char SetKeyCount, UpKeyCount;    // Data will be placed in common memory, so these can be
                                                        // accessed in asm without knowing the current bank selectecd
unsigned char CursorDigit;

// F U N C T I O N   P R O T O T Y P E S
void AdjustTime(void);
void UpKeyDelay(unsigned char ms);
void SetKeyDelay(unsigned char ms);

void main(void)
{
    TRISA   = 0x10;             // All are o/p, but RA4 is connected to RC1 (due to PCB fault)
    TRISB   = 0x01;             // All are o/p, except RB0
    TRISC   = 0xFC;             // RC0, RC1 are output
    CMCON   = 0x07;             // Disable comparators
    ADCON1  = 0x06;             // All are digital i/o
    OPTION  = 0x81;             // RB0 fallind edge INT, TMR0 Presc 1:4

    PORTA   = 0x00;             // Clear ports
    PORTB   = 0x00;
    PORTC   = 0x00;

    flag._byte = 0;             // Clear all flags
    OpenI2C(MASTER, SLEW_OFF);  // I2C Master, slew rate off for 100kHz
    SSPADD  = SSPADD_VAL;       // Set 100kHz I2C
    InitDS3231();               // Initialise RTC
    INTCON  = 0xA0;             // GIE, TMR0IE
    DisplayOn = 1;              // Turn on the displays
    Read    = 1;                // Read now
    while(1)
    {
        if(Read)                // Is the time for a read?
        {
            Read = 0;           // Y. Reset flag
            if(!DispTemp)
                GetTime();      // Read from RTC and update digits
            if(DispTemp)
                GetTemp();      // Read temperature from the RTC
            while(UpKeyCount != 0);
        }
        if(SetKeyCount & 0x80) AdjustTime();    // If key pressed for 1 sec, go to adjust time
        if(UpKeyCount >= MIN_ON)
        {
            TimeOut = 10;       // Display temperature for a 10 sec
            DispTemp ^= 1;      // Toggle the flag
            Read = 1;           // Read from RTC
        }
    }// End of main while loop
}//End of main

// F U N C T I O N S

//
// Function Adjust time
// Change current time using SET KEY, and UP KEY
// SET KEY isused to change selection, and up key to increment selected digit
// The Selected digit will be blinked, as long as the Menu is true;
// Exit from this is when the selection reaches one full rotation or by time out
//
void AdjustTime(void)
{
    CursorDigit = 0;
    Menu = 1;
    while(SetKeyCount != 0);    // Wait, till user releases the Key
    TimeOut = TIME_OUT_TIME;
    do
    {
        if(SetKeyCount > MIN_ON)
        {
            if(++CursorDigit >= 4) Menu = 0;    // Reached the end, exit
            SetKeyDelay(250);
        }
        if(UpKeyCount > MIN_ON)
        {
            switch(CursorDigit)
            {
                case 0:
                    if(Digit[4] >= 0x03) MinInc();  // Inc minutes and/or
                    ResetSeconds();                 // Clear Seconds
                    SetTime();                      // Set time to RTC
                break;

                case 1:
                    MinInc();                       // Increment Minutes
                    SetTime();                      // Set time to RTC
                break;

                case 2:
                    HourInc();                      // Increment Hour
                    SetTime();                      // Set time to RTC
                break;

                case 3:
                    DisplayOn = 0;                  // Temporoly off, becuase not show time in this mode
                    GetTime();                      // Read current time from RTC
                    HR12 ^= 1;                      // Toggle HR 12 Format
                    ChangeTimeFormat();             // Change Time Format, depend on the HR12 flag
                    SetTime();
                break;
            }// End of switch(CursorDigit)
            UpKeyDelay(250);
        }// End of if(UpKeyCount > MIN_ON)
        if(CursorDigit != 3)
        {
            if(Read)
            {
                Read = 0;
                GetTime();                          // Get time from the RTC
            }
        }
        else
        {
            if(HR12)
            {
                Digit[0] = 1; Digit[1] = 2;         // Show 12 in the clock
            }
            else
            {
                Digit[0] = 2; Digit[1] = 4;         // Show 24 in the clock
            }
            Digit[2] = 11;                          // H
            Digit[3] = Digit[4] = Digit[5] = 10;    // All others are blank
            DisplayOn = 1;                          // Display 12H or 24H
        }
    }while(Menu);
}
//
// Function Key delay
// Wait here till UP Key released or for the specified time (ms)
//
void UpKeyDelay(unsigned char ms)
{
    TimeOut = TIME_OUT_TIME;                        // Set time out time
    do
    {
        if(UpKeyCount == 0) return;                 // If key released, exit
        __delay_ms(1);                              // Delay for 1 ms
    }while(--ms != 0);
}
//
// Function Key delay
// Wait here till UP Key released or for the specified time (ms)
//
void SetKeyDelay(unsigned char ms)
{
    TimeOut = TIME_OUT_TIME;
    do
    {
        if(SetKeyCount == 0) break;
        __delay_ms(1);
    }while(--ms != 0);
}
//
//          I N T E R R U P T   S E R V I C E   R O U T I N E
//
// Timer 0 interrupt is used to multiplex common 7 segment displays
// Timer 0 interrupt is set at rate of ~1kHz, and at each interrup display
// switched to next
// External interrupt is at 1Hz from the RTC and it initialise reading of RTC
// by setting Read flag
//
void interrupt isr(void)
{
    // Table for converting BCD to 7 Segment LED Display format, if the digit is 10, gives a blank
    unsigned char const SevenSeg[14] = {0x7E, 0x0C, 0xB6, 0x9E, 0xCC, 0xDA, 0xFA, 0x0E, 0xFE, 0xDE, 0x00, 0xEC, 0x80, 0x72};    // Common anode
    unsigned char const TenHour[3] = {0x00, 0x0C, 0xB6}; // Ten hour will be blank for zero                         // Common anode
    static COM_STATES ComState;
    static unsigned char Counter;
    if(TMR0IF)
    {
        TMR0IF = 0; // Interrupt @ 1.024 ms
        if(++Counter >= 8)
        {
            Counter = 0;
            // Increment, but not beyond 255
            #asm
            incfsz  _SetKeyCount,W
            movwf   _SetKeyCount
            incfsz  _UpKeyCount,W
            movwf   _UpKeyCount
            #endasm
            if(SET_KEY) SetKeyCount = 0;    // Clear if key is not pressed
            if(UP_KEY) UpKeyCount = 0;      // Clear if key is not pressed
        }
        if(++ComState > C5) ComState = C0;  // Reset the state to beginning
        DP_LED = OFF;
        switch(ComState) // Display one by one (multiplexing 7 seg displays)
        {
            case C0:
            COM5 = OFF;
            PORTB = TenHour[Digit[0]];
            //PORTB = SevenSeg[Digit[0]];
            if(HR12 && (!PM)) DP_LED = ON;  // 7 seg dp is not connected here, instead parallel LED for AM
            if(DisplayOn)
                COM0 = ON;
            break;

            case C1:
            COM0 = OFF;
            PORTB = SevenSeg[Digit[1]];
            if(Colon)   DP_LED = ON;        // DP is used as colon, lit it C1, C2, C3. C4 states
            if(DisplayOn)
                COM1 = ON;
            break;

            case C2:
            COM1 = OFF;
            PORTB = SevenSeg[Digit[2]];
            if(Colon)   DP_LED = ON;        // DP is used as colon, lit it C1, C2, C3. C4 states
            if(DisplayOn)
                COM2 = ON;
            break;

            case C3:
            COM2 = OFF;
            PORTB = SevenSeg[Digit[3]];
            if(Colon)   DP_LED = ON;        // DP is used as colon, lit it C1, C2, C3. C4 states
            if(DispTemp) DP_LED = ON;       // Decimal point for temperature display
            if(DisplayOn)
                COM3 = ON;
            break;

            case C4:
            COM3 = OFF;
            PORTB = SevenSeg[Digit[4]];
            if(Colon)   DP_LED = ON;        // DP is used as colon, lit it C1, C2, C3. C4 states
            if(DisplayOn)
                COM4 = ON;
            break;

            case C5:
            COM4 = OFF;
            if(HR12 && PM) DP_LED = ON;     // 7 seg dp is not connected here, instead parallel LED for PM
            PORTB = SevenSeg[Digit[5]];
            if(DisplayOn)
                COM5 = ON;
            break;
        }
        if(!DispTemp)                       // No colon for temperature display
            Colon = 1;                      // On colon
        if(Menu)                            // Creates a blinking Digits for the menu
        {
            if(RB0)                         // RB0 high for an half seconds (SQ. Wave o/p is connected to RB0)
            {
                if(CursorDigit == 0)        // Cursor Points to Seconds
                    COM4 = COM5 = OFF;      // Blink seconds
                if(CursorDigit == 1)        // Cursor Points to Minutes
                    COM3 = COM2 = OFF;      // Blink Minutes
                if(CursorDigit >= 2)        // Cursor Points to Hours or Time Format
                    COM1 = COM0 = OFF;      // Blink hours
                if(CursorDigit == 3)        // Cursor points to time format selection
                    COM2 = OFF;             // Blink hour ('H')
            }// if(RB0)
            if(CursorDigit == 3)            // Turn off cursor for the hour format selection menu
                Colon = 0;                  //   
        }// if(Menu)
        if(RB0) Colon = 0;                  // Turn off colon for an half second
    }
    if(INTF)
    {
        // Interrupt @ 1 sec, SQW o/p of RTC is connected to RB0
        INTF = 0;
        if(TimeOut != 0)                    // Decrement if not equal to zero
        {
            if(--TimeOut == 0)
            {
                Menu = 0;                   // When it reaches zero, exit from set time menu
                DispTemp = 0;               // If displays temperatur, back to time mode
            }
        }
        Read = 1;                           // Time to read from RTC
    }
}
