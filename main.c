#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "MCP4728_lib.h"
#include "hardware/pio.h"

#include "pulse.pio.h"

#define PULSE_PIN 16

static PIO pio = pio0;
static uint sm;
static uint offset;

#define CMD_BUFFER_SIZE 64


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

typedef struct {
    const char key;
    const char *description;
    void (*handler)();
} Command;

uint16_t set_power = 0;

void cmd_setPower() {
    
    printf("set power to: \n"); 
    for(int x = 0; x < 10; x++) {
        printf("%d: %dV\n", x, (x+1)*200);
    }    
    int input = getchar();

    if(input == 27 || input < 48 || input > 57) {
        printf("power not changed\n");
        return;
    }
    set_power = (input + 1) * 200;
    setChannelValue(MCP4728_CHANNEL_A, set_power, MCP4728_VREF_INTERNAL, MCP4728_GAIN_2X, MCP4728_PD_MODE_NORMAL, false);
    printf("Power set to %d\n - %dV", input-48, set_power);  
}

int repetitions = 0;
int periodms = 0;

void cmd_setRepetition() {
    printf("Enter repetitions (input not visible in terminal): ");    
    scanf("%d", &repetitions);
    printf("\nEnter period in ms (input not visible in terminal)");
    scanf("%d", &periodms);

    printf("\nperiod set to %dms and %d repetitions\n", periodms, repetitions);
}

void cmd_help();
void triggerTms() ;

#define commandcount 4
Command main_commands[commandcount] = {
    {'l',  "Show Command List",        cmd_help},
   // {'c',  "show i2c address", scan_test},
    {'t', "Trigger TMS now",  triggerTms},
    {'p', "Set Power", cmd_setPower},
    {'r', "Set Repetition", cmd_setRepetition}
};

void cmd_help()
{
    printf("\033c");
    printf("\nAvailable commands:\n");

    for (size_t i = 0; i < commandcount; i++) {
        printf("%c - %s\n",
               main_commands[i].key,
               main_commands[i].description);
    }
}

void triggerTms() {
    printf("triggering TMS, press esc to cancel\n");
    char cancel = 0;
    for(int x = 0; x < repetitions; x++) {
        pio_sm_put_blocking(pio, sm, 0);
        printf(".");
        cancel = getchar_timeout_us(periodms * 1000);        
        if(cancel == 27) {
            printf("\nTrigger cancelled \n");
            return;            
        }
    }
    
    printf("\nTms trigger finished\n");
}

void menu() {
    cmd_help();
    int command = 0;
    while(true) {
        command = getchar_timeout_us(1000);
        for(int i = 0; i < commandcount; i++) {
            if(main_commands[i].key == command) {
                main_commands[i].handler();
            }
        }
        //enter is menu:
        if(command == 13) cmd_help();
    }
}

int main()
{
    stdio_init_all();
    pulse_init();
    sleep_ms(4000);    
    int ret = initi2c();
    menu();
        
    while(1) {
        for(int x = 0; x < 11; x++) {
            ret = setChannelValue(MCP4728_CHANNEL_A, x*200, MCP4728_VREF_INTERNAL, MCP4728_GAIN_2X, MCP4728_PD_MODE_NORMAL, false);
            printf("channel A set to %d\n", x*200);
            sleep_ms(1000);
            
        }        
    }

    return 0;
}
