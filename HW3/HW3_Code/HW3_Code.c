#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments

// MCP23008 Var Add pins all low -> address = 0100 000 R/W (0 = W, 1 = R) - 8 bits

// send data: i2c_write_blocking(i2c_default, ADDR, buf, 2, false);
// note that buff is a 2 byte array, byte 1 = register and byte 2 = value to write to register

// read data: i2c_write_blocking(i2c_default, ADDR, &reg, 1, true);  // true to keep host control of bus
// i2c_read_blocking(i2c_default, ADDR, &buf, 1, false);  // false - finished with bus
// note that reg is the 8bit register you want to read from, and buf is the 8bit number you are getting from the chip

//SDA on pico pin 6, SCL on pico pin 7, heartbeat on pin 20 (GP15)

//step 1: initialize i2c chip and blink led on chip GP7
//step 2: read button on i2c chip GP0 and control GP7 led

#define I2C_PORT i2c0
#define I2C_SDA 6
#define I2C_SCL 7
#define picoHeart 15
#define ADDRESS 0x20

//prototypes
void setPin(uint8_t addr, uint8_t reg, uint8_t val){
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = val;
    i2c_write_blocking(i2c_default, addr, buf, 2, false);
}
unsigned char readPin(uint8_t addr, uint8_t reg, uint8_t val);

void initializeChip(){
    //IODIR = 0x00
    uint8_t buf1[2];
    buf1[0] = 0x00;
    buf1[1] = 0b00000001;
    i2c_write_blocking(i2c_default, ADDRESS, buf1, 2, false);

    //OLAT = 0x0A, set all LOW
    uint8_t buf2[2];
    buf2[0] = 0x0A;
    buf2[1] = 0b00000000;
    i2c_write_blocking(i2c_default, ADDRESS, buf2, 2, false);
}

int main()
{
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_init(15);
    gpio_set_dir(15, GPIO_OUT);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    initializeChip();
    sleep_ms(100);

    uint8_t led = 0b10000000;
    setPin(ADDRESS, 0x0A, led);

    bool beat = false;

    while (true) {
        
        beat = !beat;
        gpio_put(15, beat);
        sleep_ms(100);
    }
}

//functions: initialize chip, read, write
