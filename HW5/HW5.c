#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "font.h"
#include "ssd1306.h"
#include "hardware/adc.h"
#include "mpu6050.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5
#define HEART_BEAT 16



int main()
{
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    i2c_init(I2C_PORT, 400*1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    
    gpio_init(HEART_BEAT);
    gpio_set_dir(HEART_BEAT, GPIO_OUT);

    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    //sleep_ms(5000); // time to open terminal: screen /dev/tty.usbmodem101 115200

    printf("hi!\n");

    //sleep_ms(1000);
    
    printf("trying to initialize ssd1306\n");
    ssd1306_setup();
    printf("ssd1306 initialized\n");

    //sleep_ms(1000);

    printf("trying to initialize mpu6050\n");
    mpu6050_setup();
    printf("mpu6050 initialized\n");

    //sleep_ms(1000);

    uint8_t identifier = mpu6050_whoami();
    printf("my name is 0x%02X\n", identifier);

    //sleep_ms(1000);

    printf("entering while loop\n");

    sleep_ms(1000);
    
    char message1[50];
    char message2[50];
    char message3[50];

    float measurements[7];

    while (true) {
        //gpio_put(HEART_BEAT, 1);
        mpu6050_collect_data(measurements);

        sprintf(message1, "X acc = %.3f g", measurements[0]);
        sprintf(message2, "Y acc = %.3f g", measurements[1]);
        sprintf(message3, "Z acc = %.3f g", measurements[2]);

        ssd1306_clear();
        /*
        ssd1306_drawMessage(0, 0, message1);
        ssd1306_drawMessage(0, 8, message2);
        ssd1306_drawMessage(0, 16, message3);
        ssd1306_drawPixel(0,0,1);
        ssd1306_drawPixel(127,0,1);
        ssd1306_drawPixel(0,31,1);
        ssd1306_drawPixel(127,31,1);

        //center
        ssd1306_drawPixel(63,15,1);
        ssd1306_drawPixel(63,16,1);
        ssd1306_drawPixel(64,15,1);
        ssd1306_drawPixel(64,16,1);*/
        ssd1306_drawSingleVector(measurements[0], measurements[1]);

        ssd1306_update();

        sleep_ms(100);

        //gpio_put(HEART_BEAT, 0);

        //sleep_ms(100);
    }
}
