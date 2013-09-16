#ifndef DNS320L_H
#define DNS320L_H
/*

Simple system daemon for D-Link DNS-320L

(c) 2013 Andreas Boehler, andreas _AT_ aboehler.at

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <http://www.gnu.org/licenses/>.

*/

#define ERR_WRONG_ANSWER      -1
#define SUCCESS                1

#define CMD_START_MAGIC     0xfa
#define CMD_STOP_MAGIC      0xfb
#define GPIO_BUTTON_POWER     29

char DeviceReadyCmd[] =        {0xfa, 0x03, 0x01, 0x00, 0x00, 0x00, 0xfb};
char AckFromSerial[] =         {0xfa, 0x30, 0x00, 0x00, 0x00, 0x00, 0xfb};
char ThermalStatusGetCmd[] =   {0xfa, 0x03, 0x08, 0x00, 0x00, 0x00, 0xfb};
char FanStopCmd[] =            {0xfa, 0x02, 0x00, 0x00, 0x00, 0x00, 0xfb};
char FanHalfCmd[] =            {0xfa, 0x02, 0x00, 0x01, 0x00, 0x00, 0xfb};
char FanFullCmd[] =            {0xfa, 0x02, 0x00, 0x02, 0x00, 0x00, 0xfb};
char DeviceShutdownCmd[] =     {0xfa, 0x03, 0x03, 0x01, 0x01, 0x0a, 0xfb};
char APREnableCmd[] =          {0xfa, 0x03, 0x02, 0x01, 0x00, 0x00, 0xfb};
char APRDisableCmd[] =         {0xfa, 0x03, 0x02, 0x00, 0x00, 0x00, 0xfb};
char APRStatusCmd[] =          {0xfa, 0x03, 0x02, 0x02, 0x00, 0x00, 0xfb};
char PwrLedOnCmd[] =           {0xfa, 0x03, 0x06, 0x01, 0x00, 0x01, 0xfb};
char PwrLedOffCmd[] =          {0xfa, 0x03, 0x06, 0x00, 0x00, 0x01, 0xfb};
char PwrLedBlinkCmd[] =        {0xfa, 0x03, 0x06, 0x02, 0x00, 0x01, 0xfb};
char WOLStatusEnableCmd[] =    {0xfa, 0x03, 0x0a, 0x01, 0x00, 0x00, 0xfb};
char WOLStatusDisableCmd[] =   {0xfa, 0x03, 0x0a, 0x00, 0x00, 0x00, 0xfb};
char WOLStatusGetCmd[] =       {0xfa, 0x03, 0x0a, 0x02, 0x00, 0x00, 0xfb};
char RDateAndTimeCmd[] =       {0xfa, 0x01, 0x08, 0x01, 0x01, 0x00, 0xfb};
char WAlarmEnableCmd[] =       {0xfa, 0x01, 0x10, 0x02, 0x01, 0x01, 0xfb};
char WAlarmDisableCmd[] =      {0xfa, 0x01, 0x10, 0x02, 0x01, 0x00, 0xfb};
char RAlarmMonthCmd[] =        {0xfa, 0x01, 0x0a, 0x01, 0x01, 0x00, 0xfb};
char RAlarmDateCmd[] =         {0xfa, 0x01, 0x0b, 0x01, 0x01, 0x00, 0xfb};
char RAlarmHourCmd[] =         {0xfa, 0x01, 0x0c, 0x01, 0x01, 0x00, 0xfb};
char RAlarmSecondCmd[] =       {0xfa, 0x01, 0x0e, 0x01, 0x01, 0x00, 0xfb};
char RAlarmMinuteCmd[] =       {0xfa, 0x01, 0x0d, 0x01, 0x01, 0x00, 0xfb};
char RAlarmFlagCmd[] =         {0xfa, 0x01, 0x0f, 0x01, 0x01, 0x00, 0xfb};
char WAlarmMonthCmd[] =        {0xfa, 0x01, 0x0a, 0x02, 0x01, 0x00, 0xfb};
char WAlarmDateCmd[] =         {0xfa, 0x01, 0x0b, 0x02, 0x01, 0x00, 0xfb};
char WAlarmHourCmd[] =         {0xfa, 0x01, 0x0c, 0x02, 0x01, 0x00, 0xfb};
char WAlarmMinuteCmd[] =       {0xfa, 0x01, 0x0d, 0x02, 0x01, 0x00, 0xfb};
char WAlarmSecondCmd[] =       {0xfa, 0x01, 0x0e, 0x02, 0x01, 0x00, 0xfb};
char WDateAndTimeCmd[] =       {0xfa, 0x01, 0x08, 0x02, 0x07, 0x17, 0x06, 0x21, 0x02, 0x10, 0x09, 0x13, 0xfb};

const char ThermalTable[] = {0x74, 0x73, 0x72, 0x71, 0x70, 0x6F, 0x6E, 0x6D, 0x6C, 0x6B,
                 0x6A, 0x69, 0x68, 0x67, 0x66, 0x65, 0x64, 0x63, 0x62, 0x61,
                 0x60, 0x5F, 0x5E, 0x5D, 0x5C, 0x5B, 0x5A, 0x59, 0x58, 0x57,
                 0x56, 0x55, 0x54, 0x53, 0x52, 0x51, 0x50, 0x4F, 0x4E, 0x4D,
                 0x4C, 0x4B, 0x4A, 0x49, 0x48, 0x47, 0x46, 0x45, 0x44, 0x43,
                 0x42, 0x41, 0x41, 0x40, 0x3F, 0x3E, 0x3E, 0x3D, 0x3D, 0x3C,
                 0x3B, 0x3A, 0x3A, 0x39, 0x38, 0x38, 0x37, 0x36, 0x36, 0x35,
                 0x34, 0x34, 0x33, 0x33, 0x32, 0x31, 0x31, 0x30, 0x30, 0x2F,
                 0x2F, 0x2E, 0x2E, 0x2D, 0x2C, 0x2C, 0x2B, 0x2B, 0x2A, 0x2A, 
                 0x29, 0x29, 0x28, 0x28, 0x27, 0x27, 0x27, 0x26, 0x26, 0x25,
                 0x25, 0x24, 0x24, 0x23, 0x23, 0x22, 0x22, 0x21, 0x21, 0x21,
                 0x20, 0x20, 0x1F, 0x1F, 0x1E, 0x1E, 0x1E, 0x1D, 0x1D, 0x1C,
                 0x1C, 0x1B, 0x1B, 0x1B, 0x1B, 0x1A, 0x19, 0x19, 0x19, 0x18,
                 0x18, 0x17, 0x17, 0x25, 0x1B, 0x1B, 0x19, 0x19, 0x19, 0x18,
                 0x18, 0x17, 0x17, 0x16, 0x16, 0x16, 0x15, 0x15, 0x14, 0x14,
                 0x14, 0x13, 0x13, 0x12, 0x12, 0x12, 0x11, 0x11, 0x10, 0x10,
                 0x10, 0xF, 0xF, 0xE, 0xE, 0xE, 0xD, 0xD, 0xC, 0xC, 0xC, 0xB,
                 0xB, 0xA, 0xA, 9, 9, 9, 8, 8, 7, 7, 7, 6, 6, 5, 5, 4, 4, 4, 3,
                 3, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};


#endif //DNS320L_H
