#ifndef TESEO_COMMUNICATE_H_
#define TESEO_COMMUNICATE_H_

// for debug messages
#include <string>

// for the moment, the library restricts how many sattelites it entertains.
// it influences the size of the read buffer (not a drama, this is a static buffer)
// it also influences the size of the vector that will accept replies that are "per sattelite"
// Currently, the code does not allow that the vector that holds these, grows (focus on embedded)
// later, this can be changed to allow flex, if you accept the dynamic 
// memory growth impact (acceptable for larger systems like PC, processors, ...)
#define MAX_SATELLITE_REPLIES 7

#include "hardware/uart.h"
#include <cassert>
#define UART_PORT (uart1)
#define UART_BAUD (9600)
#define UART_TX (4)
#define UART_RX (5)
// multiline replies take decent buffer size
// calculate 70 characters per nmea replies, + 60 for the status line
// many libraries limit the number of satelites to say 7
#define BUFFSIZE (70 * MAX_SATELLITE_REPLIES + 60)
// multiline replies take a while at 9600 baud. 
// 400 ms is not enough for commands like GPGSV with 7 sattelites
// the time is dependent on how many sattelites are actually in sight, 
// not restricted by MAX_SATELLITE_REPLIES
#define UART_WAITFORREPLY_MS (500)
// forward declaration
void on_uart_rx();

// calculate 70 characters per satellite, + 60 for the status line
// many libraries limit the number of satelites to say 6
#define NMEA_MAX_REPLIES (MAX_SATELLITE_REPLIES)

void write(const std::string& s);
void read(std::string& s);
void init();

#endif // TESEO_COMMUNICATE_H_