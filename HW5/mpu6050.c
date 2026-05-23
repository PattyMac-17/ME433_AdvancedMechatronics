#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "mpu6050.h"

void mpu6050_command(unsigned char reg, unsigned char data) {
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = data;
    i2c_write_blocking(i2c0, MPU6050_ADDRESS, buf, 2, false);
}

void mpu6050_setup() {
    mpu6050_command(PWR_MGMT_1, 0x00);
    mpu6050_command(ACCEL_CONFIG, 0x00);
    mpu6050_command(GYRO_CONFIG, 0x18);
}

uint8_t mpu6050_whoami() {
    uint8_t whoami_value;
    mpu6050_read(WHO_AM_I, &whoami_value, 1);
    return whoami_value;
}

void mpu6050_read(unsigned char reg, uint8_t* destination, int numReads) {
    i2c_write_blocking(i2c0, MPU6050_ADDRESS, &reg, 1, true);
    i2c_read_blocking(i2c0, MPU6050_ADDRESS, destination, numReads, false);
}

void mpu6050_convert(int16_t* rawValues, float* cleanValues){
    //convert acceleration to g
    cleanValues[0] = (float)rawValues[0] * 0.000061; //x acc
    cleanValues[1] = (float)rawValues[1] * 0.000061; //y acc
    cleanValues[2] = (float)rawValues[2] * 0.000061; //z acc

    //convert temp to kelvin
    cleanValues[3] = ((float)rawValues[3] / 340.0) + 36.53 + 273.15; //kelvin

    //convert gyroscope to degrees/second
    cleanValues[4] = (float)rawValues[4] * 0.00763; //x rot
    cleanValues[5] = (float)rawValues[5] * 0.00763; //y rot
    cleanValues[6] = (float)rawValues[6] * 0.00763; //z rot
}

void mpu6050_collect_data(float* niceData){
    uint8_t rawData[14];
    int16_t values[7];

    mpu6050_read(ACCEL_XOUT_H, rawData, 14);

    values[0] = (int16_t)(rawData[0]<<8 | rawData [1]); //x acc
    values[1] = (int16_t)(rawData[2]<<8 | rawData [3]); //y acc
    values[2] = (int16_t)(rawData[4]<<8 | rawData [5]); //z acc
    values[3] = (int16_t)(rawData[6]<<8 | rawData [7]); //temp
    values[4] = (int16_t)(rawData[8]<<8 | rawData [9]); //x rot
    values[5] = (int16_t)(rawData[10]<<8 | rawData [11]); //y rot
    values[6] = (int16_t)(rawData[12]<<8 | rawData [13]); //z rot

    mpu6050_convert(values, niceData);
}