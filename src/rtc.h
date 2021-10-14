#define HR12_SET_MASK   0x40
#define HR12_CLR_MASK   0xBF
#define PM_SET_MASK     0x20
#define PM_CLR_MASK     0xDF
#define RTC_ADDR        0xD0
#define WRITE           0x00
#define READ            0x01

#define HR12            flag.b1
#define PM              flag.b2
//
// Initialise DS3231
// Read the Control register of DS3231, and if it is not the required value
// then update the DS3231 registers, this will privent unwanted writes in each
// time the processor starts
//
void InitDS3231(void);
//
// Reading time from RTC using I2C protocol
// First data pointer of RTC is set to 0, i.e at the location of seconds
// RTC data pointer will be incremented on consecutive reading
// Time in packed bcd format, so unpacked to array "Digits", which is used in
// interrupt to display. Care should be taken Digits never go beyond 10 at any
// instant otherwise leads to unpredicatble result, while executing the interrupt
//
void GetTime(void);
//
// Reset seconds
// Clear the seconds in the RTC
//
void ResetSeconds();
//
// Write current time shows by Digits to the RTC
// Since Digits arrays are in unpacked bcd, first to packed bcd and writes
//
void SetTime(void);
//
// Change time format from 12 hour to 24 hour or vice versa
// Set the HR12 (12 hout / 24 hour flag) before calling this function
//
void ChangeTimeFormat(void);
//
// Increment minutes and if upper limit reaches roll back to zero
//
void MinInc(void);
//
// Increment hours, and check limits in the current time format
// Updates PM flag if necessory
//
void HourInc(void);
//
// Reading temperature from the RTC via I2C bus, and convert to BCD format
//
void GetTemp(void);
//
//  Binary to BCD converter
//  Input: int Binary
//  Output: BCD[4] - LDS in BCD[3], MSD in BCD[0]
//
void BinToBCD(unsigned int Binary);
