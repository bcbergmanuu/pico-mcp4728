#include <stdio.h>
#include "pico/stdlib.h"

#include "menu.h"
#include "helpers.h"


int main()
{
    stdio_init_all();
    
    sleep_ms(4000); 
    pulse_init();    
    int ret = initi2c();
    if(ret != 0) {
        printf("error i2c init");
        return 0;
    }

    run_menu();

    return 0;
}
