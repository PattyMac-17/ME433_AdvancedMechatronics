#include "hx711.h"
#include <hardware/gpio.h>
#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"

int32_t hx711_read(int DATA, int CLOCK) {
    uint32_t raw = 0;
    
    while (gpio_get(DATA)) {
        tight_loop_contents();
    }

    for (int i = 0; i < 24; i++) {
        gpio_put(CLOCK, 1);
        sleep_us(1);

        raw = raw << 1;
        if (gpio_get(DATA)) {
            raw |= 1;
        }

        gpio_put(CLOCK, 0);
        sleep_us(1);
    }

    gpio_put(CLOCK, 1);
    sleep_us(1);
    gpio_put(CLOCK, 0);
    sleep_us(1);

    if (raw & 0x800000) {
        raw |= 0xFF000000;
    }

    return (int32_t)raw;
}

void hx711_IIR(int32_t* raw, int32_t* filtered, int numSamples) {
    if (numSamples <= 0) {
        return;
    }

    float A = 0.95f;
    float B = 0.05f;

    filtered[0] = raw[0];

    for (int i = 1; i < numSamples; i++) {
        filtered[i] = (int32_t)((A * filtered[i - 1]) + (B * raw[i]));
    }
}

void hx711_collectDataPoints(int32_t* rawForceDestination, uint32_t* timeDestination, int numSamples, int DATA, int CLOCK){
    uint32_t startTime = to_ms_since_boot(get_absolute_time());
    for(int i = 0; i < numSamples; i++){
        rawForceDestination[i] = hx711_read(DATA, CLOCK);
        timeDestination[i] = to_ms_since_boot(get_absolute_time()) - startTime;
    }
}

void hx711_measure(int DATA, int CLOCK, int32_t* rawDestination, int32_t* filteredDestination, uint32_t* timeDestination, int numSamples){
    hx711_collectDataPoints(rawDestination, timeDestination, numSamples, DATA, CLOCK);
    hx711_IIR(rawDestination, filteredDestination, numSamples);
}