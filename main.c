#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "MCP4728_lib.h"


 // I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
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
    printf("initi2c\n");
    i2c_init(I2C_PORT, 100*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    scan_test();
    // while(1) {
    //   sleep_ms(1000);
    // }
    return 0;
}

int main()
{
    stdio_init_all();
    sleep_ms(4000);
    printf("stdio ready\n");
    int ret = initi2c();
    
    while(1) {
        ret = setChannelValue(MCP4728_CHANNEL_B, 2048, MCP4728_VREF_INTERNAL, MCP4728_GAIN_2X, MCP4728_PD_MODE_NORMAL, false);
        printf("channel B set to 1024, %d\n", ret);
        sleep_ms(2000);
        ret = setChannelValue(MCP4728_CHANNEL_A, 512, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X, MCP4728_PD_MODE_NORMAL, false);
        printf("channel A set to 512, %d\n", ret);
        sleep_ms(2000);
        ret = setChannelValue(MCP4728_CHANNEL_A, 256, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X, MCP4728_PD_MODE_NORMAL, false);
        printf("channel A set to 256, %d\n", ret);
        sleep_ms(2000);
        ret = setChannelValue(MCP4728_CHANNEL_B, 0, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X, MCP4728_PD_MODE_NORMAL, false);
        printf("channel B set to 0, %d\n", ret);
        sleep_ms(2000);
    }

    return 0;
}
