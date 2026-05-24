// based on adafruit and sparkfun libraries

#include <string.h> // for memset
#include "ssd1306.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "font.h"
#include <stdlib.h>

unsigned char SSD1306_ADDRESS = 0b0111100; // 7bit i2c address
unsigned char ssd1306_buffer[513]; // 128x32/8. Every bit is a pixel except first byte

void ssd1306_setup() {
    // first byte in ssd1306_buffer is a command
    ssd1306_buffer[0] = 0x40;
    // give a little delay for the ssd1306 to power up
    //_CP0_SET_COUNT(0);
    //while (_CP0_GET_COUNT() < 48000000 / 2 / 50) {
    //}
    sleep_ms(20);
    ssd1306_command(SSD1306_DISPLAYOFF);
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);
    ssd1306_command(0x80);
    ssd1306_command(SSD1306_SETMULTIPLEX);
    ssd1306_command(0x1F); // height-1 = 31
    ssd1306_command(SSD1306_SETDISPLAYOFFSET);
    ssd1306_command(0x0);
    ssd1306_command(SSD1306_SETSTARTLINE);
    ssd1306_command(SSD1306_CHARGEPUMP);
    ssd1306_command(0x14);
    ssd1306_command(SSD1306_MEMORYMODE);
    ssd1306_command(0x00);
    ssd1306_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_command(SSD1306_COMSCANDEC);
    ssd1306_command(SSD1306_SETCOMPINS);
    ssd1306_command(0x02);
    ssd1306_command(SSD1306_SETCONTRAST);
    ssd1306_command(0x8F);
    ssd1306_command(SSD1306_SETPRECHARGE);
    ssd1306_command(0xF1);
    ssd1306_command(SSD1306_SETVCOMDETECT);
    ssd1306_command(0x40);
    ssd1306_command(SSD1306_DISPLAYON);
    ssd1306_clear();
    ssd1306_update();
}

// send a command instruction (not pixel data)
void ssd1306_command(unsigned char c) {
    //i2c_master_start();
    //i2c_master_send(ssd1306_write);
    //i2c_master_send(0x00); // bit 7 is 0 for Co bit (data bytes only), bit 6 is 0 for DC (data is a command))
    //i2c_master_send(c);
    //i2c_master_stop();

    uint8_t buf[2];
    buf[0] = 0x00;
    buf[1] =c;
    i2c_write_blocking(i2c0, SSD1306_ADDRESS, buf, 2, false);
}

// update every pixel on the screen
void ssd1306_update() {
    ssd1306_command(SSD1306_PAGEADDR);
    ssd1306_command(0);
    ssd1306_command(0xFF);
    ssd1306_command(SSD1306_COLUMNADDR);
    ssd1306_command(0);
    ssd1306_command(128 - 1); // Width

    unsigned short count = 512; // WIDTH * ((HEIGHT + 7) / 8)
    unsigned char * ptr = ssd1306_buffer; // first address of the pixel buffer
    /*
    i2c_master_start();
    i2c_master_send(ssd1306_write);
    i2c_master_send(0x40); // send pixel data
    // send every pixel
    while (count--) {
        i2c_master_send(*ptr++);
    }
    i2c_master_stop();
    */

    i2c_write_blocking(i2c0, SSD1306_ADDRESS, ptr, 513, false);
}

// set a pixel value. Call update() to push to the display)
void ssd1306_drawPixel(unsigned char x, unsigned char y, unsigned char color) {
    if ((x < 0) || (x >= 128) || (y < 0) || (y >= 32)) {
        return;
    }

    if (color == 1) {
        ssd1306_buffer[1 + x + (y / 8)*128] |= (1 << (y & 7));
    } else {
        ssd1306_buffer[1 + x + (y / 8)*128] &= ~(1 << (y & 7));
    }
}

