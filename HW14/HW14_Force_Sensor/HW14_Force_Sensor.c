#include <hardware/gpio.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hx711.h"

#define DATA 6
#define CLOCK 7
#define NUM_SAMPLES 1000
#define PRECOUNT 7

int main()
{
    stdio_init_all();

    gpio_init(CLOCK);
    gpio_init(DATA);
    gpio_set_dir(CLOCK, GPIO_OUT);
    gpio_set_dir(DATA, GPIO_IN);
    gpio_put(CLOCK, 0);

    printf("Starting in:\n");
    for(int i = PRECOUNT; i > 0; i--){
        printf("%d\n", i);
        sleep_ms(1000);
    }
    printf("Entering loop...");
    /*
    int32_t rawData[NUM_SAMPLES];
    int32_t filteredData[NUM_SAMPLES];
    uint32_t timeStamp[NUM_SAMPLES];

    printf("Collecting data...\n");
    hx711_measure(DATA, CLOCK, rawData, filteredData, timeStamp, NUM_SAMPLES);
    printf("Data collection finished.\n");

    printf("time_ms,raw,filtered\n");
    for (int i = 0; i < NUM_SAMPLES; i++) {
        printf("%lu,%ld,%ld\r\n", timeStamp[i], rawData[i], filteredData[i]);
    }
    */
   
    while (true) {
        int numMeasurements = 0;
        int result = scanf("%d", &numMeasurements);

        int32_t rawData[numMeasurements];
        int32_t filteredData[numMeasurements];
        uint32_t timeStamp[numMeasurements];

        hx711_measure( DATA, CLOCK, rawData, filteredData, timeStamp, numMeasurements);

        for (int i = 0; i < numMeasurements; i++) {
            printf("%lu,%ld,%ld\r\n", timeStamp[i], rawData[i], filteredData[i]);
        }
    }
}
