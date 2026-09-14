
#ifndef _helpers_h
#define _helpers_h


#define buffersize 7
#define PULSE_PIN 16

void save_values(const uint8_t* system_values, size_t size);
void load_values(int* system_values, size_t size);

int input_number(int * result);
void trigger_pulse();
void setPower(int power);
int pulse_init(void);
int initi2c();
#endif