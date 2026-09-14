#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/i2c.h"
#include "MCP4728_lib.h"
#include "hardware/pio.h"
#include <stdbool.h>
#include "pulse.pio.h"

#include "helpers.h"
#include "string.h"



static PIO pio = pio0;
static uint sm;
static uint offset;


#include "hardware/flash.h"
#include "hardware/sync.h"

#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

void save_values(const uint8_t* system_values, size_t size)
{    
    uint32_t interrupts = save_and_disable_interrupts();

    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(
        FLASH_TARGET_OFFSET,
        system_values,
        size
    );

    restore_interrupts(interrupts);
}

void load_values(int* system_values, size_t size)
{    
    const uint8_t *data =
        (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);

    memcpy(system_values, data, size);
}

bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void scan_test() {
    printf("\nI2C Bus Scan\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_blocking(i2c1, addr, &rxdata, 1, false);
        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n");
}

int initi2c() { 
    i2c_init(I2C_PORT, 100*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    return 0;
}

int pulse_init(void)
{
    // Load the PIO program
    offset = pio_add_program(pio, &pulse_program);

    // Claim a state machine
    sm = pio_claim_unused_sm(pio, true);

    // Configure GPIO
    pio_gpio_init(pio, PULSE_PIN);
    pio_sm_set_consecutive_pindirs(pio, sm, PULSE_PIN, 1, true);

    pio_sm_config c = pulse_program_get_default_config(offset);

    // GPIO controlled by SET PINS
    sm_config_set_set_pins(&c, PULSE_PIN, 1);

    // 125 MHz / 125 = 1 MHz
    // Therefore each PIO instruction takes 1 us.
    sm_config_set_clkdiv(&c, 125.0f);

    // Start with output LOW
    pio_sm_set_pins_with_mask(pio, sm, 0, 1u << PULSE_PIN);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

void trigger_pulse() {
     pio_sm_put_blocking(pio, sm, 0);
}


#define buffersize 7
int input_number(int * result) {

    char rx_buffer[buffersize];
    char received_char = 0;
    int buffer_index = 0;

    while(true) {
        received_char = getchar();
        if(received_char == 27) return -27;
        if(received_char == '\r' || received_char == '\n' ){            
            if(buffer_index == 0 ) return -13; //no value
            else break;
        }
        if(buffer_index > buffersize-1) break;
            
        if (received_char == '\b' && buffer_index > 0) {
            buffer_index--;
            putchar('\b');
            putchar(' ');
            putchar('\b');
        } else if( received_char >= '0' && received_char <= '9') {                    
            putchar(received_char);
            rx_buffer[buffer_index++] = received_char;
        }
    }

    rx_buffer[buffer_index] = '\0';
    return sscanf(rx_buffer, "%d", result);
}


void setPower(int power) {        
    uint16_t internal_power = (uint16_t)power;        
    setChannelValue(MCP4728_CHANNEL_A, internal_power, MCP4728_VREF_INTERNAL, MCP4728_GAIN_2X, MCP4728_PD_MODE_NORMAL, false);
}

