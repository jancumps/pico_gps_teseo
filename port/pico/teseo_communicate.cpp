#include "teseo_communicate.h"
// for memset
#include <cstring>
#include "hardware/gpio.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include <algorithm>

uint8_t buf[BUFFSIZE]; // read buffer, intentionally not initialised

void init() {
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

