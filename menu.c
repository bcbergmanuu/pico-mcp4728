#include <stdio.h>
#include "pico/stdlib.h"

#include "menu.h"
#include "helpers.h"

int system_values[paramter_amount] = {20, 50, 500, 4, 100, 1000};

Command main_commands[menu_item_count] = {
    {'h',  "Show Command List",        cmd_help},
   // {'c',  "show i2c address", scan_test},
   {'s', "Set program parameters", cmd_setparameters}, 
   
   {'l', "Show current paramters", cmd_showparameters},
   {'t', "Trigger TMS now",  triggerTms},
    
};

system_parameter system_parameters[paramter_amount] = {
    {pulses, "number of pulses per trial", "", 2, 10000},
    {period, "period","ms", 10, 10000},
    {interval, "intertrial interval", "ms" ,0, 10000},
    {trials, "number of trials", "", 1, 100},
    {low_outp, "base system output", "V", 10, 2000},
    {high_outp, "high system output", "V", 10, 2000}
};



int set_systemparameter(system_parameter *param) {
    while (true) {                              
        printf("\n%s" , param->desc);
        if(param->unit[0] != '\0') {  
            printf(" [%s]", param->unit);
        }
        int tempvalue = 0;
        printf(" (%d): ", system_values[param->id]);        
        int err = input_number(&tempvalue);
        if(err < 0) return err;     
        if(err > 0 && tempvalue >= param->min && tempvalue <= param->max) {
            system_values[param->id] = tempvalue;
            return 0;
        } 
        printf("\n\t!Value should be within %d and %d %s", param->min, param->max, param->unit);
    }   
    
}

void cmd_showparameters() {
    printf("%c\n", main_commands[show].key);
    for(int x =0; x< paramter_amount; x++ ){
        printf("%30s: %d %s\n", system_parameters[x].desc, system_values[x], system_parameters[x].unit);
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
            break;
        } else if (err == -13) {
            printf("no change");
        }
    } 
    save_values((const uint8_t*)system_values, sizeof(system_values));      
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
    setPower(system_values[low_outp]);
    for(int trial_num = 0; trial_num < system_values[trials]; trial_num++) {                                
        printf("\n%d:", trial_num);
        int cancel = 0;
        for(int x = 0; x < system_values[pulses]; x++) {
            //last pulse, do not charge capacitor anymore
            if(x == system_values[pulses] -1) {
                setPower(system_values[low_outp]);
                printf("#");            
            }
            trigger_pulse();            
            //second last pulse, ramp up voltage right after
            if(x == system_values[pulses] -2) {
                setPower(system_values[high_outp]);
                printf("*");
            }
            //none of above, just indicate
            if(x < system_values[pulses] - 2) {
                printf(".");
            }
            
            cancel = getchar_timeout_us(system_values[period] * 1000) ;
            if(cancel != PICO_ERROR_TIMEOUT) {
                printf("\nexecution cancelled during trial %d\n", cancel);
                return;                        
            }
        }
        
        cancel = getchar_timeout_us(system_values[interval] * 1000);            
        if(cancel != PICO_ERROR_TIMEOUT) {
            printf("\nexecution cancelled during interval %d\n", cancel);
            return;
        }            
    }
    
    printf("\nTms trigger finished\n");
}


void run_menu() {
    load_values(system_values, sizeof(system_values));
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