#include "hardware/gpio.h"
#include "reset.h"
#include "pico/stdlib.h"

void reset() {
    gpio_put(RESET_PIN, 0);
    sleep_ms(RESET_APPLY_MS);
    gpio_put(RESET_PIN, 1);
    sleep_ms(RESET_RECOVER_MS);
    return;
}


