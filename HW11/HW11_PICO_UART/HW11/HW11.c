#include <hardware/structs/io_bank0.h>
#include <pico/time.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "string.h"

// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart1
#define BAUD_RATE 115200

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define HEART_BEAT 16



int main()
{
    stdio_init_all();

    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    gpio_init(HEART_BEAT);
    gpio_set_dir(HEART_BEAT, GPIO_OUT);
    
    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART
    
    // Send out a string, with CR/LF conversions
    //uart_puts(UART_ID, " Hello, UART!\n");
    
    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart
    while (true) {
        if (uart_is_readable(UART_ID)) {
            char input = uart_getc(UART_ID);
            if (input == '\r') {
                // ignore - wire uses '\n' as line terminator
            } else if (input == '\n') {
                printf("\r\n");
            } else {
                printf("%c", input);
            }
        }

        int output = getchar_timeout_us(0);
        if (output != PICO_ERROR_TIMEOUT) {
            if (output == '\r' || output == '\n') {
                printf("\r\n");
                uart_putc(UART_ID, '\n');
            } else {
                putchar(output);
                uart_putc(UART_ID, output);
            }
        }
    }
}
