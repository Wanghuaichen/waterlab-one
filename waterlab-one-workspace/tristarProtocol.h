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

/*
[desc]  Initialization code for the tristar module. This module's funcationality is
		undefined if this is not called before other functions.
*/
void tstarStart(void);

/*
[desc]  Returns the battery voltage as measured by the Tristar MMPT. Blocking.

[ret]   The battery voltage.
*/
double tstarBattVolt(void);

/*
[desc]  Returns the pv current as measured by the Tristar MMPT. Blocking.

[ret]   The PV panel current.
*/
double tstarPVCurrent(void);

    
#endif /* TRISTAR_PROTOCOL_H */
