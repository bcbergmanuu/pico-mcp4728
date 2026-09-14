#include <stdio.h>
#include "pico/stdlib.h"

#include "menu.h"
#include "helpers.h"

Command main_commands[menu_item_count] = {
    {'h',  "Show Command List",        cmd_help},
   // {'c',  "show i2c address", scan_test},
   {'s', "Set program parameters", cmd_setparameters}, 
   
   {'l', "Show current paramters", cmd_showparameters},
   {'t', "Trigger TMS now",  triggerTms},
    
};

system_parameter system_parameters[paramter_amount] = {
    {"number of pulses per trial", "", 20, 2, 10000},
    {"period","ms", 50, 10, 10000},
    {"intertrial interval", "ms", 500,0, 10000},
    {"number of trials", "", 4, 1, 100},
    {"base system output", "V", 100, 10, 2000},
    {"high system output", "V", 1000, 10, 2000}
};

int set_systemparameter(system_parameter *param) {
    while (true) {                              
        printf("\n%s" , param->desc);
        if(param->unit[0] != '\0') {  
            printf(" [%s]", param->unit);
        }
        int tempvalue = 0;
        printf(" (%d): ",param->value);        
        int err = input_number(&tempvalue);
        if(err < 0) return err;     
        if(err > 0 && tempvalue >= param->min && tempvalue <= param->max) {
            param->value = tempvalue;
            return 0;
        } 
        printf("\n\t!Value should be within %d and %d %s", param->min, param->max, param->unit);
    }        
}

void cmd_showparameters() {
    printf("%c\n", main_commands[show].key);
    for(int x =0; x< paramter_amount; x++ ){
        printf("%30s: %d %s\n", system_parameters[x].desc, system_parameters[x].value, system_parameters[x].unit);
    }
}

void cmd_setparameters() { 
    
    printf("%c\n", main_commands[set].key);
    int err = 0;
    for(int x =0; x< paramter_amount; x++ ){
        err = set_systemparameter(&system_parameters[x]);
        if(err == -27) 
        {
            printf("\n\n");
            return;
        } else if (err == -13) {
            printf("no change");
        }
    }       
}


void cmd_help()
{    
    //printf("\033[2J");
    //printf("\033c");
    printf("%c\nAvailable commands:\n", main_commands[help].key);

    for (size_t i = 0; i < menu_item_count; i++) {
        printf("%c - %s\n",
               main_commands[i].key,
               main_commands[i].description);
    }
}


void triggerTms() {
    printf("%c\n", main_commands[trigger].key);
    printf("triggering TMS, press esc to cancel\n");
    for(int trial_num = 0; trial_num < system_parameters[trials].value; trial_num++) {
        
                
        setPower(system_parameters[low_outp].value);
        printf("\n%d:", trial_num);
        int cancel = 0;
        for(int x = 0; x < system_parameters[pulses].value; x++) {
            trigger_pulse();
            //pio_sm_put_blocking(pio, sm, 0);
            if(x == system_parameters[pulses].value -2) {
                setPower(system_parameters[high_outp].value);
                printf("*");
            } else if(x == system_parameters[pulses].value -1) {
                setPower(system_parameters[low_outp].value);
                printf("#");            
            } else {
                printf(".");
            }
            
            cancel = getchar_timeout_us(system_parameters[period].value * 1000) ;
            if(cancel != PICO_ERROR_TIMEOUT) {
                printf("\nexecution cancelled during trial %d\n", cancel);
                return;                        
            }
        }
        
        cancel = getchar_timeout_us(system_parameters[interval].value * 1000);            
        if(cancel != PICO_ERROR_TIMEOUT) {
            printf("\nexecution cancelled during interval %d\n", cancel);
            return;
        }            
    }
    
    printf("\nTms trigger finished\n");
}


void run_menu() {
    cmd_help();
    int command = 0;
    printf("\n\n> ");
    while(true) {
        
        command = getchar();
        for(int i = 0; i < menu_item_count; i++) {
            if(main_commands[i].key == command) {
                main_commands[i].handler();
                printf("\n\n> ");
            }
        }
    }
}