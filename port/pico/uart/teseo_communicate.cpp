module;

// for debug messages
#include <string>
// for memset
#include <cstring>
#include "hardware/gpio.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include <algorithm>
#include "hardware/regs/intctrl.h"


export module port_pico_communicate;

import port_pico_reset;


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
// how long to wait for a single character before timing out
#define UART_WAITFORREPLY_MS (40)
// forward declaration
void on_uart_rx();

// calculate 70 characters per satellite, + 60 for the status line
// many libraries limit the number of satelites to say 6
export const size_t NMEA_MAX_REPLIES  = MAX_SATELLITE_REPLIES;

uint8_t buf[BUFFSIZE]; // read buffer, intentionally not initialised

volatile bool bWantChars; // explicitely uninitialised
volatile absolute_time_t  fail_at;
int UART_IRQ = UART1_IRQ;
uint8_t *pBuf; // explicitely uninitialised

export void initialize() {
    stdio_init_all();
    uart_init(UART_PORT, UART_BAUD);
    uart_set_fifo_enabled(UART_PORT, false);
    gpio_set_function(UART_TX, GPIO_FUNC_UART);
    gpio_set_function(UART_RX, GPIO_FUNC_UART);
    // set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);
    // by default all UART interrupts off
    uart_set_irq_enables(UART_PORT, false, false);
    
    port_pico::reset_initialize();
}

void on_uart_rx() {
    uint8_t letter;

    while (uart_is_readable(UART_PORT)) {
        letter = uart_getc(UART_PORT);
        if (bWantChars) {
            fail_at = delayed_by_ms(get_absolute_time(), UART_WAITFORREPLY_MS);
            pBuf[0] = letter;

            if (pBuf[0] == 0) {
                bWantChars = false; // a null read
            }
            if ((pBuf - buf) < BUFFSIZE-1) { // if we reach max buffer size, just keep emptying any additional characters in the last position;
                pBuf++;
            }
            assert ((pBuf - buf) < BUFFSIZE);
        }
    }
}

export void write(const ::std::string& s) {
    uart_write_blocking(UART_PORT, reinterpret_cast<const uint8_t*>(s.c_str()), s.length() +1);
    return;  
}

export void read(::std::string& s) {
    memset (buf, 0, BUFFSIZE);  // initialise buffer before reading
    pBuf = buf;
    bWantChars = true;
    // enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_PORT, true, false);
    fail_at = delayed_by_ms(get_absolute_time(), UART_WAITFORREPLY_MS);
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