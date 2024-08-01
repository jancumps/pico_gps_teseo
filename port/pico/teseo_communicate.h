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
#define MAX_SATELITES 7

// #define GPS_OVER_I2C  // you can set this in the CMake file
// #define GPS_OVER_UART // you can set this in the CMake file

#ifdef GPS_OVER_I2C
#ifdef GPS_OVER_UART
#error "invalid configuration. define GPS_OVER_I2C or GPS_OVER_UART"
#endif
#include "hardware/i2c.h"
#define I2C_PORT (i2c0)
#define I2C_BAUD (20 * 1000)
#define I2C_SDA (16)
#define I2C_SCL (17)
#define I2C_ADDR (0x3A)
#define BUFFSIZE (1024)
#endif

#ifdef GPS_OVER_UART
#ifdef GPS_OVER_I2C
#error "invalid configuration. define GPS_OVER_I2C or GPS_OVER_UART"
#endif
#include "hardware/uart.h"
#include <cassert>
#define UART_PORT (uart1)
#define UART_BAUD (9600)
#define UART_TX (4)
#define UART_RX (5)
// multiline replies take decent buffer size
// calculate 70 characters per satelite, + 60 for the status line
// many libraries limit the number of satelites to say 7
#define BUFFSIZE (70 * MAX_SATELITES + 60)
// multiline replies take a while at 9600 baud. 
// 400 ms is not enough for commands like GPGSV with 7 sattelites
// the time is dependent on how many sattelites are actually in sight, not restricted by MAX_SATELITES
#define UART_WAITFORREPLY_MS (500)
// forward declaration
void on_uart_rx();
int UART_IRQ = UART1_IRQ;
uint8_t *pBuf; // explicitely uninitialised
volatile bool bWantChars; // explicitely uninitialised
#endif

#ifndef GPS_OVER_I2C
#ifndef GPS_OVER_UART
#error "invalid configuration. define GPS_OVER_I2C or GPS_OVER_UART"
#endif
#endif


#define RESET_PIN (18)
#define RESET_APPLY_MS (1)
// recover must be more than 3 seconds
#define RESET_RECOVER_MS (4000)

// calculate 70 characters per satelite, + 60 for the status line
// many libraries limit the number of satelites to say 6
#define NMEA_MAX_REPLIES (MAX_SATELITES)

void write(const std::string& s);
void read(std::string& s);
void init();
void reset();


#endif // TESEO_COMMUNICATE_H_