/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsp/board_api.h"
#include "tusb.h"

#include "usb_descriptors.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "font.h"
#include "ssd1306.h"
#include "hardware/gpio.h"
#include "mpu6050.h"

#define SQUARE_SPEED 5
#define IMU_SPEED 20
#define SQUARE_TIME 10
#define POLL_INTERVAL 10
#define BUTTON_PIN 17
#define LED_PIN 16
#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
    BLINK_NOT_MOUNTED = 250, // fast = device code broken
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);
void hw6_hardware_setup(void);

/*------------- MAIN -------------*/
int main(void) {
    board_init(); // initialize imu and oled here

    hw6_hardware_setup(); // screen /dev/tty.usbmodem101 115200
    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);

    if (board_init_after_tusb) {
        board_init_after_tusb();
    }

    while (1) {
        tud_task(); // tinyusb device task - this is really important
        led_blinking_task();

        hid_task(); // our code goes in here
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
    blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
    blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
    blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn) {
    // skip if hid is not ready yet
    if (!tud_hid_ready())
        return;

    switch (report_id) {
    case REPORT_ID_KEYBOARD: {
        // use to avoid send multiple consecutive zero report for keyboard
        static bool has_keyboard_key = false;

        if (btn) {
            uint8_t keycode[6] = {0};
            keycode[0] = HID_KEY_A;

            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
            has_keyboard_key = true;
        } else {
            // send empty key report if previously has key pressed
            if (has_keyboard_key)
                tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
            has_keyboard_key = false;
        }
    } break;

    case REPORT_ID_MOUSE: // we're here
        // objectives
        // 1) read if a button is pushed to select modes
        // 2) mode 1 is standard, mode 2 is mouse movement
        {
            float measurements[7];
            mpu6050_collect_data(measurements);

            static int dir = 0;
            static int time = 0;
            static bool square_mode = false;
            static bool last_button_pressed = false;

            int deltax;
            int deltay;

            bool button_pressed = !gpio_get(BUTTON_PIN);
            if (button_pressed && !last_button_pressed) {
                square_mode = !square_mode;
            }
            last_button_pressed = button_pressed;

            if (square_mode) {
                gpio_put(LED_PIN, 1); // led on = square mode
                if (dir == 0) {
                    // move right
                    deltax = SQUARE_SPEED;
                    deltay = 0;
                }
                if (dir == 1) {
                    // move up
                    deltax = 0;
                    deltay = SQUARE_SPEED;
                }
                if (dir == 2) {
                    // move left
                    deltax = -SQUARE_SPEED;
                    deltay = 0;
                }
                if (dir == 3) {
                    // move down
                    deltax = 0;
                    deltay = -SQUARE_SPEED;
                }
                time++;
                if (time == SQUARE_TIME) {
                    dir++;
                    time = 0;
                }
                if (dir == 4) {
                    dir = 0;
                }
            } else {
                gpio_put(LED_PIN, 0);
                deltax = mpu6050_helper_bin_accel(measurements[0], IMU_SPEED);
                deltay = mpu6050_helper_bin_accel(1.2 * measurements[1], IMU_SPEED);
            }
            // int8_t const delta = SQUARE_SPEED; // by default we are moving 5,5 from top left
            // corner
            //  - so we read the imu to put stuff here

            // no button, right + down, no scroll, no pan
            tud_hid_mouse_report(
                REPORT_ID_MOUSE, 0x00, deltax, deltay, 0,
                0); // the report takes the report id, then an 8 bit number for buttons, then two
                    // signed 8 bit ints for x y movement, then scroll wheels
        }
        break;

    case REPORT_ID_CONSUMER_CONTROL: {
        // use to avoid send multiple consecutive zero report
        static bool has_consumer_key = false;

        if (btn) {
            // volume down
            uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
            tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
            has_consumer_key = true;
        } else {
            // send empty key report (release key) if previously has key pressed
            uint16_t empty_key = 0;
            if (has_consumer_key)
                tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
            has_consumer_key = false;
        }
    } break;

    case REPORT_ID_GAMEPAD: {
        // use to avoid send multiple consecutive zero report for keyboard
        static bool has_gamepad_key = false;

        hid_gamepad_report_t report = {
            .x = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0, .hat = 0, .buttons = 0};

        if (btn) {
            report.hat = GAMEPAD_HAT_UP;
            report.buttons = GAMEPAD_BUTTON_A;
            tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

            has_gamepad_key = true;
        } else {
            report.hat = GAMEPAD_HAT_CENTERED;
            report.buttons = 0;
            if (has_gamepad_key)
                tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
            has_gamepad_key = false;
        }
    } break;

    default:
        break;
    }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void) {
    // Poll every 10ms
    const uint32_t interval_ms =
        POLL_INTERVAL; // runs every 10 ms (10 times/s) you can update this to go faster (smoother)
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return; // not enough time
    start_ms += interval_ms;

    uint32_t const btn = board_button_read();

    // Remote wakeup
    if (tud_suspended() && btn) {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    } else {
        // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
        send_hid_report(REPORT_ID_MOUSE, btn); // probably need to change
    }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len) {
    (void)instance;
    (void)len;

    uint8_t next_report_id = report[0] + 1u;

    if (next_report_id < REPORT_ID_COUNT) {
        send_hid_report(next_report_id, board_button_read());
    }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type,
                               uint8_t *buffer, uint16_t reqlen) {
    // TODO not Implemented
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type,
                           uint8_t const *buffer, uint16_t bufsize) {
    (void)instance;

    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        // Set keyboard LED e.g Capslock, Numlock etc...
        if (report_id == REPORT_ID_KEYBOARD) {
            // bufsize should be (at least) 1
            if (bufsize < 1)
                return;

            uint8_t const kbd_leds = buffer[0];

            if (kbd_leds & KEYBOARD_LED_CAPSLOCK) {
                // Capslock On: disable blink, turn led on
                blink_interval_ms = 0;
                board_led_write(true);
            } else {
                // Caplocks Off: back to normal blink
                board_led_write(false);
                blink_interval_ms = BLINK_MOUNTED;
            }
        }
    }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void) {
    static uint32_t start_ms = 0;
    static bool led_state = false;

    // blink is disabled
    if (!blink_interval_ms)
        return;

    // Blink every interval ms
    if (board_millis() - start_ms < blink_interval_ms)
        return; // not enough time
    start_ms += blink_interval_ms;

    board_led_write(led_state);
    led_state = 1 - led_state; // toggle
}

void hw6_hardware_setup() {
    // sleep_ms(5000); // screen /dev/tty.usbmodem101 115200

    // printf("beginning hardware initialization\n");

    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    gpio_init(BUTTON_PIN);
    gpio_init(LED_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    gpio_pull_up(BUTTON_PIN);

    // printf("pins initialized\n");

    ssd1306_setup();
    printf("ssd1306 initialized\n");
    mpu6050_setup();
    printf("mpu6050 initialized\n");

    // sleep_ms(1000);

    // uint8_t identifier = mpu6050_whoami();
    // printf("my name is 0x%02X\n", identifier);
}