#include "teseo_communicate.h"
// for memset
#include <cstring>
#include "hardware/gpio.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include <algorithm>
#include "reset.h"


uint8_t buf[BUFFSIZE]; // read buffer, intentionally not initialised

void init() {
    stdio_init_all();
    // I2C is "open drain", pull ups to keep signal high when no data is being sent 
    // (not needed. board has pullups)
    i2c_init(I2C_PORT, I2C_BAUD);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    // gpio_pull_up(I2C_SDA);
    // gpio_pull_up(I2C_SCL);
    
    gpio_init(RESET_PIN);
    gpio_put(RESET_PIN, 1);
    gpio_set_dir(RESET_PIN, GPIO_OUT);    
}

void write(const std::string& s) {
    i2c_write_blocking(I2C_PORT, I2C_ADDR, reinterpret_cast<const uint8_t*>(s.c_str()), s.length() +1, false);
    return;  
}

void read(std::string& s) {
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
            failures++;
        }
    }
    while ((bufptr - buf < BUFFSIZE) || (failures == I2C_FAIL_AFTER_EMPTY_READS));
    s = reinterpret_cast<const char*>(buf);
    return;
}

