
#ifndef _menu_h
#define _menu_h

typedef const struct {
    char key;
    char *description;
    void (*handler)();
} Command;



enum menu {
    help,
    set,
    show,
    trigger,
    menu_item_count,
};

typedef enum {
    pulses,
    period,
    interval,
    trials,
    low_outp,
    high_outp,
    paramter_amount,
} system_desc;

typedef const struct {
    system_desc id;
    char *desc;
    char *unit;  
    int min;
    int max;    
} system_parameter; 


void run_menu();
void cmd_help();
void triggerTms() ;
void cmd_showparameters();
void cmd_setparameters();
 
#endif