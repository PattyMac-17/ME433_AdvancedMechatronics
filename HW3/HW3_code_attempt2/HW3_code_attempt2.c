#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5
#define HEART_BEAT 16
#define CHIP 0100000 // A0, A1, A2 are all grounded
#define IODIR 0x00
#define CHIP_GPIO 0x09
#define OLAT 0x0A

void startTimer();
void setPin(unsigned char address, unsigned char register, unsigned char value);
unsigned char readPin(unsigned char address, unsigned char register);
void setupChip();

int main()
{
    stdio_init_all();

    startTimer();
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    //gpio_pull_up(I2C_SDA);
    //gpio_pull_up(I2C_SCL);

    gpio_init(HEART_BEAT);
    gpio_set_dir(HEART_BEAT, GPIO_OUT);

    setupChip();
    sleep_ms(1000);
    setPin(CHIP, OLAT, 0xFF);
    printf("BLUE LED should be on"); //but it's not


    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    while (true) {
        gpio_put(HEART_BEAT, 1);
        sleep_ms(500);
        gpio_put(HEART_BEAT, 0);
        sleep_ms(500);
    }
}

void startTimer(){
    //time to open serial screen
    //printf("8\n");
    //sleep_ms(1000);
    printf("7\n");
    sleep_ms(1000);
    printf("6\n");
    sleep_ms(1000);
    printf("5\n");
    sleep_ms(1000);
    printf("4\n");
    sleep_ms(1000);
    printf("3\n");
    sleep_ms(1000);
    printf("2\n");
    sleep_ms(1000);
    printf("1\n");
    sleep_ms(1000);
    printf("0: Beginning Program\n\n");

    return;
}

void setupChip(){
    //set IODIR
    uint8_t io_buf[2];
    io_buf[0] = IODIR;
    io_buf[1] = 0x00;
    i2c_write_blocking(I2C_PORT, CHIP, io_buf, 2, false);

    //set OLAT
    uint8_t ol_buf[2];
    ol_buf[0] = OLAT;
    ol_buf[1] = 0x00;
    i2c_write_blocking(I2C_PORT, CHIP, ol_buf, 2, false);

    return;
}

void setPin(unsigned char address, unsigned char reg, unsigned char value){
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;
    i2c_write_blocking(I2C_PORT, address, buf, 2, false);
}
