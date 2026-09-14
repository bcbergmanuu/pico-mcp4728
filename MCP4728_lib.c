#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "MCP4728_lib.h"

int setChannelValue(MCP4728_channel_t channel, uint16_t new_value, MCP4728_vref_t new_vref,
    MCP4728_gain_t new_gain, MCP4728_pd_mode_t new_pd_mode, bool udac) {

    uint8_t output_buffer[3];

    // build the setter header/ "address"
    // 0 1 0 0 0 DAC1 DAC0 UDAC[A]
    uint8_t sequential_write_cmd = MCP4728_MULTI_IR_CMD;
    sequential_write_cmd |= (channel << 1);
    sequential_write_cmd |= udac;

    output_buffer[0] = sequential_write_cmd;
    // VREF PD1 PD0 Gx D11 D10 D9 D8 [A] D7 D6 D5 D4 D3 D2 D1 D0 [A]
    new_value |= (new_vref << 15);
    new_value |= (new_pd_mode << 13);
    new_value |= (new_gain << 12);

    output_buffer[1] = new_value >> 8;
    output_buffer[2] = new_value & 0xFF;

    return i2c_write_blocking(I2C_PORT, MCP4728_I2CADDR, output_buffer, 3, false);    
}

int fastWrite(uint16_t channel_a_value,
                                 uint16_t channel_b_value,
                                 uint16_t channel_c_value,
                                 uint16_t channel_d_value) {

  uint8_t output_buffer[8];

  output_buffer[0] = channel_a_value >> 8;
  output_buffer[1] = channel_a_value & 0xFF;

  output_buffer[2] = channel_b_value >> 8;
  output_buffer[3] = channel_b_value & 0xFF;

  output_buffer[4] = channel_c_value >> 8;
  output_buffer[5] = channel_c_value & 0xFF;

  output_buffer[6] = channel_d_value >> 8;
  output_buffer[7] = channel_d_value & 0xFF;

  return i2c_write_blocking(I2C_PORT, MCP4728_I2CADDR, output_buffer, 8, false);  
}

/**
 * @brief Saves the DAC's input register settings to the internal EEPROM,
 * makeing them the default values when the ADC is powered on
 *
 * @return true if the write was successful
 * @return false if there was an error with I2C communication between the MCU
 * and the DAC */

int saveToEEPROM(void) {
  uint8_t input_buffer[24];
  uint8_t output_buffer[9];

  i2c_read_blocking (I2C_PORT, MCP4728_I2CADDR, input_buffer, 24, false);

  // build header byte 0 1 0 1 0 DAC1 DAC0 UDAC [A]
  uint8_t eeprom_write_cmd = MCP4728_MULTI_EEPROM_CMD; // 0 1 0 1 0 xxx
  eeprom_write_cmd |=
      (MCP4728_CHANNEL_A << 1); // DAC1 DAC0, start at channel A obvs
  eeprom_write_cmd |= 0;        // UDAC ; yes, latch please
  // First byte is the write command+options
  output_buffer[0] = eeprom_write_cmd;

  // copy the incoming input register bytes to the outgoing buffer
  // Channel A
  output_buffer[1] = input_buffer[1];
  output_buffer[2] = input_buffer[2];
  // Channel B
  output_buffer[3] = input_buffer[7];
  output_buffer[4] = input_buffer[8];
  // Channel C
  output_buffer[5] = input_buffer[13];
  output_buffer[6] = input_buffer[14];
  // Channel D
  output_buffer[7] = input_buffer[19];
  output_buffer[8] = input_buffer[20];

  return i2c_write_blocking(I2C_PORT, MCP4728_I2CADDR, output_buffer, 9, false);  
}

/**
 * @brief Read the current value of one DAC value.
 *
 * @param channel the channel to read
 * @return current value of the specified channel
 */

int getChannelValue(MCP4728_channel_t channel, uint16_t *value) {  
  uint8_t input_buffer[24];
  /* 24 bytes are (3 bytes outreg, 3 bytes EEPROM) x 4 channels */
  uint8_t reg_base = 6 * ((uint8_t)channel);

  int ret = i2c_read_blocking (I2C_PORT, MCP4728_I2CADDR, input_buffer, 24, false);
  *value = input_buffer[reg_base + 2] + ((0x0F & input_buffer[reg_base + 1]) << 8);
  
  return ret;
}

/**
 * @brief Read the current value of one EEPROM value.
 *
 * @param channel the channel to read
 * @return current value of the specified channel
 */

int getEEPROMValue(MCP4728_channel_t channel, uint16_t *value) {  
  uint8_t input_buffer[24];
  /* 24 bytes are (3 bytes outreg, 3 bytes EEPROM) x 4 channels */
  uint8_t reg_base = 6 * ((uint8_t)channel);

  int ret = i2c_read_blocking (I2C_PORT, MCP4728_I2CADDR, input_buffer, 24, false);
  *value = input_buffer[reg_base + 5] + ((0x0F & input_buffer[reg_base + 4]) << 8);

  return ret;
}




