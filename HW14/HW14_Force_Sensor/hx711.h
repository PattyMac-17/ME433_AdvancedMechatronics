#ifndef HX711_H__
#define HX711_H__

#include <stdint.h>

int32_t hx711_read(int DATA, int CLOCK);
void hx711_IIR(int32_t* raw, int32_t* filtered, int numSamples);
void hx711_collectDataPoints(int32_t* rawForceDestination, uint32_t* timeDestination, int numSamples, int DATA, int CLOCK);
void hx711_measure(int DATA, int CLOCK, int32_t* rawDestination, int32_t* filteredDestination, uint32_t* timeDestination, int numSamples);

#endif