
#ifndef _menu_h
#define _menu_h


typedef struct {
    const char key;
    const char *description;
    void (*handler)();
} Command;

typedef struct {
    const char desc[40];
    const char unit[4];
    int value;
    const int min;
    const int max;    
} system_parameter; 


enum menu {
    help,
    set,
    show,
    trigger,
    menu_item_count,
};


enum system_desc {
    pulses,
    period,
    interval,
    trials,
    low_outp,
    high_outp,
    paramter_amount
};

void run_menu();
void cmd_help();
void triggerTms() ;
void cmd_showparameters();
void cmd_setparameters();
 
#endif