module;

#include "hardware/gpio.h"
#include "pico/stdlib.h"

export module port_pico_reset;

export namespace port_pico {


#define RESET_PIN (18)
#define RESET_APPLY_MS (1)
// recover must be more than 3 seconds
#define RESET_RECOVER_MS (4000)


void reset() {
    gpio_put(RESET_PIN, 0);
    sleep_ms(RESET_APPLY_MS);
    gpio_put(RESET_PIN, 1);
    sleep_ms(RESET_RECOVER_MS);
    return;
}

void reset_initialize() {
    gpio_init(RESET_PIN);
    gpio_put(RESET_PIN, 1);
    gpio_set_dir(RESET_PIN, GPIO_OUT); 
}

}