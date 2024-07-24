// https://www.st.com/resource/en/application_note/an5203-teseoliv3f--i2c-positioning-sensor--stmicroelectronics.pdf


/*! @mainpage C++ library for ST Teseo GPS
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
* -# main.cpp
*/

#include <string>
// for memset
#include <cstring>
// for std::find
#include <algorithm>

#include "teseo.h"
#include "hardware/gpio.h"

// for debug messages
#include <stdio.h>
#include "pico/stdlib.h"

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
#define I2C_BAUD (10 * 1000)
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

uint8_t buf[BUFFSIZE]; // read buffer, intentionally not initialised

// forward declaration
void write(const std::string& s);
void read(std::string& s);

teseo::teseo gps;
std::string reply;
std::vector<std::string> replies(NMEA_MAX_REPLIES); 
// vector size is a suggestion. STL will allocate at least NMEA_MAX_REPLIES

void init () {
    stdio_init_all();
    #ifdef GPS_OVER_I2C
    // I2C is "open drain", pull ups to keep signal high when no data is being sent 
    // (not needed. board has pullups)
    i2c_init(I2C_PORT, I2C_BAUD);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    // gpio_pull_up(I2C_SDA);
    // gpio_pull_up(I2C_SCL);
#endif // GPS_OVER_I2C
#ifdef GPS_OVER_UART
    uart_init(UART_PORT, UART_BAUD);
    uart_set_fifo_enabled(UART_PORT, false);
    gpio_set_function(UART_TX, GPIO_FUNC_UART);
    gpio_set_function(UART_RX, GPIO_FUNC_UART);
    // set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);
    // by default all UART interrupts off
    uart_set_irq_enables(UART_PORT, false, false);
#endif // GPS_OVER_UART

    
    gpio_init(RESET_PIN);
    gpio_put(RESET_PIN, 1);
    gpio_set_dir(RESET_PIN, GPIO_OUT);    
}

#ifdef GPS_OVER_I2C
void write(const std::string& s) {
    i2c_write_blocking(I2C_PORT, I2C_ADDR, reinterpret_cast<const uint8_t*>(s.c_str()), s.length() +1, false);
    return;  
}

void read(std::string& s) {
    memset (buf, 0, BUFFSIZE);  // initialise buffer before reading
    for (buf[BUFFSIZE-1] = 0; buf[BUFFSIZE-1] != 0xff;) { // TODO in line with AN5203 remove after loooong testing
      // read in one go as register addresses auto-increment
      i2c_read_blocking(I2C_PORT, I2C_ADDR, buf, BUFFSIZE, false);
      // find first non 0xFF. That's the start
      auto iter_begin =  std::find(std::begin(buf), std::end(buf), '$');
      // find first 0xFF. That's the end
      auto iter_end =  std::find(iter_begin, std::end(buf), 0xff);
      s = std::string(iter_begin, iter_end);
    }
    return;
}
#endif // GPS_OVER_I2C

#ifdef GPS_OVER_UART
void on_uart_rx() {
    uint8_t letter;
    static uint8_t previousletter;

    if(pBuf == buf) { //* initialise previousletter at each buffer start
        previousletter = 0;
    }
    while (uart_is_readable(UART_PORT)) {
        letter = uart_getc(UART_PORT);
        if (bWantChars) {
            pBuf[0] = letter;
            if (pBuf[0] == '\n') {
                if (previousletter == '\n') { // two newlines is end of conversation
                    bWantChars = false;
                }
            }
            if (pBuf[0] == 0) {
                bWantChars = false; // a null read
            }
            previousletter = letter;
            if ((pBuf - buf) < BUFFSIZE-1) { // if we reach max buffer size, just keep emptying any additional characters in the last position;
                pBuf++;
            }
            assert ((pBuf - buf) < BUFFSIZE);
        }
    }
}

void write(const std::string& s) {
    uart_write_blocking(UART_PORT, reinterpret_cast<const uint8_t*>(s.c_str()), s.length() +1);
    return;  
}

void read(std::string& s) {
    memset (buf, 0, BUFFSIZE);  // initialise buffer before reading
    pBuf = buf;
    bWantChars = true;
    // enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_PORT, true, false);
    absolute_time_t  fail_at = delayed_by_ms(get_absolute_time(), UART_WAITFORREPLY_MS);
    while (bWantChars){
        if (absolute_time_diff_us(fail_at, get_absolute_time()) >= 0) {
            bWantChars = false; // timeout
        }
    };
    // disable the UART to send interrupts
    uart_set_irq_enables(UART_PORT, false, false);
    s = std::string(reinterpret_cast<const char*>(buf));

    return;
}
#endif // GPS_OVER_YART

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
    uint count; // intentionally uninitialised
    bool valid; // intentionally uninitialised

    while (true) {
        valid = gps.ask_gll(reply);
        printf("GLL valid: %s.\r\n", valid ? "yes" : "no");
        printf(reply.c_str());

        valid = gps.ask_gsv(replies, count);
        printf("GSV valid: %s. count: %u.\r\n", valid ? "yes" : "no", count);
        // the vector may contain more string values than reported, because
        // the library doesn't discard values above the read count (for efficiency).
        // also, vector size may be larger than NMEA_MAX_REPLIES
        // you can either use a for loop that runs through the valid entries only,
        // or reset the unused slots yourself.
        // I'm doing both here.
        std::for_each(replies.begin() + count, replies.end(), [](auto &s) { 
            s = std::string(); });
        std::for_each(replies.begin(), replies.begin() + count, [](auto &s) { 
            printf(s.c_str()); });

        valid = gps.ask_gsa(replies, count);
        printf("GSA valid: %s. count: %u.\r\n", valid ? "yes" : "no", count);
        std::for_each(replies.begin() + count, replies.end(), [](auto &s) { 
            s = std::string(); });
        std::for_each(replies.begin(), replies.begin() + count, [](auto &s) { 
            printf(s.c_str()); });

        valid = gps.ask_gga(reply);
        printf("GGA valid: %s.\r\n", valid ? "yes" : "no");
        printf(reply.c_str());

        valid = gps.ask_vtg(reply);
        printf("VTG valid: %s.\r\n", valid ? "yes" : "no");
        printf(reply.c_str());

        valid = gps.ask_rmc(reply);
        printf("RMC valid: %s.\r\n", valid ? "yes" : "no");
        printf(reply.c_str());

        // example: execute a custom command that returns one line
        teseo::nmea_rr pstcmu("$PSTMNMEAREQUEST,800000,0\n\r", "$PSTMNMEAREQUEST,800000,0");
        valid = gps.ask_nmea(pstcmu, reply);
        printf("PSTMCPU valid: %s.\r\n", valid ? "yes" : "no");
        printf(reply.c_str());

        // example: execute a custom command that returns multiple lines (reusing the GSA command)
        teseo::nmea_rr gsa("$PSTMNMEAREQUEST,4,0\n\r", "$PSTMNMEAREQUEST,4,0");
        valid = gps.ask_nmea_multiple(gsa, replies, count);
        printf("custom GSA valid: %s. count: %u.\r\n", valid ? "yes" : "no", count);
        std::for_each(replies.begin() + count, replies.end(), [](auto &s) { 
            s = std::string(); });
        std::for_each(replies.begin(), replies.begin() + count, [](auto &s) { 
            printf(s.c_str()); });

        printf("\r\n");
        sleep_ms(1000);
    }
}