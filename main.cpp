// https://www.st.com/resource/en/application_note/an5203-teseoliv3f--i2c-positioning-sensor--stmicroelectronics.pdf


/** @mainpage C++ library for ST Teseo GPS
*
* @authors Jan Cumps
*
* @section MAIN C++ driver for the ST Teseo-LIV3 GPS module 
* goals:
* - Teseo lib code does not need to know what the target microcontroller is.
* - Teseo lib code does not need to know if the project uses I2C or UART
* - controller and protocol functionality is provided by the user's project code. It has to plug in a reader and writer function.
* - lean, for embedded evelopment
*
* articles on element14:  
* 1: [Pico and I2C support](https://community.element14.com/technologies/embedded/b/blog/posts/c-library-for-st-teseo-gps---pt-1-pico-and-i2c-support?CommentId=a0dfd5e9-20a5-4ae6-8b1d-723620f2db3f)  
* 2: [Dynamic GPS configuration (and some other things) ](https://community.element14.com/technologies/embedded/b/blog/posts/c-library-for-st-teseo-gps---pt-2-dynamic-gps-configuration-and-some-other-things)  
*
* example:
* 
* -# main.cpp
*/

#include <string>
// for memset
#include <cstring>
// for std::find
#include <algorithm>

#include "teseo.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

// for debug messages
#include <stdio.h>
#include "pico/stdlib.h"


#define I2C_PORT (i2c0)
#define I2C_BAUD (10 * 1000)
#define I2C_SDA (16)
#define I2C_SCL (17)
#define I2C_ADDR (0x3A)
#define I2C_BUFFSIZE (180)

#define RESET_PIN (18)
#define RESET_APPLY_MS (1)
// recover must be more than 3 seconds
#define RESET_RECOVER_MS (4000)

uint8_t buf[I2C_BUFFSIZE]; // read buffer, intentionally not initialised


teseo::teseo gps;

void init () {
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
    i2c_write_blocking(i2c_default, I2C_ADDR, reinterpret_cast<const uint8_t*>(s.c_str()), s.length() +1, false);
    return;  
}

void read(std::string& s) {
    memset (buf, 0, I2C_BUFFSIZE);
    for (buf[I2C_BUFFSIZE-1] = 0; buf[I2C_BUFFSIZE-1] != 0xff;) { // in line with AN5203
      // read in one go as register addresses auto-increment
      i2c_read_blocking(i2c_default, I2C_ADDR, buf, I2C_BUFFSIZE, false);
      // find first non 0xFF. That's the start
      auto iter_begin =  std::find(std::begin(buf), std::end(buf), '$');
      // find first 0xFF. That's the end
      auto iter_end =  std::find(iter_begin, std::end(buf), 0xff);
      s = std::string(iter_begin, iter_end);
    }

    return;
}

void reset() {
    gpio_put(RESET_PIN, 0);
    sleep_ms(RESET_APPLY_MS);
    gpio_put(RESET_PIN, 1);
    sleep_ms(RESET_RECOVER_MS);
    return;
}

int main() {
    init();

    gps.getWriteCallback().set([](const std::string& s) -> void {
        write(s);
    });

    gps.getReadCallback().set([](std::string& s) -> void {
        read(s);
    });

    gps.getResetCallback().set([]() -> void {
        reset();
    });

    /*
    when the teseo is preset for i2c according to AN5203,
    init is not required, and you can cut 4s 10ms from the startup sequence
    https://www.st.com/resource/en/application_note/an5203-teseoliv3f--i2c-positioning-sensor--stmicroelectronics.pdf
    */
    gps.init();

    while (true) {
        std::string reply; // 
        gps.ask_gpgll(reply);
        printf(reply.c_str());
        sleep_ms(1000);
    }
}