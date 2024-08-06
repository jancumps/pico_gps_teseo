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

#include "hardware/i2c.h"
#define I2C_PORT (i2c0)
#define I2C_BAUD (100 * 1000)
#define I2C_SDA (16)
#define I2C_SCL (17)
#define I2C_ADDR (0x3A)
#define BUFFSIZE (1024)
#define I2C_FAIL_AFTER_EMPTY_READS (1024)


// calculate 70 characters per satellite, + 60 for the status line
// many libraries limit the number of satelites to say 6
#define NMEA_MAX_REPLIES (MAX_SATELLITE_REPLIES)

void init();
void write(const std::string& s);
void read(std::string& s);


#endif // TESEO_COMMUNICATE_H_