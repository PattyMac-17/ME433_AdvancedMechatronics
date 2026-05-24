#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "font.h"
#include "ssd1306.h"
#include "mpu6050.h"

#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5

int main()
{
    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_setup();
    mpu6050_setup();

    char messages[2][50];

    float measurements[7];
    char letter;

    while (true)
    {
        mpu6050_collect_data(measurements);

        for (int i = 0; i < 2; i++)
        {
            if (i == 0)
            {
                letter = 'X';
            }
            else
            {
                letter = 'Y';
            }
            sprintf(messages[i], "%c acc = % .3f g", letter, measurements[i]);
        }

        printf("%.3f,%.3f\n", measurements[0], measurements[1]);

        ssd1306_clear();

        ssd1306_drawMessage(0, 16, messages[0]);
        ssd1306_drawMessage(0, 24, messages[1]);

        ssd1306_update();

        sleep_ms(50);
    }
}
