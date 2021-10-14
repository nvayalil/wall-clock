#include <pic.h>
#include "rtc.h"
#include "i2c.h"
#include "type_def.h"
extern volatile BYTE flag;
extern volatile unsigned char Digit[6];
unsigned char BCD[4];
#define Negative        flag.b7
#define DisplayOn       flag.b5
//
// Reading time from RTC using I2C protocol
// First data pointer of RTC is set to 0, i.e at the location of seconds
// RTC data pointer will be incremented on consecutive reading
// Time in packed bcd format, so unpacked to array "Digits", which is used in
// interrupt to display. Care should be taken Digits never go beyond 10 at any
// instant otherwise leads to unpredicatble result, while executing the interrupt
//
void GetTime(void)
{
    unsigned static char Temp;
    // Set DS3231 Address pointer to Time register and read it
    // <S><RTC_ADDR+WRITE><A(S)><DATA><A(S)>DATA>....<A(S)><P>
    IdleI2C();
    StartI2C();
    WriteI2C(RTC_ADDR | WRITE);
    WriteI2C(0x00);
    // Read timer registers of DS1337 <S><RTC_ADDR+READ><A(M)><DATA><A(M)>DATA>....<NA(M)><P>
    RestartI2C();
    WriteI2C(RTC_ADDR | READ);
    Temp = ReadI2C();                           // Seconds
    AckI2C();
    Digit[5] = Temp & 0x0F;                     // Low Niblle
    Digit[4] = Temp >> 4;                       // High Nibble

    Temp = ReadI2C();                           // Minutes
    AckI2C();
    Digit[3] = Temp & 0x0F;                     // Low Nibble
    Digit[2] = Temp >> 4;                       // High Nibble

    Temp = ReadI2C();                           // Hours
    NotAckI2C();
    StopI2C();
    HR12 = 0;
    Digit[1] = Temp & 0x0F;                     // High Nibble
    if(Temp & HR12_SET_MASK)                    // 12/24 HR format bit
    {
        HR12 = 1;
        PM = 0;
        if(Temp & PM_SET_MASK) PM = 1;          // AM/PM bit
        Temp &= PM_CLR_MASK;                    // Clear AM/PM status bit
    }
    Temp &= HR12_CLR_MASK;                      // Clear 12/24 Status bit
    Digit[0] = Temp >> 4;
}
//
// Reading temperature from the RTC via I2C bus, and convert to BCD format
//
void GetTemp(void)
{
    #define TEMP_REG 0x11
    #define NEGATIVE 0x0C
    #define _C_      0x0D
    static signed int Temperature;              // Temperature register
    //static unsigned char BCD[4];                // BCD data
    IdleI2C();                                  // Wait for bus idle
    StartI2C();                                 // Start condition on bus
    WriteI2C(RTC_ADDR | WRITE);                 // Write the RTC address
    WriteI2C(TEMP_REG);                         // Set the RTC pointer to temperature reg
    RestartI2C();                               // Restart on bus
    WriteI2C(RTC_ADDR | READ);                  // Address RTC for read
    *((unsigned char*) &Temperature + 1) = ReadI2C();   // Read MSB
    AckI2C();                                   // Ack by master
    *((unsigned char*) &Temperature + 0) = ReadI2C();   // Read LSB
    NotAckI2C();                                // Not Ack by master
    StopI2C();                                  // Stop I2C
    Temperature >>= 6;                          // Remove last 6 bits (non data bits)
    Temperature *= 5;                           // Multiply by 2.5 (or 5/2)
    Temperature += 1;                           // Round off (if last bit is 1, add 1 to next bit)
    Temperature >>= 1;                          //
    Digit[5] = _C_;                             // C
    Digit[0] = 0;
    Negative = 0;                               // Clear negative flag
    if(Temperature & 0x8000)                    // If negative, Convert to +ve and set negative flag
    {
        Temperature = ~Temperature;
        Temperature++;
        Negative = 1;
    }
    BinToBCD(Temperature);                      // Binary to BCD
    Digit[1] = BCD[0];
    if(Digit[1] == 0) Digit[1] = 10;
    if(Negative) Digit[1] = NEGATIVE;           // Add a negative sign
    Digit[2] = BCD[1];
    if(Digit[2] == 0) Digit[2] = 10;
    Digit[3] = BCD[2];
    Digit[4] = BCD[3];
}
//
//  Binary to BCD converter
//  Input: int Binary
//  Output: BCD[4] - LDS in BCD[3], MSD in BCD[0]
//
void BinToBCD(unsigned int Binary)
{
    static unsigned char Count;
    BCD[0] = 0;
    BCD[1] = 0;
    BCD[2] = 0;
    BCD[3] = 0;
    for (Count = 0; Count < 16; Count++)
    {
        if(BCD[3] >= 5) BCD[3] += 0x7B;
        if(BCD[2] >= 5) BCD[2] += 0x7B;
        if(BCD[1] >= 5) BCD[1] += 0x7B;
        if(BCD[0] >= 5) BCD[0] += 0x7B;
        {
        #asm
        bcf   _STATUS,0
        rlf   BinToBCD@Binary,f
        rlf   BinToBCD@Binary+1,f
        rlf   _BCD+3,f
        rlf   _BCD+2,f
        rlf   _BCD+1,f
        rlf   _BCD+0,f
        #endasm
        }
    }
}
//
// Initialise DS3231
// Read the Control register of DS3231, and if it is not the required value
// then update the DS3231 registers, this will privent unwanted writes in each
// time the processor starts
//
void InitDS3231(void)
{
    #define DS3231_A1M1         0x07
    #define DS3231_CTRL         0x0E
    #define DS3231_CTRL_VAL     0x00

    static unsigned char Control;
    // Set DS3131 Address pointer to control register to read it
    IdleI2C();
    StartI2C();                                 // <S><RTC_ADDR+WRITE><A(S)><DATA><A(S)>DATA>....<A(S)><P>
    Control = WriteI2C(RTC_ADDR | WRITE);       // Write
    Control = WriteI2C(DS3231_CTRL);            // Write address pionter
    // Read control register the register <S><RTC_ADDR+READ><A(M)><DATA><A(M)>DATA>....<NA(M)><P>
    RestartI2C();                               // Restart
    Control = WriteI2C(RTC_ADDR | READ);        // Read
    Control = ReadI2C();
    NotAckI2C();
    if(Control !=  DS3231_CTRL_VAL)
    {
        RestartI2C();
        WriteI2C(RTC_ADDR | WRITE);             // Write
        WriteI2C(DS3231_A1M1);

        WriteI2C(0x80);                         // A1M1
        WriteI2C(0x81);                         // A1M2
        WriteI2C(0x80);                         // A1M3
        WriteI2C(0x80);                         // A1M4

        WriteI2C(0x00);                         // A2M2
        WriteI2C(0x00);                         // A2M3
        WriteI2C(0x00);                         // A2M4

        WriteI2C(DS3231_CTRL_VAL);              // Control reg
    }
    StopI2C();
}
//
// Reset seconds
// Clear the seconds in the RTC
//
void ResetSeconds()
{
    IdleI2C();
    StartI2C();                                 // <S><RTC_ADDR+WRITE><A(S)><DATA><A(S)>DATA>....<A(S)><P>
    WriteI2C(RTC_ADDR | WRITE);                 // Address the RTC
    WriteI2C(0x00);                             // Set RTC pointer to 0x00 (seconds)
    WriteI2C(0x00);                             // Clear the RTC seconds
    StopI2C();                                  // <P>
}    
//
// Write current time shows by Digits to the RTC
// Since Digits arrays are in unpacked bcd, first to packed bcd and writes
//
void SetTime(void)
{
    // Set DS1337 Address pointer to Time register to Write
    static unsigned char Temp;
    IdleI2C();
    StartI2C();                                 // <S><RTC_ADDR+WRITE><A(S)><DATA><A(S)>DATA>....<A(S)><P>

    //Temp = Digit[4] << 4;                       // Seconds High Nibble
    //Temp |= Digit[5];                           // Seconds Low Nibble
    WriteI2C(RTC_ADDR | WRITE);
    WriteI2C(0x01);                             // Set DS3231 data pointer to 1
    //WriteI2C(Temp);                             // Write Seconds
    Temp = Digit[2] << 4;                       // Minutes High Nibble
    Temp |= Digit[3];                           // Minutes Low Nibble
    WriteI2C(Temp);                             // Write Minutes
    Temp = Digit[0] << 4;                       // Hours High Nibble
    if(HR12)
    {
        // Only need the following bits in 12HR format
        Temp |= HR12_SET_MASK;
        if(PM) Temp |= PM_SET_MASK;
    }
    Temp |= Digit[1];                           // Hours Low Nibble
    WriteI2C(Temp);                             // Write Hours with flags
    StopI2C();
}
//
// Change time format from 12 hour to 24 hour or vice versa
// Set the HR12 (12 hout / 24 hour flag) before calling this function
//
void ChangeTimeFormat(void)
{
    // It is necessory to set the 12/24 Hr (HR12) flag before calling this routine
    // Change format of time from 12 hr to 24 hour and vice versa
    if(!HR12)
    {
        // Change Time to 24 Hr Format
        if((Digit[0] != 0x01) || (Digit[1] != 0x02)) // Is it 12 hour ?
        {
            // No, if PM, add 12 to get 24 hour format time
            if(PM)
            {
                Digit[0] += 0x01;               // Add 12 hour
                Digit[1] += 0x02;
                if(Digit[1] > 0x09)             // If greater than 9, adjust bcd
                {
                    Digit[1] += 0xF6;           // Keep in bcd
                    Digit[0]++;                 // Increment next digit
                }
            }
        }
        else
        {
            // Yes, it is 12 hour, if PM, do nothing else set as 00
            if(!PM)
            {
                Digit[1] = Digit[0] = 0x00;
            }
        }
    }
    else
    {
        // Change to 12 Hr Format
        PM = 0;
        if(Digit[0] >= 0x01)
        {
            if(Digit[1] >= 0x02) PM = 1;        // For 12 hour, set PM flag
            if((Digit[1] > 0x02) || (Digit[0] >= 0x02))
            {
                // Adjust if greater than 12 hour
                Digit[0]--;                     // Subtract 12 hour
                Digit[1] -= 0x02;
                if(Digit[1] > 0x09)             // If greater than 9, adjust bcd
                {
                    Digit[1] += 0x0A;           // Keep in bcd (0-9)
                    Digit[0] --;                // Decrement next digit
                }
                PM = 1;                         // Set PM flag
            }
        }
        else
        {
            // The Digit[0] is zero
            if(Digit[1] == 0)
            {
                // Now time is 00 hours i.e. 12AM
                Digit[0] = 0x01;
                Digit[1] = 0x02;
            }
        }
    }
}
//
// Increment minutes and if upper limit reaches roll back to zero
//
void MinInc(void)
{
    if(++Digit[3] > 9)
    {
        Digit[3] = 0;
        if(++Digit[2] >= 6) Digit[2] = 0;
    }
}
//
// Increment hours, and check limits in the current time format
// Updates PM flag if necessory
//
void HourInc(void)
{
    if(++Digit[1] > 9)
    {
        Digit[1] = 0;                           // Set to zero
        Digit[0]++;                             // Inc next digit
    }
    if(HR12)
    { //12 Hour Format
        if(Digit[0] >= 1)
        {
            if(Digit[1] == 2) PM ^= 1;          // If inremented to 12, toggle AM/PM status
            if(Digit[1] > 2)
            {
                Digit[1] = 1;                   // If greater than 12 set hours to 01
                Digit[0] = 0;
            }
        }
    }
    else
    {
        if((Digit[0] >= 2) && (Digit[1] >= 4))
        {
            Digit[0] = Digit[1] = 0;            // If greater than 23 roll back to zero
        }
    }
}
