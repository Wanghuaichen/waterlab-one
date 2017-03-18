/*
    Carl Lindquist
    Mar 17, 2017

    Library for interfacing with the Atlas-Scientific EZO class circuits
    over I2C. This library will poll the sensors using a hardware timer 
    and store data points for immediate public access to recent readings.
*/
    
#ifndef EZO_PROTOCOL_H
#define EZO_PROTOCOL_H

#include "project.h"
    
    
#define MAX_RESPONSE_LENGTH 128
#define EC_SENSOR_ADDRESS 100
    

typedef struct arrStruct { char d[MAX_RESPONSE_LENGTH]; } arrStruct;


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

/*
[desc]  Initialization code for the ezoLibrary. This library's funcationality is
		undefined if this is not called before other functions.
*/
void ezoStart(void);


/*
[desc]  Sends a string to the slave specified by slaveAddress in I2C protocol. Assumes
		this device is set up as an I2C Master with a hardware block names I2CM.

[slaveAddress] Address of an I2C device between 0 and 127
[string] A string to send over I2C
*/
void i2cSendString(uint8 slaveAddress, char string[]);

/*
[desc]  Reads a buffer from the slave specified by slaveAddress in I2C protocol. Assumes
		this device is set up as an I2C Master with a hardware block names I2CM.

[slaveAddress] Address of an I2C device between 0 and 127

[ret]	An arrStruct holding the buffer of data read from the slave.
*/
arrStruct i2cReadString(uint8 slaveAddress);






#endif /* EZO_PROTOCOL_H */