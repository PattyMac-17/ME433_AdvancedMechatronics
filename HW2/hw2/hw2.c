#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define servoPin 15

const uint16_t wrap = 20000;

void setServoAngle(float angle){
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    float duty_percent = 2.0f + (angle / 180.0f) * (11.5f - 1.5f);
    uint16_t level = (uint16_t)((duty_percent / 100.0f) * (float)wrap);
    pwm_set_gpio_level(servoPin, level);
}
int main()
{
    gpio_set_function(servoPin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(servoPin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 150.0f);
    pwm_config_set_wrap(&config, wrap);
    pwm_init(slice, &config, true);

    while(1){
        setServoAngle(0);
        sleep_ms(500);
        setServoAngle(180);
        sleep_ms(500);
    }
}
