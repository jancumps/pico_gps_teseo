module;

// for debug messages
#include <string>
// for memset
#include <cstring>
#include "hardware/gpio.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include <algorithm>

export module port_pico_communicate;

import port_pico_reset;


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
#define I2C_FAIL_AFTER_EMPTY_READS (1024U * 4)


// calculate 70 characters per satellite, + 60 for the status line
// many libraries limit the number of satelites to say 6
export const size_t NMEA_MAX_REPLIES  = MAX_SATELLITE_REPLIES;

uint8_t buf[BUFFSIZE]; // read buffer, intentionally not initialised

export void initialize() {
    stdio_init_all();
    // I2C is "open drain", pull ups to keep signal high when no data is being sent 
    // (not needed. board has pullups)
    i2c_init(I2C_PORT, I2C_BAUD);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    // gpio_pull_up(I2C_SDA);
    // gpio_pull_up(I2C_SCL);
    
    port_pico::reset_initialize();    
}

export void write(const ::std::string& s) {
    i2c_write_blocking(I2C_PORT, I2C_ADDR, reinterpret_cast<const uint8_t*>(s.c_str()), s.length() +1, false);
    return;  
}

export void read(::std::string& s) {
    memset (buf, 0, BUFFSIZE);  // initialise buffer before reading
    bool gotData = false;
    unsigned int failures = 0U;
    uint8_t *bufptr = buf;
    do {
        i2c_read_blocking(I2C_PORT, I2C_ADDR, bufptr, 1, false);
        if (*bufptr != 0xff) {
            gotData = true;
            bufptr++;
        } else if (gotData) { // we are done
            *bufptr = 0;
            bufptr = buf + BUFFSIZE;
        } else {
            *bufptr = 0;
            failures++;
        }
    }
    while ((bufptr - buf < BUFFSIZE) && (failures < I2C_FAIL_AFTER_EMPTY_READS));
    s = reinterpret_cast<const char*>(buf);
    return;
}
