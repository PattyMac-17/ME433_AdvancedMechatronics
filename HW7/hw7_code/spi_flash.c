/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Example of reading/writing an external serial flash using the PL022 SPI interface

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

#define FLASH_PAGE_SIZE        256
#define FLASH_SECTOR_SIZE      4096

#define FLASH_CMD_PAGE_PROGRAM 0x02
#define FLASH_CMD_READ         0x03
#define FLASH_CMD_STATUS       0x05
#define FLASH_CMD_WRITE_EN     0x06
#define FLASH_CMD_SECTOR_ERASE 0x20

#define FLASH_STATUS_BUSY_MASK 0x01

static inline void cs_select(uint cs_pin) { //inline fxn = does something in assembly to make the code run faster at the cost of more space - so it doesn't get called it is copy pasted
    asm volatile("nop \n nop \n nop"); // FIXME - this is a delay (nop = do nothing for a clock cycle)- makes the chip select work
    gpio_put(cs_pin, 0); //chip select pin goes low
    asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1); //chip select pin goes high
    asm volatile("nop \n nop \n nop"); // FIXME
}

void __not_in_flash_func(flash_read)(spi_inst_t *spi, uint cs_pin, uint32_t addr, uint8_t *buf, size_t len) {
    cs_select(cs_pin);
    uint8_t cmdbuf[4] = {
            FLASH_CMD_READ,
            addr >> 16,
            addr >> 8,
            addr
    };
    spi_write_blocking(spi, cmdbuf, 4);
    spi_read_blocking(spi, 0, buf, len);
    cs_deselect(cs_pin);
}

void __not_in_flash_func(flash_write_enable)(spi_inst_t *spi, uint cs_pin) {
    cs_select(cs_pin);
    uint8_t cmd = FLASH_CMD_WRITE_EN;
    spi_write_blocking(spi, &cmd, 1);
    cs_deselect(cs_pin);
}

void __not_in_flash_func(flash_wait_done)(spi_inst_t *spi, uint cs_pin) {
    uint8_t status;
    do {
        cs_select(cs_pin);
        uint8_t buf[2] = {FLASH_CMD_STATUS, 0};
        spi_write_read_blocking(spi, buf, buf, 2);
        cs_deselect(cs_pin);
        status = buf[1];
    } while (status & FLASH_STATUS_BUSY_MASK);
}

void __not_in_flash_func(flash_sector_erase)(spi_inst_t *spi, uint cs_pin, uint32_t addr) {
    uint8_t cmdbuf[4] = {
            FLASH_CMD_SECTOR_ERASE,
            addr >> 16,
            addr >> 8,
            addr
    };
    flash_write_enable(spi, cs_pin);
    cs_select(cs_pin); //lines are clock, data in, data out, and individual chip select pins (3 unique + one select per chip - keep chip select high all the time and only send one low at a time to avoid short circuits)
    spi_write_blocking(spi, cmdbuf, 4); // your job is to write the writing fxn that will contain this write fxn -> all spi is read/write they exchange data at the same time (you tell chip what to send in cycle 1 and then it sends you what you want in cycle 2 while you tell it what you want next)
    cs_deselect(cs_pin);
    flash_wait_done(spi, cs_pin);
}

void __not_in_flash_func(flash_page_program)(spi_inst_t *spi, uint cs_pin, uint32_t addr, uint8_t data[]) {
    uint8_t cmdbuf[4] = {
            FLASH_CMD_PAGE_PROGRAM,
            addr >> 16,
            addr >> 8,
            addr
    };
    flash_write_enable(spi, cs_pin);
    cs_select(cs_pin);
    spi_write_blocking(spi, cmdbuf, 4);
    spi_write_blocking(spi, data, FLASH_PAGE_SIZE);
    cs_deselect(cs_pin);
    flash_wait_done(spi, cs_pin);
}

void printbuf(uint8_t buf[FLASH_PAGE_SIZE]) {
    for (int i = 0; i < FLASH_PAGE_SIZE; ++i) {
        if (i % 16 == 15)
            printf("%02x\n", buf[i]);
        else
            printf("%02x ", buf[i]);
    }
}

/*int main() {
    // Enable UART so we can print status output
    stdio_init_all();
#if !defined(spi_default) || !defined(PICO_DEFAULT_SPI_SCK_PIN) || !defined(PICO_DEFAULT_SPI_TX_PIN) || !defined(PICO_DEFAULT_SPI_RX_PIN) || !defined(PICO_DEFAULT_SPI_CSN_PIN)
#warning spi/spi_flash example requires a board with SPI pins
    puts("Default SPI pins were not defined");
#else

    printf("SPI flash example\n");

    // Enable SPI 0 at 1 MHz and connect to GPIOs
    spi_init(spi_default, 1000 * 1000);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    // Make the SPI pins available to picotool
    bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
    // Make the CS pin available to picotool
    bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));

    printf("SPI initialised, let's goooooo\n");

    uint8_t page_buf[FLASH_PAGE_SIZE];

    const uint32_t target_addr = 0;

    flash_sector_erase(spi_default, PICO_DEFAULT_SPI_CSN_PIN, target_addr);
    flash_read(spi_default, PICO_DEFAULT_SPI_CSN_PIN, target_addr, page_buf, FLASH_PAGE_SIZE);

    printf("After erase:\n");
    printbuf(page_buf);

    for (int i = 0; i < FLASH_PAGE_SIZE; ++i)
        page_buf[i] = i;
    flash_page_program(spi_default, PICO_DEFAULT_SPI_CSN_PIN, target_addr, page_buf);
    flash_read(spi_default, PICO_DEFAULT_SPI_CSN_PIN, target_addr, page_buf, FLASH_PAGE_SIZE);

    printf("After program:\n");
    printbuf(page_buf);

    flash_sector_erase(spi_default, PICO_DEFAULT_SPI_CSN_PIN, target_addr);
    flash_read(spi_default, PICO_DEFAULT_SPI_CSN_PIN, target_addr, page_buf, FLASH_PAGE_SIZE);

    printf("Erase again:\n");
    printbuf(page_buf);

    return 0;
#endif
}*/

//stuff about the DAC for HW7
//our chip is 2 chanels of 8 bit dac resolution (mcp4902 from microchip)
//rail to rail = if you giv e it 3.3V it can make 0-3.3V
//simultaneous latching of dual dacs on LDAC pin - send it low, wire shdn high
//settling time (how long to change volts) ~ 5 uS -> can run @ 200kHz (not fast enough for sound)
//only two registers in chip - directly control voltage divider for each channel
// use capacitors on power to condition it
//equation 4.1 shows how voltage is calculated based on input data - resolution of steps is reference /2^bit resolution - in mV and 10 bit resolution is so fine that its hidden under pico noise
//send the chip 16 bits (spi = 8 bit bytes) explained in register 5.3
// keep A/B to 1, BUF to 1, gain to 1, shdn to 1, then bit 11 thru bit 0 (counting down and ignoring final 2 since only 10 bit) are the voltage we want