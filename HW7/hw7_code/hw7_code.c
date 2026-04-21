#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19



int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}

//include hardwire spi h file
//include math.h
//in initialization:
/* spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    set up cs pin with gpio_put follow this stuff on. Hw7 wiki

    float t = 0,
    t += 0.01
    float voltage = sine(2*pi*f*t) + 1 / 2 * 3.3
    //or just calculate all the voltages beforehand and feed in the array

    writeDAC(channel, voltage)
    sleep(10 ms)

    void writeDat(int channel, float v){
        //channel is A or B, float = # 0-3.3v
        uint16_t myV = v/3.3*1023
        uint8_t data[2]
        data[0] = 0b01110000
        data[1] = 0b00000000 - these two concatenated are 512 + the command bits...

        basically just do math to get the channel voltage into the right bits of the spi command

        data[0] = data[0] | (channel & 0b1)<<7 - this is how you compare our zero value and 
        //look at channel a on multimeter and you should see 1.56 volts (or something)

         cs_select(cs_pin);
        spi_write_blocking(spi, cmdbuf, 4);
        
        cs_deselect(cs_pin);

        //update this shit at 10x the desired frequency (5x multiple of nyquist) - nick actually wants 50 updates/second
    }
    
    */

