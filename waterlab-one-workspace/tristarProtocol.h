/*
    Carl Lindquist
    June 1, 2017

    Module for interfacing the PSoC 5LP with the Morningstar Tristar MPPT 45 solar
    charge controller. This module outputs to a UART shifter, external hardware
    must shift the voltages to the RS-232 standard.
*/
#ifndef TRISTAR_PROTOCOL_H
#define TRISTAR_PROTOCOL_H
    
#include "project.h"
    
enum MODBUS_FUNCTION_CODES {
    READ_DISCRETE_INPUTS = 0x02,
    READ_COILS = 0x01,
    WRITE_SINGLE_COIL = 0x05,
    READ_INPUT_REG = 0x04,
    READ_HOLD_REG = 0x03,
    WRITE_SINGLE_REG = 0x06,
    WRITE_MULTIPLE_REG = 0x10,
    READ_WRITE_MULTIPLE_REG = 0x17,
    MASK_WRITE_REG = 0x16,
    READ_FIFO_QUEUE = 0x18,
    READ_EXCEPTION_STATUS = 0x07,
    DIAGNOSTIC = 0x08,
    READ_DEVICE_ID = 0x2B,
};

    
//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//
    
void tstarStart(void);


/*
[desc]  Sends an array of data to a Tristar MPPT PC over UART. This must be shifted
        and inverted to RS-232 voltages externally. Will only send if the
        Tristar recognizes the [function] code chosen as valid.

[function] A MODBUS function code for a server request. Use the MODBUS_FUNCTION_CODES enum.
[data] The data to be sent in the frame.
[length] Length of the data to be sent.

[ret]   Returns 1 if [function] code is a valid Tristar command, 0 otherwise.
*/
uint8 tstarSendData(uint8 function, uint8 data[], uint8 length);

    
#endif /* TRISTAR_PROTOCOL_H */