// zero every pixel value
void ssd1306_clear() {
    memset(ssd1306_buffer, 0, 512); // make every bit a 0, memset in string.h
    ssd1306_buffer[0] = 0x40; // first byte is part of command
}

void ssd1306_drawDoubleVector(float x, float y){
    //center
    ssd1306_drawPixel(63,15,1);
    ssd1306_drawPixel(63,16,1);
    ssd1306_drawPixel(64,15,1);
    ssd1306_drawPixel(64,16,1);

    float max_g = 1.0;

    //vector in y
    int y_range = 15; //pixels
    int scaled_y = y * y_range / max_g;
    if (scaled_y > y_range) {
        scaled_y = y_range;
    }
    if (scaled_y < -y_range) {
        scaled_y = -y_range;
    }

    if(scaled_y > 0){
        for (int i = 0; i < scaled_y; i++){
            ssd1306_drawPixel(63, 14-i, 1);
            ssd1306_drawPixel(64, 14-i, 1);
        }
    }
    else{
        for (int i = 0; i < -scaled_y; i++){
            ssd1306_drawPixel(63, 17+i, 1);
            ssd1306_drawPixel(64, 17+i, 1);
        }
    }

    //vector in x
    int x_range = 63; //pixels
    int scaled_x = x * x_range / max_g;
    if (scaled_x > x_range) {
        scaled_x = x_range;
    }
    if (scaled_x < -x_range) {
        scaled_x = -x_range;
    }

    if(scaled_x > 0){
        for (int j = 0; j < scaled_x; j++){
            ssd1306_drawPixel(65+j, 15, 1);
            ssd1306_drawPixel(65+j, 16, 1);
        }
    }
    else{
        for (int j = 0; j < -scaled_x; j++){
            ssd1306_drawPixel(62-j, 15, 1);
            ssd1306_drawPixel(62-j, 16, 1);
        }
    }
}

void ssd1306_drawChar(unsigned char x, unsigned char y, char letter){
    for (int j = 0; j < 5; j++){
        for(int k = 0; k < 8; k++){
            if(((ASCII[letter - 0x20][j])>>k)&1){
                ssd1306_drawPixel(x+j, y+k, 1);
            }
            else{
                ssd1306_drawPixel(x+j, y+k, 0);
            }
        }
    }
    return;
}

void ssd1306_drawMessage(unsigned char x, unsigned char y, char *message){
    int s = 0;
    char letter;
    while (message[s] != 0){
        letter = message[s];
        unsigned char x_pos = x + (6 * s);
        ssd1306_drawChar(x_pos, y, letter);
        s++;
    }
    return;
}

void ssd1306_drawLine(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;

    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;

    int err = dx + dy;

    while (true) {
        ssd1306_drawPixel(x0, y0, 1);

        if (x0 == x1 && y0 == y1) {
            break;
        }

        int e2 = 2 * err;

        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void ssd1306_drawSingleVector(float x, float y) {
    int center_x = 64;
    int center_y = 16;

    float max_g_x = 1.0;
    float max_g_y = 0.6;

    int x_range = 63;
    int y_range = 15;

    int scaled_x = (int)(x * x_range / max_g_x);
    int scaled_y = (int)(y * y_range / max_g_y);

    if (scaled_x > x_range) {
        scaled_x = x_range;
    }
    if (scaled_x < -x_range) {
        scaled_x = -x_range;
    }

    if (scaled_y > y_range) {
        scaled_y = y_range;
    }
    if (scaled_y < -y_range) {
        scaled_y = -y_range;
    }

    int x2 = center_x + scaled_x;
    int y2 = center_y - scaled_y;

    // center marker
    ssd1306_drawPixel(63, 15, 1);
    ssd1306_drawPixel(63, 16, 1);
    ssd1306_drawPixel(64, 15, 1);
    ssd1306_drawPixel(64, 16, 1);

    // vector line
    ssd1306_drawLine(center_x, center_y, x2, y2);
}