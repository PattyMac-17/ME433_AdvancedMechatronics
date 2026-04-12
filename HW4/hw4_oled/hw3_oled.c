#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "font.h"
#include "ssd1306.h"
#include "hardware/adc.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5
#define HEART_BEAT 15



int main()
{
    stdio_init_all();

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

   // sleep_ms(5000); //time to open screen in terminal

    

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    
    gpio_init(HEART_BEAT);
    gpio_set_dir(HEART_BEAT, GPIO_OUT);

    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    //printf("trying to initialize ssd1306\n");
    ssd1306_setup();
    //printf("ssd1306 initialized\n");
    
    char letter = 'A';
    char lerr = 'B';

    //int i = 15;
    char message1[50];
    char message2[50];
    char message3[50];
    char count[50];
    //sprintf(message, "my var = %d", i);

    printf("%c", letter);

    unsigned int t = to_us_since_boot(get_absolute_time());
    long i = 0;
    long j = 0;
    long k = 0;
    int delta_us;
    double FPS = 0;
    uint16_t adc0_value;
    float volts;
    
    while (true) {
        unsigned int startCycle = to_us_since_boot(get_absolute_time());

        uint16_t value = adc_read();
        volts = value * 3.3f / 4095.0f;

        sprintf(message1, "Current value = %ld", i);
        sprintf(message2, "Current value = %ld", j);
        sprintf(message3, "ADC voltage = %.2f V", volts);

        ssd1306_drawMessage(0, 0, message1);
        ssd1306_drawMessage(0, 8, message2);
        ssd1306_drawMessage(0, 16, message3);

        ssd1306_update();

        i++;
        j = j + 2;
        k = k + 4;

        delta_us = to_us_since_boot(get_absolute_time()) - startCycle;
        FPS = 1000000.0 / delta_us;

        sprintf(count, "FPS = %.1f", FPS);
        ssd1306_drawMessage(0, 24, count);
        //ssd1306_update();
        

        //sleep_ms(100);

        //gpio_put(HEART_BEAT, 1);
        //ssd1306_drawChar(0,0,letter);
        //ssd1306_drawChar(6,0,lerr);
        //ssd1306_drawMessage(0, 9, message);
        //ssd1306_update();
        //sleep_ms(500);

        //gpio_put(HEART_BEAT, 0);
        //ssd1306_clear();
        //ssd1306_update();
        //sleep_ms(500);
    }
}
